// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +

/********** PLEASE NOTE! THIS FILE WAS AUTO-GENERATED FROM CMAKE.  ***********************/
/********** Source = geometry.cpp.in *****************************************************/

#include <boost/python/module.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/python/def.hpp>

// See http://docs.scipy.org/doc/numpy/reference/c-api.array.html#PY_ARRAY_UNIQUE_SYMBOL
#define PY_ARRAY_UNIQUE_SYMBOL GEOMETRY_ARRAY_API
#include <numpy/arrayobject.h>

// Forward declare
void export_BoundingBox();
void export_IComponent();
void export_ICompAssembly();
void export_IObject();
void export_IObjComponent();
void export_IDetector();
void export_IMDDimension();
void export_Component();
void export_CompAssembly();
void export_ObjComponent();
void export_ObjCompAssembly();
void export_Detector();
void export_DetectorGroup();
void export_GridDetector();
void export_GridDetectorPixel();
void export_RectangularDetector();
void export_Instrument();
void export_UnitCell();
void export_MDFrame();
void export_OrientedLattice();
void export_ReferenceFrame();
void export_Goniometer();
void export_Object();
void export_PeakShape();
void export_Group();
void export_PointGroup();
void export_PointGroupFactory();
void export_SpaceGroup();
void export_SpaceGroupFactory();
void export_SymmetryElement();
void export_SymmetryElementFactory();
void export_SymmetryOperation();
void export_SymmetryOperationFactory();
void export_CrystalStructure();
void export_ReflectionGenerator();
void export_DetectorInfo();
void export_DetectorInfoItem();
void export_DetectorInfoPythonIterator();
void export_ComponentInfo();
void export_ComponentInfoItem();
void export_ComponentInfoPythonIterator();

BOOST_PYTHON_MODULE(_geometry)
{
  // Doc string options - User defined, python arguments, C++ call signatures
  boost::python::docstring_options docstrings(true, true, false);
  // Import numpy
  _import_array();

export_BoundingBox();
export_IComponent();
export_ICompAssembly();
export_IObject();
export_IObjComponent();
export_IDetector();
export_IMDDimension();
export_Component();
export_CompAssembly();
export_ObjComponent();
export_ObjCompAssembly();
export_Detector();
export_DetectorGroup();
export_GridDetector();
export_GridDetectorPixel();
export_RectangularDetector();
export_Instrument();
export_UnitCell();
export_MDFrame();
export_OrientedLattice();
export_ReferenceFrame();
export_Goniometer();
export_Object();
export_PeakShape();
export_Group();
export_PointGroup();
export_PointGroupFactory();
export_SpaceGroup();
export_SpaceGroupFactory();
export_SymmetryElement();
export_SymmetryElementFactory();
export_SymmetryOperation();
export_SymmetryOperationFactory();
export_CrystalStructure();
export_ReflectionGenerator();
export_DetectorInfo();
export_DetectorInfoItem();
export_DetectorInfoPythonIterator();
export_ComponentInfo();
export_ComponentInfoItem();
export_ComponentInfoPythonIterator();
}
