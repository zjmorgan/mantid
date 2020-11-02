# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#   NScD Oak Ridge National Laboratory, European Spallation Source,
#   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
# SPDX - License - Identifier: GPL - 3.0 +
import unittest
from mantid.api import WorkspaceGroup, MatrixWorkspace
from mantid.simpleapi import Load, config, mtd, D7AbsoluteCrossSections

class D7AbsoluteCrossSectionsTest(unittest.TestCase):

    _facility = None
    _instrument = None

    @classmethod
    def setUpClass(cls):
        cls._facility = config['default.facility']
        cls._instrument = config['default.instrument']

        config['default.facility'] = 'ILL'
        config['default.instrument'] = 'D7'

        config.appendDataSearchSubDir('ILL/D7/')
        Load('vanadium_uniaxial.nxs', OutputWorkspace='vanadium_uniaxial')
        Load('vanadium_xyz.nxs', OutputWorkspace='vanadium_xyz')
        Load('vanadium_10p.nxs', OutputWorkspace='vanadium_10p')
        Load('396993_reduced.nxs', OutputWorkspace='vanadium_data')
        Load('397004_reduced.nxs', OutputWorkspace='sample_data')

    def tearDown(self):
        if self._facility:
            config['default.facility'] = self._facility
        if self._instrument:
            config['default.instrument'] = self._instrument

    @classmethod
    def tearDownClass(cls):
        mtd.clear()

    def test_uniaxial_separation(self):
        D7AbsoluteCrossSections(InputWorkspace='vanadium_uniaxial', OutputWorkspace='_unused',
                                CrossSectionsOutputWorkspace='uniaxial',
                                CrossSectionSeparationMethod='Uniaxial')
        self._check_output('uniaxial', 1, 132, 2, onlySeparation=True)

    def test_xyz_separation(self):
        D7AbsoluteCrossSections(InputWorkspace='vanadium_xyz', OutputWorkspace='_unused',
                                CrossSectionsOutputWorkspace='xyz',
                                CrossSectionSeparationMethod='XYZ', NormalisationMethod='None')
        self._check_output('xyz', 1, 132, 3, onlySeparation=True)

    def test_10p_separation(self):
        D7AbsoluteCrossSections(InputWorkspace='vanadium_10p', OutputWorkspace='_unused',
                                CrossSectionsOutputWorkspace='10p',
                                CrossSectionSeparationMethod='10p', ThetaOffset=1.0,
                                NormalisationMethod='None')
        self._check_output('10p', 1, 132, 3, onlySeparation=True)

    def test_10p_separation_double_xyz(self):
        D7AbsoluteCrossSections(InputWorkspace='vanadium_xyz', RotatedXYZWorkspace='vanadium_xyz',
                                CrossSectionsOutputWorkspace='10p_double_xyz', OutputWorkspace='_unused',
                                CrossSectionSeparationMethod='10p', ThetaOffset=1.0,
                                NormalisationMethod='None')
        self._check_output('10p_double_xyz', 1, 132, 3, onlySeparation=True)

    def test_vanadium_normalisation(self):
        D7AbsoluteCrossSections(InputWorkspace='sample_data', OutputWorkspace='normalised_sample_vanadium',
                                CrossSectionSeparationMethod='XYZ', NormalisationMethod='Vanadium',
                                VanadiumInputWorkspace='vanadium_data', AbsoluteUnitsNormalisation=False)
        self._check_output('normalised_sample_vanadium', 132, 1, 6, onlySeparation=False)

    def test_paramagnetic_normalisation(self):
        D7AbsoluteCrossSections(InputWorkspace='sample_data', OutputWorkspace='normalised_sample_magnetic',
                                CrossSectionSeparationMethod='XYZ', NormalisationMethod='Paramagnetic',
                                SampleSpin = 0.5, AbsoluteUnitsNormalisation=False)
        self._check_output('normalised_sample_magnetic', 132, 1, 6, onlySeparation=False)

    def test_incoherent_normalisation(self):
        D7AbsoluteCrossSections(InputWorkspace='sample_data', OutputWorkspace='normalised_sample_incoherent',
                                CrossSectionSeparationMethod='XYZ', NormalisationMethod='Incoherent',
                                AbsoluteUnitsNormalisation=False)
        self._check_output('normalised_sample_incoherent', 132, 1, 3, onlySeparation=False)

    def _check_output(self, ws, blocksize, spectra, nEntries, onlySeparation):
        self.assertTrue(mtd[ws])
        self.assertTrue(isinstance(mtd[ws], WorkspaceGroup))
        self.assertTrue(mtd[ws].getNumberOfEntries(), nEntries)
        for entry in mtd[ws]:
            self.assertTrue(isinstance(entry, MatrixWorkspace))
            if onlySeparation:
                name = entry.name()
                name = name[name.rfind("_")+1:]
                self.assertTrue(name in ['Coherent', 'Incoherent', 'Magnetic'])
                self.assertTrue(entry.isHistogramData())
                self.assertTrue(not entry.isDistribution())
            else:
                self.assertTrue(not entry.isHistogramData())
                self.assertTrue(not entry.isDistribution())
            self.assertEqual(entry.blocksize(), blocksize)
            self.assertEqual(entry.getNumberHistograms(), spectra)
            self.assertTrue(entry.getHistory())

if __name__ == '__main__':
    unittest.main()
