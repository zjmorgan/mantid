// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +

/********** PLEASE NOTE! THIS FILE WAS AUTO-GENERATED FROM CMAKE.  ***********************/
/********** Source = curvefitting.cpp.in **********************************************************/

#include <boost/python/def.hpp>
#include <boost/python/module.hpp>
#include <boost/python/docstring_options.hpp>

// Forward declare
void export_ProductFunction();

BOOST_PYTHON_MODULE(_curvefitting)
{
  // Doc string options - User defined, python arguments, C++ call signatures
  boost::python::docstring_options docstrings(true, true, false);

export_ProductFunction();
}
