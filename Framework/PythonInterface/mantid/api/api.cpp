// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +

/********** PLEASE NOTE! THIS FILE WAS AUTO-GENERATED FROM CMAKE.  ***********************/
/********** Source = api.cpp.in **********************************************************/

#include <boost/python/def.hpp>
#include <boost/python/module.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/python/args.hpp>


// See http://docs.scipy.org/doc/numpy/reference/c-api.array.html#PY_ARRAY_UNIQUE_SYMBOL
#define PY_ARRAY_UNIQUE_SYMBOL API_ARRAY_API
#include <numpy/arrayobject.h>

#include "MantidAPI/AlgorithmFactory.h"
#include "MantidAPI/Workspace.h"

namespace
{

  /**
   * Checks if two workspace shared pointers point to the same workspace
   */
  bool isSameWorkspaceObject(Mantid::API::Workspace_sptr workspace1, 
		             Mantid::API::Workspace_sptr workspace2) {
    return workspace1 == workspace2;
  }

}

// Forward declare
void export_ialgorithm();
void export_algorithm_observer();
void export_AlgorithmHistory();
void export_CatalogManager();
void export_CatalogSession();
void export_DeprecatedAlgorithmChecker();
void export_leaf_classes();
void export_SerialAlgorithm();
void export_ParallelAlgorithm();
void export_DistributedAlgorithm();
void export_DataProcessorAlgorithm();
void export_SerialDataProcessorAlgorithm();
void export_ParallelDataProcessorAlgorithm();
void export_DistributedDataProcessorAlgorithm();
void export_AlgorithmFactory();
void export_AlgorithmFactoryObserver();
void export_AlgorithmManager();
void export_AnalysisDataService();
void export_ActionEnum();
void export_FileProperty();
void exportInstrumentFileFinder();
void export_MultipleFileProperty();
void export_FrameworkManager();
void export_ISpectrum();
void export_IEventList();
void export_WorkspaceHistory();
void export_ExperimentInfo();
void export_MultipleExperimentInfos();
void export_Workspace();
void export_IWorkspaceProperty();
void export_WorkspaceProperty();
void export_ITableWorkspace();
void export_ITableWorkspaceProperty();
void export_ISplittersWorkspace();
void export_MDGeometry();
void export_IMDWorkspace();
void export_IMDWorkspaceProperty();
void export_IMDHistoWorkspace();
void export_IMDHistoWorkspaceProperty();
void export_IMDEventWorkspace();
void export_IMDEventWorkspaceProperty();
void export_MatrixWorkspace();
void export_MatrixWorkspaceProperty();
void export_IEventWorkspace();
void export_IEventWorkspaceProperty();
void export_IMaskWorkspace();
void export_IPeaksWorkspaceIterator();
void export_IPeaksWorkspace();
void export_IPeaksWorkspaceProperty();
void export_BinaryOperations();
void export_WorkspaceGroup();
void export_WorkspaceGroupProperty();
void export_MatrixWorkspaceValidator();
void export_WorkspaceValidators();
void export_ADSValidator();
void export_InstrumentValidator();
void export_OrientedLatticeValidator();
void export_Axis();
void export_SpectraAxis();
void export_NumericAxis();
void export_BinEdgeAxis();
void export_TextAxis();
void export_IPeak();
void export_BoxController();
void export_FileFinder();
void export_FileLoaderRegistry();
void export_Sample();
void export_ScriptRepository();
void export_ScriptRepositoryFactory();
void export_SpectrumInfo();
void export_Run();
void export_WorkspaceFactory();
void export_IFunction();
void export_IFunction1D();
void export_IPeakFunction();
void export_CompositeFunction();
void export_Jacobian();
void export_FunctionFactory();
void export_Progress();
void export_Projection();
void export_FunctionProperty();
void export_AlgorithmProperty();
void export_MultiDomainFunction();
void export_SpectrumDefinition();
void export_SpectrumInfoItem();
void export_SpectrumInfoIterator();
void export_SpectrumInfoPythonIterator();
void export_AnalysisDataServiceObserver();
void export_Citation();
void export_IPreview();
void export_PreviewManager();

BOOST_PYTHON_MODULE(_api)
{
  // Doc string options - User defined, python arguments, C++ call signatures
  boost::python::docstring_options docstrings(true, true, false);
  // Import numpy
  _import_array();

  // Workspace address comparison
  boost::python::def("isSameWorkspaceObject",
		     &isSameWorkspaceObject, 
		     boost::python::args("workspace1", "workspace2"));

  try {
export_ialgorithm();
export_algorithm_observer();
export_AlgorithmHistory();
export_CatalogManager();
export_CatalogSession();
export_DeprecatedAlgorithmChecker();
export_leaf_classes();
export_SerialAlgorithm();
export_ParallelAlgorithm();
export_DistributedAlgorithm();
export_DataProcessorAlgorithm();
export_SerialDataProcessorAlgorithm();
export_ParallelDataProcessorAlgorithm();
export_DistributedDataProcessorAlgorithm();
export_AlgorithmFactory();
export_AlgorithmFactoryObserver();
export_AlgorithmManager();
export_AnalysisDataService();
export_ActionEnum();
export_FileProperty();
exportInstrumentFileFinder();
export_MultipleFileProperty();
export_FrameworkManager();
export_ISpectrum();
export_IEventList();
export_WorkspaceHistory();
export_ExperimentInfo();
export_MultipleExperimentInfos();
export_Workspace();
export_IWorkspaceProperty();
export_WorkspaceProperty();
export_ITableWorkspace();
export_ITableWorkspaceProperty();
export_ISplittersWorkspace();
export_MDGeometry();
export_IMDWorkspace();
export_IMDWorkspaceProperty();
export_IMDHistoWorkspace();
export_IMDHistoWorkspaceProperty();
export_IMDEventWorkspace();
export_IMDEventWorkspaceProperty();
export_MatrixWorkspace();
export_MatrixWorkspaceProperty();
export_IEventWorkspace();
export_IEventWorkspaceProperty();
export_IMaskWorkspace();
export_IPeaksWorkspaceIterator();
export_IPeaksWorkspace();
export_IPeaksWorkspaceProperty();
export_BinaryOperations();
export_WorkspaceGroup();
export_WorkspaceGroupProperty();
export_MatrixWorkspaceValidator();
export_WorkspaceValidators();
export_ADSValidator();
export_InstrumentValidator();
export_OrientedLatticeValidator();
export_Axis();
export_SpectraAxis();
export_NumericAxis();
export_BinEdgeAxis();
export_TextAxis();
export_IPeak();
export_BoxController();
export_FileFinder();
export_FileLoaderRegistry();
export_Sample();
export_ScriptRepository();
export_ScriptRepositoryFactory();
export_SpectrumInfo();
export_Run();
export_WorkspaceFactory();
export_IFunction();
export_IFunction1D();
export_IPeakFunction();
export_CompositeFunction();
export_Jacobian();
export_FunctionFactory();
export_Progress();
export_Projection();
export_FunctionProperty();
export_AlgorithmProperty();
export_MultiDomainFunction();
export_SpectrumDefinition();
export_SpectrumInfoItem();
export_SpectrumInfoIterator();
export_SpectrumInfoPythonIterator();
export_AnalysisDataServiceObserver();
export_Citation();
export_IPreview();
export_PreviewManager();
  } catch(boost::python::error_already_set&) {
  PyErr_Print();
  exit(1);
}

}
