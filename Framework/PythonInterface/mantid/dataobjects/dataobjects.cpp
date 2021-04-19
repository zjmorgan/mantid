// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +

/********** PLEASE NOTE! THIS FILE WAS AUTO-GENERATED FROM CMAKE.  ***********************/
/********** Source = dataobjects.cpp.in **********************************************************/

#include <boost/python/def.hpp>
#include <boost/python/module.hpp>
#include <boost/python/docstring_options.hpp>

// See http://docs.scipy.org/doc/numpy/reference/c-api.array.html#PY_ARRAY_UNIQUE_SYMBOL
#define PY_ARRAY_UNIQUE_SYMBOL DATAOBJECTS_ARRAY_API
#include <numpy/arrayobject.h>

// Forward declare
void export_EventList();
void export_EventWorkspace();
void export_EventWorkspaceProperty();
void export_Workspace2D();
void export_RebinnedOutput();
void export_SpecialWorkspace2D();
void export_GroupingWorkspace();
void export_MaskWorkspace();
void export_MaskWorkspaceProperty();
void export_OffsetsWorkspace();
void export_MDEventWorkspaces();
void export_MDHistoWorkspace();
void export_PeaksWorkspace();
void export_LeanElasticPeaksWorkspace();
void export_PeaksWorkspaceProperty();
void export_TableWorkspace();
void export_SplittersWorkspace();
void export_WorkspaceSingleValue();

BOOST_PYTHON_MODULE(_dataobjects)
{
  // Doc string options - User defined, python arguments, C++ call signatures
  boost::python::docstring_options docstrings(true, true, false);
  // Import numpy
  _import_array();

export_EventList();
export_EventWorkspace();
export_EventWorkspaceProperty();
export_Workspace2D();
export_RebinnedOutput();
export_SpecialWorkspace2D();
export_GroupingWorkspace();
export_MaskWorkspace();
export_MaskWorkspaceProperty();
export_OffsetsWorkspace();
export_MDEventWorkspaces();
export_MDHistoWorkspace();
export_PeaksWorkspace();
export_LeanElasticPeaksWorkspace();
export_PeaksWorkspaceProperty();
export_TableWorkspace();
export_SplittersWorkspace();
export_WorkspaceSingleValue();
}
