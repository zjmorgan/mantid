// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +

/********** PLEASE NOTE! THIS FILE WAS AUTO-GENERATED FROM CMAKE.
 * ***********************/
/********** Source = kernel.cpp.in
 * *******************************************************/

#include <boost/python/def.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/python/module.hpp>
#include <boost/python/type_id.hpp>
#include <Poco/Instantiator.h>
#include <Poco/LoggingFactory.h>

#include "MantidKernel/MantidVersion.h"
#include "MantidPythonInterface/core/NDArray.h"
#include "MantidPythonInterface/kernel/Registry/TypeRegistry.h"
#include "MantidPythonInterface/kernel/kernel.h"
#include "MantidPythonInterface/core/PythonStdoutChannel.h"

// See
// http://docs.scipy.org/doc/numpy/reference/c-api.array.html#PY_ARRAY_UNIQUE_SYMBOL
#define PY_ARRAY_UNIQUE_SYMBOL KERNEL_ARRAY_API
#include <numpy/arrayobject.h>

// See comment in kernel.h
#ifdef _WIN32
void kernel_dll_import_numpy_capi_for_unittest() { _import_array(); }
#endif

using boost::python::def;
using boost::python::type_id;
namespace converter = boost::python::converter;

// Forward declare
void export_ConfigService();
void export_ConfigObserver();
void export_ConfigPropertyObserver();
void export_DataItem();
void export_IPropertyManager();
void export_Property();
void export_IValidator();
void export_IPropertySettings();
void export_EnabledWhenProperty();
void export_ErrorReporter();
void export_VisibleWhenProperty();
void export_BasicPropertyWithValueTypes();
void export_ArrayProperty();
void export_Quat();
void export_V3D();
void export_VMD();
void exportStlContainers();
void export_Logger();
void export_Unit();
void export_Label();
void export_BoundedValidator();
void export_TimeSeriesProperty_Double();
void export_TimeSeriesProperty_Bool();
void export_TimeSeriesProperty_Int32();
void export_TimeSeriesProperty_Int64();
void export_TimeSeriesProperty_String();
void export_TimeSeriesPropertyStatistics();
void export_FilteredTimeSeriesProperty();
void export_DateAndTime();
void export_time_duration();
void export_InstrumentInfo();
void export_FacilityInfo();
void export_LiveListenerInfo();
void export_NullValidator();
void export_ListValidator();
void export_ArrayLengthValidator();
void export_ArrayBoundedValidator();
void export_ArrayOrderedPairsValidator();
void export_MandatoryValidator();
void export_CompositeValidator();
void export_LogFilter();
void export_UnitConversion();
void export_UnitFactory();
void export_UnitLabel();
void export_DeltaEMode();
void export_PropertyManager();
void export_PropertyManagerDataService();
void export_PropertyManagerProperty();
void export_PropertyHistory();
void export_MemoryStats();
void export_ProgressBase();
void export_Material();
void export_MaterialBuilder();
void export_Statistics();
void export_OptionalBoolValue();
void export_OptionalBool();
void export_PropertyWithValueOptionalBool();
void export_UsageService();
void export_Atom();
void export_StringContainsValidator();
void export_PropertyFactory();
void export_RebinParamsValidator();
void export_PhysicalConstants();

BOOST_PYTHON_MODULE(_kernel) {
  // Doc string options - User defined, python arguments, C++ call signatures
  boost::python::docstring_options docstrings(true, true, false);
  // Import numpy for the core DLL
  Mantid::PythonInterface::importNumpy();
  // Import numpy for this DLL
  _import_array();
  // register the python logger with poco
  Poco::LoggingFactory::defaultFactory().registerChannelClass(
    "PythonStdoutChannel",
    new Poco::Instantiator<Poco::PythonStdoutChannel, Poco::Channel>);

  def("version_str", &Mantid::Kernel::MantidVersion::version,
      "Returns the Mantid version string in the form \"major.minor.patch\"");
  def("release_notes_url", &Mantid::Kernel::MantidVersion::releaseNotes,
      "Returns the url to the most applicable release notes");
  def("revision", &Mantid::Kernel::MantidVersion::revision,
      "Returns the abbreviated SHA-1 of the last commit");
  def("revision_full", &Mantid::Kernel::MantidVersion::revisionFull,
      "Returns the full SHA-1 of the last commit");
  def("release_date", &Mantid::Kernel::MantidVersion::releaseDate,
      "Returns the date of the last commit");
  def("doi", &Mantid::Kernel::MantidVersion::doi,
      "Returns the DOI for this release of Mantid");
  def("paper_citation", &Mantid::Kernel::MantidVersion::paperCitation,
      "Returns The citation for the Mantid paper");

  Mantid::PythonInterface::Registry::TypeRegistry::registerBuiltins();

  export_ConfigService();
export_ConfigObserver();
export_ConfigPropertyObserver();
export_DataItem();
export_IPropertyManager();
export_Property();
export_IValidator();
export_IPropertySettings();
export_EnabledWhenProperty();
export_ErrorReporter();
export_VisibleWhenProperty();
export_BasicPropertyWithValueTypes();
export_ArrayProperty();
export_Quat();
export_V3D();
export_VMD();
exportStlContainers();
export_Logger();
export_Unit();
export_Label();
export_BoundedValidator();
export_TimeSeriesProperty_Double();
export_TimeSeriesProperty_Bool();
export_TimeSeriesProperty_Int32();
export_TimeSeriesProperty_Int64();
export_TimeSeriesProperty_String();
export_TimeSeriesPropertyStatistics();
export_FilteredTimeSeriesProperty();
export_DateAndTime();
export_time_duration();
export_InstrumentInfo();
export_FacilityInfo();
export_LiveListenerInfo();
export_NullValidator();
export_ListValidator();
export_ArrayLengthValidator();
export_ArrayBoundedValidator();
export_ArrayOrderedPairsValidator();
export_MandatoryValidator();
export_CompositeValidator();
export_LogFilter();
export_UnitConversion();
export_UnitFactory();
export_UnitLabel();
export_DeltaEMode();
export_PropertyManager();
export_PropertyManagerDataService();
export_PropertyManagerProperty();
export_PropertyHistory();
export_MemoryStats();
export_ProgressBase();
export_Material();
export_MaterialBuilder();
export_Statistics();
export_OptionalBoolValue();
export_OptionalBool();
export_PropertyWithValueOptionalBool();
export_UsageService();
export_Atom();
export_StringContainsValidator();
export_PropertyFactory();
export_RebinParamsValidator();
export_PhysicalConstants();
}
