# -*- coding: utf-8 -*-

from mantid.api import AlgorithmFactory, AnalysisDataServiceImpl, DataProcessorAlgorithm, ITableWorkspaceProperty, MatrixWorkspaceProperty, mtd, PropertyMode, WorkspaceProperty
from mantid.kernel import Direct, Direction, UnitConversion
from mantid.simpleapi import AddSampleLog, CalculateFlatBackground,\
                             CloneWorkspace, ComputeCalibrationCoefVan,\
                             ConvertUnits, CorrectKiKf, CreateSingleValuedWorkspace, CreateWorkspace, DetectorEfficiencyCorUser, Divide, ExtractSpectra,\
                             FindDetectorsOutsideLimits, FindEPP, GetEiMonDet, GroupWorkspaces, Integration, Load,\
                             MaskDetectors, Minus, NormaliseToMonitor, Rebin
import numpy

INDEX_TYPE_DETECTOR_ID     = 'DetectorID'
INDEX_TYPE_SPECTRUM_NUMBER = 'SpectrumNumber'
INDEX_TYPE_WORKSPACE_INDEX = 'WorkspaceIndex'

NORM_METHOD_MONITOR = 'Monitor'
NORM_METHOD_TIME    = 'AcquisitionTime'

PROP_BINNING_Q                        = 'QBinning'
PROP_BINNING_W                        = 'WBinning'
PROP_CD_WORKSPACE                     = 'CadmiumWorkspace'
PROP_DETECTORS_FOR_EI_CALIBRATION     = 'IncidentEnergyCalibrationDetectors'
PROP_EC_WORKSPACE                     = 'EmptyCanWorkspace'
PROP_EPP_WORKSPACE                    = 'EPPWorkspace'
PROP_FLAT_BACKGROUND_FIT_BEGIN        = 'FlatBackgroundFitBegin'
PROP_FLAT_BACKGROUND_FIT_END          = 'FlatBackgroundFitEnd'
PROP_FLAT_BACKGROUND_WORKSPACE        = 'FlatBackgroundWorkspace'
PROP_INDEX_TYPE                       = 'IndexType'
PROP_INPUT_FILE                       = 'InputFile'
PROP_MONITOR_EPP_WORKSPACE            = 'MonitorEPPWorkspace'
PROP_MONITOR_INDEX                    = 'MonitorIndex'
PROP_NORMALISATION                    = 'Normalisation'
PROP_OUTPUT_EPP_WORKSPACE             = 'OutputEPPWorkspace'
PROP_OUTPUT_FLAT_BACKGROUND_WORKSPACE = 'OutputFlatBackgroundWorkspace'
PROP_OUTPUT_MONITOR_EPP_WORKSPACE     = 'OutputMonitorEPPWorkspace'
PROP_OUTPUT_SUFFIX                    = 'OutputPrefix'
PROP_OUTPUT_WORKSPACE                 = 'OutputWorkspace'
PROP_REDUCTION_TYPE                   = 'ReductionType'
PROP_TRANSMISSION                     = 'Transmission'
PROP_USER_MASK                        = 'SpectrumMask'
PROP_VANADIUM_WORKSPACE               = 'VanadiumWorkspace'

REDUCTION_TYPE_CD = 'Empty can/cadmium'
REDUCTION_TYPE_EC = REDUCTION_TYPE_CD
REDUCTION_TYPE_SAMPLE = 'Sample'
REDUCTION_TYPE_VANADIUM = 'Vanadium'

def backgroundWorkspaceName(token):
    return token + '_bkg'

def backgroundSubtractedWorkspaceName(token):
    return token + '_bkgsubtr'

def badDetectorWorkspaceName(token):
    return token + '_mask'

def detectorEfficiencyCorrectedWorkspaceName(token):
    return token + '_deteff'

def ecSubtractedWorkspaceName(token):
    return token + '_ecsubtr'

def eiCalibratedWorkspaceName(token):
    return token + '_ecalib'

def energyConvertedWorkspaceName(token):
    return token +'_econv'

def eppWorkspaceName(token):
    return token + '_epp'

def incidentEnergyWorkspaceName(token):
    return token + '_ie'

def kikfConvertedWorkspaceName(token):
    return token + '_kikf'

def maskedWorkspaceName(token):
    return token + '_masked'

def monitorEppWorkspaceName(token):
    return token + '_monepp'

def monitorWorkspaceName(token):
    return token + '_monitors'

def normalisedWorkspaceName(token):
    return token + '_norm'

def rawWorkspaceName(token):
    return token + '_raw'

def rebinnedWorkspaceName(token):
    return token + '_rebinned'

def vanadiumNormalisedWorkspaceName(token):
    return token + '_vnorm'

def guessIncidentEnergyWorkspaceName(eppWorkspace):
    splits = eppWorkspace.getName().split('_')
    return ''.join(splits[:-1]) + '_ie'

def stringListToArray(string):
    return eval('[' + string + ']')

class DGSReductionILL(DataProcessorAlgorithm):

    def __init__(self):
        DataProcessorAlgorithm.__init__(self)

    def category(self):
        return 'Workflow\\Inelastic'

    def name(self):
        return 'DGSReductionILL'

    def summary(self):
        return 'Data reduction workflow for the direct geometry time-of-flight spectrometers at ILL'

    def version(self):
        return 1

    def PyExec(self):
        reductionType = self.getProperty(PROP_REDUCTION_TYPE).value
        identifier = self.getProperty(PROP_OUTPUT_SUFFIX).value
        indexType = self.getProperty(PROP_INDEX_TYPE).value

        # Load data
        inputFilename = self.getProperty(PROP_INPUT_FILE).value
        outWs = rawWorkspaceName(identifier)
        workspace = Load(Filename=inputFilename,
                         OutputWorkspace=outWs)
        # TODO Merge runs
        # Extract monitors to a separate workspace
        monitors = list()
        notMonitors = list()
        for i in range(workspace.getNumberHistograms()):
            det = workspace.getDetector(i)
            if det.isMonitor():
                monitors.append(i)
            else:
                notMonitors.append(i)
        monitorWorkspace = monitorWorkspaceName(identifier)
        monitorWorkspace = ExtractSpectra(InputWorkspace=workspace,
                                          OutputWorkspace=monitorWorkspace,
                                          WorkspaceIndexList=monitors)
        workspace = ExtractSpectra(InputWorkspace=workspace,
                                   OutputWorkspace=workspace,
                                   WorkspaceIndexList=notMonitors)
        workspace.setMonitorWorkspace(monitorWorkspace)
        monitorIndex = self.getProperty(PROP_MONITOR_INDEX).value
        monitorIndex = self._convertToWorkspaceIndex(monitorIndex, monitorWorkspace)
        

        # Fit time-independent background
        # ATM monitor background is ignored
        bkgInWs = self.getProperty(PROP_FLAT_BACKGROUND_WORKSPACE).value
        bkgOutWs = self.getPropertyValue(PROP_OUTPUT_FLAT_BACKGROUND_WORKSPACE)
        # Fit background regardless of where it actually comes from.
        # The output needs to be set anyhow.
        if bkgOutWs or not bkgInWs:
            if not bkgOutWs:
                bkgOutWs = backgroundWorkspaceName(identifier)
            begin = self.getProperty(PROP_FLAT_BACKGROUND_FIT_BEGIN).value
            end = self.getProperty(PROP_FLAT_BACKGROUND_FIT_END).value
            bkgWorkspace = CalculateFlatBackground(InputWorkspace=workspace,
                                                   OutputWorkspace=bkgOutWs,
                                                   StartX=begin,
                                                   EndX=end,
                                                   OutputMode='Return Background',
                                                   NullifyNegativeValues=False)
            self.setProperty(PROP_OUTPUT_FLAT_BACKGROUND_WORKSPACE, bkgOutWs)
        if bkgInWs:
            bkgWorkspace = self.getProperty(PROP_FLAT_BACKGROUND_WORKSPACE).value
            # TODO: test if outputting empty workspaces is really needed.
            if not bkgOutWs:
                # Even if no background was fitted, we need to set some
                # output.
                bkgOutWs = "dummy_output"
                emptyOutput = CreateWorkspace(OutputWorkspace=bkgOutWs,
                                              DataX="",
                                              DataY="",
                                              DataE="",
                                              NSpec=0)
                self.setProperty(PROP_OUTPUT_FLAT_BACKGROUND_WORKSPACE, bkgOutWs)
        # Subtract time-independent background
        outWs = backgroundSubtractedWorkspaceName(identifier)
        workspace = Minus(LHSWorkspace=workspace,
                          RHSWorkspace=bkgWorkspace,
                          OutputWorkspace=outWs)

        # Find elastic peak positions
        eppInWs = self.getProperty(PROP_EPP_WORKSPACE).value
        monitorEppInWs = self.getProperty(PROP_MONITOR_EPP_WORKSPACE).value
        eppOutWs = self.getPropertyValue(PROP_OUTPUT_EPP_WORKSPACE)
        monitorEppOutWs = self.getPropertyValue(PROP_OUTPUT_MONITOR_EPP_WORKSPACE)
        if eppOutWs or not eppInWs:
            if not eppOutWs:
                eppOutWs = eppWorkspaceName(identifier)
                monitorEppOutWs = monitorEppWorkspaceName(identifier)
            eppWorkspace = FindEPP(InputWorkspace = workspace, 
                                   OutputWorkspace = eppOutWs)
            monitorEppWorkspace = FindEPP(InputWorkspace = monitorWorkspace,
                                          OutputWorkspace = monitorEppOutWs)
            self.setProperty(PROP_OUTPUT_EPP_WORKSPACE, eppWorkspace)
            self.setProperty(PROP_OUTPUT_MONITOR_EPP_WORKSPACE, monitorEppWorkspace)
        if eppInWs:
            eppWorkspace = self.getProperty(PROP_EPP_WORKSPACE).value
            monitorEppWorkspace = self.getProperty(PROP_MONITOR_EPP_WORKSPACE).value
            if not eppOutWs:
                eppOutWs = "dummy_output"
                emptyOutput = CreateSingleValuedWorkspace(OutputWorkspace=bkgOutWs,
                                                          DataValue = 0)
                self.setProperty(PROP_OUTPUT_EPP_WORKSPACE, eppOutWs)
                self.setProperty(PROP_OUTPUT_MONITOR_EPP_WORKSPACE, eppOutWs)
        
        # Identify bad detectors & include user mask
        userMask = stringListToArray(self.getProperty(PROP_USER_MASK).value)
        outWs = badDetectorWorkspaceName(identifier)
        badDetWorkspace, nFailures = FindDetectorsOutsideLimits(InputWorkspace=workspace,
                                                                OutputWorkspace=outWs)
        def mask(maskWs, i):
            maskWs.setY(i, numpy.array([maskWs.readY(i)[0] + 1.0]))
        for i in range(badDetWorkspace.getNumberHistograms()):
            if eppWorkspace.cell('FitStatus', i) != 'success':
                mask(badDetWorkspace, i)
            if i in userMask:
                mask(badDetWorkspace, i)
        # Mask detectors
        outWs = maskedWorkspaceName(identifier)
        workspace = CloneWorkspace(InputWorkspace=workspace,
                                   OutputWorkspace=outWs)
        MaskDetectors(Workspace=workspace,
                      MaskedWorkspace=badDetWorkspace)

        # Get calibrated incident energy from somewhere
        # It should come from the same place as the epp workspace.
        instrument = workspace.getInstrument().getName()
        if instrument in ['IN4', 'IN6']:
            eiWsName = guessIncidentEnergyWorkspaceName(eppWorkspace)
            if not AnalysisDataServiceImpl.Instance().doesExist(eiWsName):
                eiCalibrationDets = self.getProperty(PROP_DETECTORS_FOR_EI_CALIBRATION).value
                instrument = workspace.getInstrument().getName()
                if instrument == 'IN4':
                    fermiChopperSpeed = workspace.run().getLogData('FC.rotation_speed').value
                    backgroundChopper1Speed = workspace.run().getLogData('BC1.rotation_speed').value
                    backgroundChopper2Speed = workspace.run().getLogData('BC2.rotation_speed').value
                    if abs(backgroundChopper1Speed - backgroundChopper2Speed) > 1:
                        raise RuntimeError('background choppers 1 and 2 have different speeds')
                    n = fermiChopperSpeed / backgroundChopper1Speed / 4
                    pulseInterval = 60.0 / (2 * fermiChopperSpeed) * n
                elif instrument == 'IN6':
                    fermiChopperSpeed = workspace.run().getLogData('Fermi.rotation_speed').value
                    suppressorSpeed = workspace.run().getLogData('Suppressor.rotation_speed').value
                    n = fermiChopperSpeed / suppressorSpeed
                    pulseInterval = 60.0 / (2 * fermiChopperSpeed) * n
                energy = GetEiMonDet(DetectorWorkspace=workspace,
                                     DetectorEPPTable=eppWorkspace,
                                     IndexType=indexType,
                                     Detectors=eiCalibrationDets,
                                     MonitorWorkspace=monitorWorkspace,
                                     MonitorEppTable=monitorEppWorkspace,
                                     Monitor=self.getProperty(PROP_MONITOR_INDEX).value,
                                     PulseInterval=pulseInterval)
                eiWsName = incidentEnergyWorkspaceName(identifier)
                CreateSingleValuedWorkspace(OutputWorkspace=eiWsName,
                                            DataValue=energy)
            # Update incident energy
            energy = mtd[eiWsName].readY(0)[0]
            outWs = eiCalibratedWorkspaceName(identifier)
            workspace = CloneWorkspace(InputWorkspace=workspace,
                                       OutputWorkspace=outWs)
            AddSampleLog(Workspace=workspace,
                         LogName='Ei',
                         LogText=str(energy),
                         LogType='Number',
                         NumberType='Double',
                         LogUnit='meV')
            wavelength = UnitConversion.run('Energy', 'Wavelength', energy, 0, 0, 0, Direct, 5)
            AddSampleLog(Workspace=workspace,
                         Logname='wavelength',
                         LogText=str(wavelength),
                         LogType='Number',
                         NumberType='Double',
                         LogUnit='Ångström')
        else:
            self.log.notice('Skipping incident energy calibration for ' + instrument)

        # Normalisation to monitor/time
        normalisationMethod = self.getProperty(PROP_NORMALISATION).value
        if normalisationMethod:
            if normalisationMethod == NORM_METHOD_MONITOR:
                outWs = normalisedWorkspaceName(identifier)
                eppRow = monitorEppWorkspace.row(monitorIndex)
                sigma = eppRow['Sigma']
                centre = eppRow['PeakCentre']
                begin = centre - 3 * sigma
                end = centre + 3 * sigma
                workspace, normFactor = NormaliseToMonitor(InputWorkspace=workspace,
                                                           OutputWorkspace=outWs,
                                                           MonitorWorkspace=monitorWorkspace,
                                                           MonitorWorkspaceIndex=monitorIndex,
                                                           IntegrationRangeMin=begin,
                                                           IntegrationRangeMax=end)
            elif normalisationMethod == NORM_METHOD_TIME:
                outWs = normalisedWorkspaceName(identifier)
                tempWsName = '__actual_time_for_' + outWs
                time = CreateSingleValuedWorkspace(OutputWorkspace=tempWsName,
                                                   DataValue=inWs.getLogData('actual_time').value)
                workspace = Divide(LHSWorkspace=workspace,
                                   RHSWorkspace=tempWsName,
                                   OutputWorkspace=outWs)
                DeleteWorkspace(Workspace=time)
            else:
                raise RuntimeError('Unknonwn normalisation method ' + normalisationMethod)

        # Reduction for empty can and cadmium ends here.
        if reductionType == REDUCTION_TYPE_CD or reductionType == REDUCTION_TYPE_EC:
            self._finalize(workspace)
            return

        # Empty can subtraction
        ecWs = self.getProperty(PROP_EC_WORKSPACE).value
        if ecWs:
            outWs = ecSubtractedWorkspaceName(identifier)
            cdWs = self.getProperty(PROP_CD_WORKSPACE).value
            transmission = self.getProperty(PROP_TRANSMISSION).value
            tempWsName1 = '__transmission_for_' + outWs
            transmission = CreateSingleValuedWorkspace(OutputWorkspace=tempWsName1,
                                                       DataValue=transmission)
            tempWsName2 = '__input_minus_Cd_for_' + outWs
            tempWsName3 = '__EC_minus_Cd_for_' + outWs
            # If no Cd, calculate
            # out = in / transmission - EC
            # If Cd, calculate
            # out = (in - Cd) / transmission - (EC - Cd)
            if cdWs:
                workspace = Minus(LHSWorkspace=workspace,
                                  RHSWorkspace=cdWs,
                                  OutputWorkspace=tempWsName2)
                ecWs = Minus(LHSWorkspace=ecWs,
                             RHSWorkspace=cdWs,
                             OutputWorkspace=tempWsNamw3)
            workspace = Divide(LHSWorkspace=workspace,
                               RHSWorkspace=transmission,
                               OutputWorkspace=outWs)
            Minus(LHSWorkspace=workspace,
                  RHSWorkspace=inEC,
                  OutputWorkspace=workspace)
            if cdWs:
                # Cleanup
                DeleteWorkspace(Workspace=tempWsName1)
                DeleteWorkspace(Workspace=tempWsName2)
                DeleteWorkspace(Workspace=tempWsName3)

        # Reduction for vanadium ends here.
        if reductionType == REDUCTION_TYPE_VANADIUM:
            outWs = self.getPropertyValue(PROP_OUTPUT_WORKSPACE)
            workspace = ComputeCalibrationCoefVan(VanadiumWorkspace=workspace,
                                                  EPPTable=eppWorkspace,
                                                  OutputWorkspace=outWs)
            self._finalize(workspace)
            return

        # Vanadium normalisation
        vanadiumNormFactors = self.getProperty(PROP_VANADIUM_WORKSPACE).value
        outWs = vanadiumNormalisedWorkspaceName(identifier)
        workspace = Divide(LHSWorkspace=workspace,
                           RHSWorkspace=vanadiumNormFactors,
                           OutputWorkspace=outWs)

        # Convert units from TOF to energy
        outWs = energyConvertedWorkspaceName(identifier)
        workspace = ConvertUnits(InputWorkspace = workspace,
                                 OutputWorkspace = outWs,
                                 Target = 'DeltaE',
                                 EMode = 'Direct')

        # KiKf conversion
        outWs = kikfConvertedWorkspaceName(identifier)
        workspace = CorrectKiKf(InputWorkspace = workspace,
                                OutputWorkspace = outWs)

        # Rebinning
        params = self.getProperty(PROP_BINNING_W).value
        outWs = rebinnedWorkspaceName(identifier)
        workspace = Rebin(InputWorkspace = workspace,
                          OutputWorkspace = outWs,
                          Params = params)

        # Detector efficiency correction
        outWs = detectorEfficiencyCorrectedWorkspaceName(identifier)
        workspace = DetectorEfficiencyCorUser(InputWorkspace = workspace,
                                              OutputWorkspace = outWs)

        # TODO: Self-shielding corrections

        self._finalize(workspace)

    def PyInit(self):
        # Inputs
        self.declareProperty(PROP_INPUT_FILE,
                             '',
                             direction=Direction.Input,
                             doc='Filename for the data to be reduced')
        self.declareProperty(PROP_OUTPUT_SUFFIX,
                             '',
                             direction=Direction.Input,
                             doc='String to use as postfix in output workspace names')
        self.declareProperty(PROP_REDUCTION_TYPE,
                             REDUCTION_TYPE_SAMPLE,
                             direction=Direction.Input,
                             doc='Type of reduction workflow to be run on ' + PROP_INPUT_FILE)
        self.declareProperty(PROP_NORMALISATION,
                             NORM_METHOD_MONITOR,
                             direction=Direction.Input,
                             doc='Normalisation method')
        self.declareProperty(MatrixWorkspaceProperty(PROP_VANADIUM_WORKSPACE,
                                                     '',
                                                     Direction.Input,
                                                     PropertyMode.Optional),
                             doc='Reduced vanadium workspace')
        self.declareProperty(MatrixWorkspaceProperty(PROP_EC_WORKSPACE,
                                                     '',
                                                     Direction.Input,
                                                     PropertyMode.Optional),
                             doc='Reduced empty can workspace')
        self.declareProperty(MatrixWorkspaceProperty(PROP_CD_WORKSPACE,
                                                     '',
                                                     Direction.Input,
                                                     PropertyMode.Optional),
                             doc='Reduced cadmium workspace')
        self.declareProperty(ITableWorkspaceProperty(PROP_EPP_WORKSPACE,
                                                     '',
                                                     Direction.Input,
                                                     PropertyMode.Optional),
                             doc='Table workspace containing results from the FindEPP algorithm')
        self.declareProperty(ITableWorkspaceProperty(PROP_MONITOR_EPP_WORKSPACE,
                                                     '',
                                                     Direction.Input,
                                                     PropertyMode.Optional),
                             doc='Table workspace containing results from the FindEPP algorithm for the monitor workspace')
        self.declareProperty(PROP_FLAT_BACKGROUND_FIT_BEGIN,
                             0.0,
                             direction=Direction.Input,
                             doc='Beginning of background fitting range')
        self.declareProperty(PROP_FLAT_BACKGROUND_FIT_END,
                             1000.0,
                             direction=Direction.Input,
                             doc='End of background fitting range')
        self.declareProperty(MatrixWorkspaceProperty(PROP_FLAT_BACKGROUND_WORKSPACE,
                                                     '',
                                                     Direction.Input,
                                                     PropertyMode.Optional),
                             doc='Workspace from which to get flat background data')
        self.declareProperty(PROP_MONITOR_INDEX,
                             1,
                             direction=Direction.Input,
                             doc='Index of the main monitor spectrum.')
        self.declareProperty(PROP_USER_MASK,
                             '',
                             direction=Direction.Input,
                             doc='List of spectra to mask')
        self.declareProperty(PROP_TRANSMISSION,
                             1.0,
                             direction=Direction.Input,
                             doc='Sample transmission for empty can subtraction')
        self.declareProperty(PROP_BINNING_Q,
                             '',
                             direction=Direction.Input,
                             doc='Rebinning in q')
        self.declareProperty(PROP_BINNING_W,
                             '',
                             direction=Direction.Input,
                             doc='Rebinning in w')
        self.declareProperty(PROP_DETECTORS_FOR_EI_CALIBRATION,
                             '',
                             direction=Direction.Input,
                             doc='List of detectors used for the incident energy calibration')
        self.declareProperty(PROP_INDEX_TYPE,
                             INDEX_TYPE_WORKSPACE_INDEX,
                             direction=Direction.Input,
                             doc='Type of numbers in ' + PROP_MONITOR_INDEX + ' and ' + PROP_DETECTORS_FOR_EI_CALIBRATION + ' properties')
        # Output
        self.declareProperty(ITableWorkspaceProperty(PROP_OUTPUT_EPP_WORKSPACE,
                             '',
                             direction=Direction.Output,
                             optional=PropertyMode.Optional),
                             doc='Output workspace for elastic peak positions')
        self.declareProperty(ITableWorkspaceProperty(PROP_OUTPUT_MONITOR_EPP_WORKSPACE,
                             '',
                             direction=Direction.Output,
                             optional=PropertyMode.Optional),
                             doc='Output workspace for elastic peak positions')
        self.declareProperty(WorkspaceProperty(PROP_OUTPUT_FLAT_BACKGROUND_WORKSPACE,
                             '',
                             direction=Direction.Output,
                             optional=PropertyMode.Optional),
                             doc='Output workspace for flat background')
        self.declareProperty(WorkspaceProperty(PROP_OUTPUT_WORKSPACE,
                             '',
                             direction=Direction.Output),
                             doc='The output of the algorithm')

    def _convertToWorkspaceIndex(self, i, workspace):
        indexType = self.getProperty(PROP_INDEX_TYPE).value
        if indexType == INDEX_TYPE_WORKSPACE_INDEX:
            return i
        elif indexType == INDEX_TYPE_SPECTRUM_NUMBER:
            return workspace.getIndexFromSpectrumNumber(i)
        else: # INDEX_TYPE_DETECTOR_ID
            for j in len(workspace.getNumberHistograms()):
                if workspace.getSpectrum(j).hasDetectorID(i):
                    return j
            raise RuntimeError('No workspace index found for detector id ' + i)

    def _finalize(self, outputWorkspace):
        self.setProperty(PROP_OUTPUT_WORKSPACE, outputWorkspace)
        self.log().debug('Finished')


AlgorithmFactory.subscribe(DGSReductionILL)
