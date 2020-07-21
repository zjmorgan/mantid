# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
from __future__ import (absolute_import, division, print_function)

from mantid.api import *
from mantid.kernel import *
from mantid.simpleapi import *
from SANSILLAutoProcess import needs_loading, needs_processing


class SANSILLParameterScan(DataProcessorAlgorithm):
    """
    Performs treatment for scans along a parameter for D16.
    """
    progress = None
    reduction_type = None
    sample = None
    absorber = None
    container = None
    sensitivity = None
    mask = None
    default_mask = None
    output = None
    output_sens = None
    normalise = None
    thickness = None
    output2D = None
    output_reduced = None
    observable = None
    pixel_y_min = None
    pixel_y_max = None

    def category(self):
        return 'ILL\\SANS;ILL\\Auto'

    def summary(self):
        return 'Integrate SANS scan data along a parameter'

    def seeAlso(self):
        return []

    def name(self):
        return 'SANSILLParameterScan'

    def validateInputs(self):
        issues = dict()
        message = 'Wrong number of {0} runs: {1}. Provide one or as many as sample runs: {2}.'
        sample_dim = self.getPropertyValue('SampleRuns').count(',')
        abs_dim = self.getPropertyValue('AbsorberRuns').count(',')
        can_dim = self.getPropertyValue('ContainerRuns').count(',')
        mask_dim = self.getPropertyValue('MaskFiles').count(',')
        sens_dim = self.getPropertyValue('SensitivityMaps').count(',')
        if not (self.getPropertyValue('ReducedData') or self.getPropertyValue("Output2D")):
            issues["ReducedData"] = "Please provide at least one output: ReducedData or Output2D."
            issues["Output2D"] = "Please provide at least one output: ReducedData or Output2D."
        if self.getPropertyValue('SampleRuns') == '':
            issues['SampleRuns'] = 'Please provide at least one sample run.'
        if abs_dim != sample_dim and abs_dim != 0:
            issues['AbsorberRuns'] = message.format('Absorber', abs_dim, sample_dim)
        if can_dim != sample_dim and can_dim != 0:
            issues['ContainerRuns'] = message.format('Container', can_dim, sample_dim)
        if mask_dim != sample_dim and mask_dim != 0:
            issues['MaskFiles'] = message.format('Mask', mask_dim, sample_dim)
        if sens_dim != sample_dim and sens_dim != 0:
            issues['SensitivityMaps'] = message.format('Sensitivity', sens_dim, sample_dim)
        return issues

    def setUp(self):
        self.sample = self.getPropertyValue('SampleRuns')
        self.absorber = self.getPropertyValue('AbsorberRuns')
        self.container = self.getPropertyValue('ContainerRuns')
        self.sensitivity = self.getPropertyValue('SensitivityMaps')
        self.default_mask = self.getPropertyValue('DefaultMaskFile')
        self.mask = self.getPropertyValue('MaskFiles')
        self.output_sens = self.getPropertyValue('SensitivityOutputWorkspace')
        self.normalise = self.getPropertyValue('NormaliseBy')
        self.output2D = self.getPropertyValue('Output2D')
        self.output_reduced = self.getPropertyValue('ReducedData')
        self.observable = self.getPropertyValue('Observable')
        self.pixel_y_min = self.getProperty('PixelYMin').value
        self.pixel_y_max = self.getProperty('PixelYMax').value
        self.progress = Progress(self, start=0.0, end=1.0, nreports=10)

    def cleanUp(self):
        mtd["__joined"].delete()
        if mtd.doesExist("sorted"):
            mtd["sorted"].delete()
        if mtd.doesExist("masked"):
            mtd["masked"].delete()

    def checkPixelY(self, height):
        if self.pixel_y_min > self.pixel_y_max:
            self.pixel_y_min, self.pixel_y_max = self.pixel_y_max, self.pixel_y_min

        if self.pixel_y_max > height:
            self.pixel_y_max = height
            logger.warning("PixelYMax value is too high. Reduced to {0}.".format(self.pixel_y_max))

    def PyInit(self):

        self.declareProperty(WorkspaceProperty('Output2D', '', direction=Direction.Output,
                                               optional=PropertyMode.Optional),
                             doc="The output workspace containing the 2D reduced data.")

        self.declareProperty(WorkspaceProperty('ReducedData', '', direction=Direction.Output,
                                               optional=PropertyMode.Optional),
                             doc="The output workspace containing all the reduced data, before grouping.")

        self.declareProperty(MultipleFileProperty('SampleRuns',
                                                  action=FileAction.OptionalLoad,
                                                  extensions=['nxs'],
                                                  allow_empty=False),
                             doc='Sample run(s).')

        self.declareProperty(MultipleFileProperty('AbsorberRuns',
                                                  action=FileAction.OptionalLoad,
                                                  extensions=['nxs']),
                             doc='Absorber (Cd/B4C) run(s).')

        self.declareProperty(MultipleFileProperty('ContainerRuns',
                                                  action=FileAction.OptionalLoad,
                                                  extensions=['nxs']),
                             doc='Empty container run(s).')

        self.setPropertyGroup('SampleRuns', 'Numors')
        self.setPropertyGroup('AbsorberRuns', 'Numors')
        self.setPropertyGroup('ContainerRuns', 'Numors')

        self.declareProperty(MultipleFileProperty('SensitivityMaps',
                                                  action=FileAction.OptionalLoad,
                                                  extensions=['nxs']),
                             doc='File containing the map of relative detector efficiencies.')

        self.declareProperty(MatrixWorkspaceProperty('SensitivityOutputWorkspace', '',
                                                     direction=Direction.Output,
                                                     optional=PropertyMode.Optional),
                             doc='The output sensitivity map workspace.')

        self.declareProperty(FileProperty('DefaultMaskFile', '', action=FileAction.OptionalLoad, extensions=['nxs']),
                             doc='File containing the default mask to be applied to all the detector configurations.')

        self.declareProperty(MultipleFileProperty('MaskFiles',
                                                  action=FileAction.OptionalLoad,
                                                  extensions=['nxs']),
                             doc='File(s) containing the beam stop and other detector mask.')

        self.copyProperties('SANSILLReduction', ['NormaliseBy'])

        self.declareProperty('SampleThickness', 0.1, validator=FloatBoundedValidator(lower=0.),
                             doc='Sample thickness [cm]')

        self.declareProperty('Observable', 'Omega.value',
                             doc='Parameter from the sample logs along which the scan is made')

        self.declareProperty('PixelYMin', 0, validator=IntBoundedValidator(lower=0),
                             doc='Minimal y-index taken in the integration')
        self.declareProperty('PixelYMax', 320, validator=IntBoundedValidator(lower=0),
                             doc='Maximal y-index taken in the integration')

        self.setPropertyGroup('SensitivityMaps', 'Options')
        self.setPropertyGroup('SensitivityOutputWorkspace', 'Options')
        self.setPropertyGroup('DefaultMaskFile', 'Options')
        self.setPropertyGroup('MaskFiles', 'Options')
        self.setPropertyGroup('NormaliseBy', 'Options')
        self.setPropertyGroup('SampleThickness', 'Options')
        self.setPropertyGroup('Observable', 'Options')
        self.setPropertyGroup('PixelYMin', 'Options')
        self.setPropertyGroup('PixelYMax', 'Options')

    def PyExec(self):

        self.setUp()

        _, load_ws_name = needs_loading(self.sample, "Load")
        Load(Filename=self.sample, OutputWorkspace=load_ws_name)
        ConjoinXRuns(InputWorkspaces=load_ws_name, OutputWorkspace="__joined", SampleLogAsXAxis=self.observable)
        mtd[load_ws_name].delete()

        sort_x_axis_output = 'sorted' if not self.output_reduced else self.output_reduced
        SortXAxis(InputWorkspace="__joined", OutputWorkspace="__" + sort_x_axis_output)
        mtd["__" + sort_x_axis_output].getAxis(0).setUnit("label").setLabel('Omega', 'degrees')

        load_sensitivity, sens_input = needs_loading(self.sensitivity, 'Sensitivity')
        self.progress.report('Loading sensitivity')
        if load_sensitivity:
            LoadNexusProcessed(Filename=self.sensitivity, OutputWorkspace=sens_input)

        load_mask, mask_input = needs_loading(self.mask, "Mask")
        self.progress.report('Loading mask')
        if load_mask:
            LoadNexusProcessed(Filename=self.mask, OutputWorkspace=mask_input)

        load_default_mask, default_mask_input = needs_loading(self.default_mask, "DefaultMask")
        self.progress.report('Loading default mask')
        if load_default_mask:
            LoadNexusProcessed(Filename=self.default_mask, OutputWorkspace=default_mask_input)

        process_absorber, absorber_name = needs_processing(self.absorber, 'Absorber')
        self.progress.report('Processing absorber')
        if process_absorber:
            SANSILLReduction(Run=self.absorber,
                             ProcessAs='Absorber',
                             NormaliseBy=self.normalise,
                             OutputWorkspace=absorber_name)

        process_container, container_name = needs_processing(self.container, 'Container')

        self.progress.report('Processing container')
        if process_container:
            SANSILLReduction(Run=self.container,
                             ProcessAs='Container',
                             OutputWorkspace=container_name,
                             AbsorberInputWorkspace=absorber_name,
                             CacheSolidAngle=True,
                             NormaliseBy=self.normalise)
        # even if the samples are provided as arguments, the SANSILLReduction won't use them and rather take the already
        # processed ws
        SANSILLReduction(Run=self.sample,
                         AbsorberInputWorkspace=absorber_name,
                         ContainerInputWorkspace=container_name,
                         SampleThickness=self.thickness,
                         SensitivityInputWorkspace=sens_input,
                         SensitivityOutputWorkspace=self.output_sens,
                         MaskedInputWorkspace=mask_input,
                         DefaultMaskedInputWorkspace=default_mask_input,
                         NormaliseBy=self.normalise,
                         OutputWorkspace=sort_x_axis_output)

        if self.output_reduced:
            self.setProperty('ReducedData', mtd[self.output_reduced])

        instrument = mtd["__joined"].getInstrument()
        detector = instrument.getComponentByName("detector")
        if "detector-width" in detector.getParameterNames() and "detector-height" in detector.getParameterNames():
            width = int(detector.getNumberParameter("detector-width")[0])
            height = int(detector.getNumberParameter("detector-height")[0])
        else:
            raise RuntimeError('No width or height found for this instrument. Unable to group detectors.')

        self.checkPixelY(height)
        grouping = create_detector_grouping(self.pixel_y_min, self.pixel_y_max, width, height)
        GroupDetectors(InputWorkspace=sort_x_axis_output,
                       OutputWorkspace=self.output2D,
                       GroupingPattern=grouping,
                       Behaviour="Average")
        Transpose(InputWorkspace=self.output2D, OutputWorkspace=self.output2D)

        if instrument.getName() == 'D16':
            ws = mtd[self.output2D]
            convert_spectrum_to_theta(ws, detector)
        else:
            ConvertSpectrumAxis(InputWorkspace=self.output2D,
                                OutputWorkspace=self.output2D,
                                Target="theta")

        self.setProperty('Output2D', mtd[self.output2D])
        self.cleanUp()


def convert_spectrum_to_theta(ws, detector):
    """
    Given a workspace with the X-axis in pixels, compute the 2 theta equivalent axis and set it as the new one,
    in the case of a flat D16 detector

    :param ws: the workspace to convert
    :param detector: the detector object associated with the instrument
    """
    if "x-pixel-size" in detector.getParameterNames():
        pixel_width = float(detector.getNumberParameter('x-pixel-size')[0])
    else:
        pixel_width = 1
        logger.warning("Pixel width not found for the instrument. {0} assumed.".format(pixel_width))

    run = ws.getRun()
    if run.hasProperty("Gamma.value"):
        theta_offset = run.getProperty("Gamma.value").value
    else:
        theta_offset = 0
        logger.warning("Detector angle not found. {0} assumed.".format(theta_offset))

    from numpy import arctan2, pi
    l2 = run.getPropertyAsSingleValue("L2")
    new_axis = [arctan2((i - 160) * pixel_width, 1000 * l2) * 180 / pi + theta_offset
                for i in range(ws.extractY()[0].size)]

    for i in range(ws.spectrumInfo().size()):
        ws.setX(i, new_axis)

    ws.getAxis(0).setUnit("degrees")


def create_detector_grouping(y_min, y_max, detector_width, detector_height):
    """
    Create the pixel grouping for the detector. Shape is assumed to be D16's.
    The pixel grouping consists of the vertical columns of pixels of the detector.
    :param y_min: index of the first line to take on each column.
    :param y_max: index of the last line to take on each column.
    :param detector_width: the total number of column of pixel on the detector.
    :param detector_height: the total number of lines of pixel on the detector.
    """
    grouping = []
    for i in range(detector_width):
        grouping.append(str(i * detector_height + y_min) + "-" + str(i * detector_height + y_max - 1))
    grouping = ",".join(grouping)
    return grouping


AlgorithmFactory.subscribe(SANSILLParameterScan)
