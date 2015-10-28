#include "MantidMDAlgorithms/AccumulateMD.h"
#include "MantidAPI/FrameworkManager.h"
#include "MantidKernel/ArrayProperty.h"
#include "MantidKernel/ArrayBoundedValidator.h"
#include "MantidKernel/CompositeValidator.h"
#include "MantidKernel/ListValidator.h"
#include "MantidKernel/MandatoryValidator.h"
#include "MantidKernel/PropertyWithValue.h"
#include "MantidAPI/FileFinder.h"
#include "MantidAPI/HistoryView.h"
#include "MantidDataObjects/MDHistoWorkspaceIterator.h"
#include <Poco/File.h>
#include <boost/algorithm/string.hpp>
#include "MantidAPI/IMDEventWorkspace.h"

using namespace Mantid::Kernel;
using namespace Mantid::API;
using namespace Mantid::DataObjects;

namespace Mantid {
namespace MDAlgorithms {

/*
Return names of data sources which are found as a workspace or file.
*/
void filterToExistingSources(std::vector<std::string> &input_data,
                             std::vector<double> &psi, std::vector<double> &gl,
                             std::vector<double> &gs,
                             std::vector<double> &efix) {
  for (unsigned long i = input_data.size(); i > 0; i--) {
    if (!dataExists(input_data[i - 1])) {
      input_data.erase(input_data.begin() + i - 1);
      psi.erase(psi.begin() + i - 1);
      gl.erase(gl.begin() + i - 1);
      gs.erase(gs.begin() + i - 1);
      efix.erase(efix.begin() + i - 1);
    }
  }
}

/*
Return true if dataName is an existing workspace or file
*/
bool dataExists(const std::string &data_name) {
  const std::string filepath =
      Mantid::API::FileFinder::Instance().getFullPath(data_name);
  // Calls to the ADS in algorithms like this should ordinarily
  // be avoided, unfortunately we have little choice in this case.
  // If we gave FileFinder an absolute path it just returns it (whether or not
  // the file exists) so we must check the full path returned with
  // fileExists()
  return (AnalysisDataService::Instance().doesExist(data_name) ||
          fileExists(filepath));
}

/*
Test if a file with this full path exists
*/
bool fileExists(const std::string &filename) {
  if (filename.empty())
    return false;
  Poco::File test_file(filename);
  return test_file.exists();
}

/*
Remove anything from input_data which is already in current_data
*/
void filterToNew(std::vector<std::string> &input_data,
                 std::vector<std::string> &current_data,
                 std::vector<double> &psi, std::vector<double> &gl,
                 std::vector<double> &gs, std::vector<double> &efix) {
  for (unsigned long i = input_data.size(); i > 0; i--) {
    if (appearsInCurrentData(input_data[i - 1], current_data)) {
      input_data.erase(input_data.begin() + i - 1);
      psi.erase(psi.begin() + i - 1);
      gl.erase(gl.begin() + i - 1);
      gs.erase(gs.begin() + i - 1);
      efix.erase(efix.begin() + i - 1);
    }
  }
}

bool appearsInCurrentData(const std::string &data_source,
                          std::vector<std::string> &current_data) {
  for (auto reverse_iter = current_data.rbegin();
       reverse_iter != current_data.rend(); ++reverse_iter) {
    if (data_source == *reverse_iter) {
      return true;
    }
  }
  return false;
}

/*
Return a vector of the names of files and workspaces which have previously added
to the workspace
*/
std::vector<std::string>
getHistoricalDataSources(const WorkspaceHistory &ws_history) {
  // Using a set so we only insert unique names
  std::set<std::string> historical_data_sources;

  // Get previously added data sources from DataSources property of the original
  // call of CreateMD and any subsequent calls of AccumulateMD
  auto view = ws_history.createView();
  view->unrollAll();
  const std::vector<HistoryItem> history_items = view->getAlgorithmsList();
  for (auto iter = history_items.begin(); iter != history_items.end(); ++iter) {
    auto alg_history = iter->getAlgorithmHistory();
    if (alg_history->name() == "CreateMD" ||
        alg_history->name() == "AccumulateMD") {
      auto props = alg_history->getProperties();
      for (auto prop_iter = props.begin(); prop_iter != props.end();
           ++prop_iter) {
        PropertyHistory_const_sptr prop_history = *prop_iter;
        if (prop_history->name() == "DataSources") {
          insertDataSources(prop_history->value(), historical_data_sources);
        }
      }
    }
  }

  std::vector<std::string> result(historical_data_sources.begin(),
                                  historical_data_sources.end());
  return result;
}

/*
Split string of data sources from workspace history and insert them into
complete set of historical data sources
*/
void insertDataSources(const std::string &data_sources,
                       std::set<std::string> &historical_data_sources) {
  // Split the property string into a vector of data sources
  std::vector<std::string> data_split;
  boost::split(data_split, data_sources, boost::is_any_of(","));

  // Trim any whitespace from ends of each data source string
  std::for_each(
      data_split.begin(), data_split.end(),
      boost::bind(boost::algorithm::trim<std::string>, _1, std::locale()));

  // Insert each data source into our complete set of historical data sources
  for (auto it = data_split.begin(); it != data_split.end(); ++it) {
    historical_data_sources.insert(*it);
  }
}

void padParameterVector(std::vector<double> &param_vector,
                        unsigned long grow_to_size) {
  if (param_vector.size() == 0) {
    param_vector.resize(grow_to_size, 0.0);
  } else if (param_vector.size() == 1) {
    param_vector.resize(grow_to_size, param_vector[0]);
  }
}

// Register the algorithm into the AlgorithmFactory
DECLARE_ALGORITHM(AccumulateMD)

/*
 Constructor
 */
AccumulateMD::AccumulateMD() {}

/*
 Destructor
 */
AccumulateMD::~AccumulateMD() {}

/// Algorithms name for identification. @see Algorithm::name
const std::string AccumulateMD::name() const { return "AccumulateMD"; }

/// Algorithm's version for identification. @see Algorithm::version
int AccumulateMD::version() const { return 1; }

/// Algorithm's category for identification. @see Algorithm::category
const std::string AccumulateMD::category() const { return "MDAlgorithms"; }

/// Algorithm's summary for use in the GUI and help. @see Algorithm::summary
const std::string AccumulateMD::summary() const {
  return "Add new data to an existing MDHistoWorkspace";
}

/*
 Initialize the algorithm's properties.
 */
void AccumulateMD::init() {
  declareProperty(new WorkspaceProperty<IMDEventWorkspace>("InputWorkspace", "",
                                                           Direction::Input),
                  "An input MDHistoWorkspace to append data to.");

  declareProperty(new WorkspaceProperty<IMDEventWorkspace>(
                      "OutputWorkspace", "", Direction::Output),
                  "MDHistoWorkspace with new data appended.");

  declareProperty(
      new ArrayProperty<std::string>(
          "DataSources",
          boost::make_shared<MandatoryValidator<std::vector<std::string>>>(),
          Direction::Input),
      "Input workspaces to process, or filenames to load and process");

  declareProperty(new ArrayProperty<double>("EFix", Direction::Input),
                  "datasource energy values in meV");

  std::vector<std::string> e_mode_options;
  e_mode_options.push_back("Elastic");
  e_mode_options.push_back("Direct");
  e_mode_options.push_back("Indirect");

  declareProperty("Emode", "Direct",
                  boost::make_shared<StringListValidator>(e_mode_options),
                  "Analysis mode ['Elastic', 'Direct', 'Indirect'].");

  declareProperty(
      new ArrayProperty<double>(
          "Alatt",
          boost::make_shared<MandatoryValidator<std::vector<double>>>(),
          Direction::Input),
      "Lattice parameters");

  declareProperty(
      new ArrayProperty<double>(
          "Angdeg",
          boost::make_shared<MandatoryValidator<std::vector<double>>>(),
          Direction::Input),
      "Lattice angles");

  declareProperty(
      new ArrayProperty<double>(
          "u", boost::make_shared<MandatoryValidator<std::vector<double>>>(),
          Direction::Input),
      "Lattice vector parallel to neutron beam");

  declareProperty(
      new ArrayProperty<double>(
          "v", boost::make_shared<MandatoryValidator<std::vector<double>>>(),
          Direction::Input),
      "Lattice vector perpendicular to neutron beam in the horizontal plane");

  declareProperty(new ArrayProperty<double>("Psi", Direction::Input),
                  "Psi rotation in degrees. Optional or one entry per run.");

  declareProperty(new ArrayProperty<double>("Gl", Direction::Input),
                  "gl rotation in degrees. Optional or one entry per run.");

  declareProperty(new ArrayProperty<double>("Gs", Direction::Input),
                  "gs rotation in degrees. Optional or one entry per run.");

  declareProperty(
      new PropertyWithValue<bool>("InPlace", false, Direction::Input),
      "Execute conversions to MD and Merge in one-step. Less "
      "memory overhead.");

  declareProperty(new PropertyWithValue<bool>("Clean", false, Direction::Input),
                  "Create workspace from fresh rather than appending to "
                  "existing workspace.");
}

/*
 Execute the algorithm.
 */
void AccumulateMD::exec() {

  IMDEventWorkspace_sptr input_ws = this->getProperty("InputWorkspace");
  std::vector<std::string> input_data = this->getProperty("DataSources");

  std::vector<double> psi = this->getProperty("Psi");
  padParameterVector(psi, input_data.size());
  std::vector<double> gl = this->getProperty("Gl");
  padParameterVector(gl, input_data.size());
  std::vector<double> gs = this->getProperty("Gs");
  padParameterVector(gs, input_data.size());
  std::vector<double> efix = this->getProperty("EFix");
  padParameterVector(efix, input_data.size());

  filterToExistingSources(input_data, psi, gl, gs, efix);

  // If we can't find any data, we can't do anything
  if (input_data.empty()) {
    g_log.warning() << "No data found matching input in " << this->name()
                    << std::endl;
    this->setProperty("OutputWorkspace", input_ws);
    return; // POSSIBLE EXIT POINT
  }
  this->interruption_point();

  // If Clean=True then just call CreateMD to create a fresh workspace and
  // delete the old one, note this means we don't retain workspace history...
  bool do_clean = this->getProperty("Clean");
  if (do_clean) {
    IMDWorkspace_sptr out_ws = createMDWorkspace(input_data, psi, gl, gs, efix);
    this->setProperty("OutputWorkspace", out_ws);
    g_log.notice() << this->name() << " succesfully created a clean workspace"
                   << std::endl;
    return; // POSSIBLE EXIT POINT
  }
  this->interruption_point();

  // Find what files and workspaces have already been included in the workspace.
  const WorkspaceHistory ws_history = input_ws->getHistory();
  std::vector<std::string> current_data = getHistoricalDataSources(ws_history);

  // If there's no new data, we don't have anything to do
  filterToNew(input_data, current_data, psi, gl, gs, efix);
  if (input_data.empty()) {
    g_log.notice() << "No new data to append to workspace in " << this->name()
                   << std::endl;
    this->setProperty("OutputWorkspace", input_ws);
    return; // POSSIBLE EXIT POINT
  }
  this->interruption_point();

  // If we reach here then new data exists to append to the input workspace
  // Use CreateMD with the new data to make a temp workspace
  // Merge the temp workspace with the input workspace using MergeMD
  IMDWorkspace_sptr tmp_ws = createMDWorkspace(input_data, psi, gl, gs, efix);
  this->interruption_point();

  const std::string temp_ws_name = "TEMP_WORKSPACE_ACCUMULATEMD";
  // Currently have to us ADS here as list of workspaces can only be passed as a
  // string of names
  AnalysisDataService::Instance().add(temp_ws_name, tmp_ws);
  std::string ws_names_to_merge = input_ws->getName();
  ws_names_to_merge.append(",");
  ws_names_to_merge.append(temp_ws_name);

  Algorithm_sptr merge_alg = createChildAlgorithm("MergeMD");
  merge_alg->setProperty("InputWorkspaces", ws_names_to_merge);
  merge_alg->executeAsChildAlg();

  API::IMDEventWorkspace_sptr out_ws =
      merge_alg->getProperty("OutputWorkspace");

  this->setProperty("OutputWorkspace", out_ws);
  g_log.notice() << this->name() << " successfully appended data" << std::endl;

  // Clean up temporary workspace
  AnalysisDataService::Instance().remove(temp_ws_name);
}

/*
 Use the CreateMD algorithm to create an MD workspace
 */
IMDWorkspace_sptr AccumulateMD::createMDWorkspace(
    const std::vector<std::string> &data_sources,
    const std::vector<double> &psi, const std::vector<double> &gl,
    const std::vector<double> &gs, const std::vector<double> &efix) {

  Algorithm_sptr create_alg = createChildAlgorithm("CreateMD");

  create_alg->setProperty("DataSources", data_sources);
  create_alg->setProperty("EFix", efix);
  create_alg->setPropertyValue("EMode", this->getPropertyValue("EMode"));
  create_alg->setPropertyValue("Alatt", this->getPropertyValue("Alatt"));
  create_alg->setPropertyValue("Angdeg", this->getPropertyValue("Angdeg"));
  create_alg->setPropertyValue("u", this->getPropertyValue("u"));
  create_alg->setPropertyValue("v", this->getPropertyValue("v"));
  create_alg->setProperty("Psi", psi);
  create_alg->setProperty("Gl", gl);
  create_alg->setProperty("Gs", gs);
  create_alg->setPropertyValue("InPlace", this->getPropertyValue("InPlace"));
  create_alg->executeAsChildAlg();

  return create_alg->getProperty("OutputWorkspace");
}

} // namespace MDAlgorithms
} // namespace Mantid
