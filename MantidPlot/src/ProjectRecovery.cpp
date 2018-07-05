#include "ProjectRecovery.h"

#include "ApplicationWindow.h"
#include "Folder.h"
#include "ProjectSerialiser.h"
#include "ScriptingWindow.h"

#include "MantidAPI/FileProperty.h"
#include "MantidAPI/FrameworkManager.h"
#include "MantidKernel/ConfigService.h"
#include "MantidKernel/Logger.h"
#include "MantidKernel/UsageService.h"

#include "boost/optional.hpp"

#include "Poco/DirectoryIterator.h"
#include "Poco/NObserver.h"
#include "Poco/Path.h"

#include <QMessageBox>
#include <QObject>
#include <QString>
#include <QMetaObject>

#include <chrono>
#include <condition_variable>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

namespace {
Mantid::Kernel::Logger g_log("ProjectRecovery");

// Config helper methods
template <typename T>
boost::optional<T> getConfigValue(const std::string &key) {
  T returnedValue;

  int valueIsGood =
      Mantid::Kernel::ConfigService::Instance().getValue<T>(key, returnedValue);

  if (valueIsGood != 1) {
    return boost::optional<T>{};
  }

  return boost::optional<T>{returnedValue};
}

boost::optional<bool> getConfigBool(const std::string &key) {
  auto returnedValue = getConfigValue<std::string>(key);
  if (!returnedValue.is_initialized()) {
    return boost::optional<bool>{};
  }

  return returnedValue->find("true") != std::string::npos;
}

/// Returns a string to the current top level recovery folder
std::string getRecoveryFolder() {
  static std::string recoverFolder =
      Mantid::Kernel::ConfigService::Instance().getAppDataDir() + "/recovery/";
  return recoverFolder;
}

/// Gets a formatted timestamp
std::string getTimeStamp() {
  const char *formatSpecifier = "%Y-%m-%dT%H-%M-%S";
  auto time = std::time(nullptr);
  auto localTime = std::localtime(&time);

#if __GNUG__ && __GNUG__ < 5
  // Have to workaround GCC 4 not having std::put_time on RHEL7
  // this ifdef can be removed when RHEL7 uses a newer compiler
  char timestamp[20];
  if (strftime(timestamp, sizeof(timestamp), formatSpecifier, localTime) > 0) {
    return {timestamp};
  }

  return {};
#else
  std::ostringstream timestamp;
  timestamp << std::put_time(localTime, formatSpecifier);
  return timestamp.str();
#endif
}

/// Returns a string to the current timestamped recovery folder
Poco::Path getOutputPath() {

  auto timestamp = getTimeStamp();
  auto timestampedPath = getRecoveryFolder().append(timestamp);

  return Poco::Path{timestampedPath};
}

std::vector<Poco::Path>
getRecoveryFolderCheckpoints(const std::string &recoveryFolderPath) {
  Poco::Path recoveryPath;
  if (!recoveryPath.tryParse(recoveryFolderPath)) {
    // Folder may not exist yet
    g_log.debug("Project Saving: Failed to get working folder whilst deleting "
                "checkpoints");
    return {};
  }

  std::vector<Poco::Path> folderPaths;

  Poco::DirectoryIterator dirIterator(recoveryFolderPath);
  Poco::DirectoryIterator end;
  // Find all the folders which exist in this folder
  while (dirIterator != end) {
    std::string iterPath = recoveryFolderPath + dirIterator.name() + '/';
    Poco::Path foundPath(iterPath);

    if (foundPath.isDirectory()) {
      folderPaths.push_back(std::move(foundPath));
    }
    ++dirIterator;
  }

  // Ensure the oldest is first in the vector
  std::sort(folderPaths.begin(), folderPaths.end(),
            [](const Poco::Path &a, const Poco::Path &b) {
              return a.toString() < b.toString();
            });

  return folderPaths;
}

const std::string OUTPUT_PROJ_NAME = "recovery.mantid";

// Config keys
const std::string SAVING_ENABLED_CONFIG_KEY = "projectRecovery.enabled";
const std::string SAVING_TIME_KEY = "projectRecovery.secondsBetween";
const std::string NO_OF_CHECKPOINTS_KEY = "projectRecovery.numberOfCheckpoints";

// Config values
bool SAVING_ENABLED =
    getConfigBool(SAVING_ENABLED_CONFIG_KEY).get_value_or(false);
const int SAVING_TIME =
    getConfigValue<int>(SAVING_TIME_KEY).get_value_or(60); // Seconds
const int NO_OF_CHECKPOINTS =
    getConfigValue<int>(NO_OF_CHECKPOINTS_KEY).get_value_or(5);

// Implementation variables
const std::chrono::seconds TIME_BETWEEN_SAVING(SAVING_TIME);

} // namespace

namespace MantidQt {

/**
 * Constructs a new ProjectRecovery, a class which encapsulates
 * a background thread to save periodically. This does not start the
 * background thread though
 *
 * @param windowHandle :: Pointer to the main application window
 */
ProjectRecovery::ProjectRecovery(ApplicationWindow *windowHandle)
    : m_backgroundSavingThread(), m_stopBackgroundThread(true),
      m_configKeyObserver(*this, &ProjectRecovery::configKeyChanged),
      m_windowPtr(windowHandle) {}

/// Destructor which also stops any background threads currently in progress
ProjectRecovery::~ProjectRecovery() { stopProjectSaving(); }

bool ProjectRecovery::attemptRecovery() {
  QString recoveryMsg = QObject::tr(
      "Mantid did not close correctly and a recovery"
      " checkpoint has been found. Would you like to attempt recovery?");

  int userChoice = QMessageBox::information(
      m_windowPtr, QObject::tr("Project Recovery"), recoveryMsg,
      QObject::tr("Open in script editor"), QObject::tr("No"), 0, 1);

  if (userChoice == 1) {
    // User selected no
    return true;
  }

  const auto checkpointPaths =
      getRecoveryFolderCheckpoints(getRecoveryFolder());
  auto &mostRecentCheckpoint = checkpointPaths.back();

  // TODO automated recovery
  switch (userChoice) {
  case 0:
    return openInEditor(mostRecentCheckpoint);
  default:
    throw std::runtime_error("Unknown choice in ProjectRecovery");
  }
}

bool ProjectRecovery::checkForRecovery() const {
  const auto checkpointPaths =
      getRecoveryFolderCheckpoints(getRecoveryFolder());
  return checkpointPaths.size() != 0; // Non zero indicates recovery is pending
}

/// Returns a background thread with the current object captured inside it
std::thread ProjectRecovery::createBackgroundThread() {
  // Using a lambda helps the compiler deduce the this pointer
  // otherwise the resolution is ambiguous
  return std::thread([this] { projectSavingThreadWrapper(); });
}

/// Callback for POCO when a config change had fired for the enabled key
void ProjectRecovery::configKeyChanged(
    Mantid::Kernel::ConfigValChangeNotification_ptr notif) {
  if (notif->key() != (SAVING_ENABLED_CONFIG_KEY)) {
    return;
  }

  if (notif->curValue() == "True") {
    SAVING_ENABLED = true;
    startProjectSaving();
  } else {
    SAVING_ENABLED = false;
    stopProjectSaving();
  }
}

void ProjectRecovery::compileRecoveryScript(const Poco::Path &inputFolder,
                                            const Poco::Path &outputFile) {
  const std::string algName = "OrderWorkspaceHistory";
  auto *alg =
      Mantid::API::FrameworkManager::Instance().createAlgorithm(algName, 1);
  if (!alg) {
    throw std::runtime_error("Could not get pointer to alg: " + algName);
  }

  alg->initialize();
  alg->setRethrows(true);
  alg->setProperty("RecoveryCheckpointFolder", inputFolder.toString());
  alg->setProperty("OutputFilepath", outputFile.toString());

  alg->execute();

  g_log.notice("Saved your recovery script to:\n" + outputFile.toString());
}

/**
 * Deletes existing checkpoints, oldest first, in the recovery
 * folder. This is based on the configuration key which
 * indicates how many points to keep
 */
void ProjectRecovery::deleteExistingCheckpoints(
    size_t checkpointsToKeep) const {
  const auto folderPaths = getRecoveryFolderCheckpoints(getRecoveryFolder());

  size_t numberOfDirsPresent = folderPaths.size();
  if (numberOfDirsPresent <= checkpointsToKeep) {
    // Nothing to do
    return;
  }

  size_t checkpointsToRemove = numberOfDirsPresent - checkpointsToKeep;
  bool recurse = true;
  for (size_t i = 0; i < checkpointsToRemove; i++) {
    Poco::File(folderPaths[i]).remove(recurse);
  }
}

/// Starts a background thread which saves out the project periodically
void ProjectRecovery::startProjectSaving() {
  // Close the existing thread first
  stopProjectSaving();

  if (!SAVING_ENABLED) {
    return;
  }

  // Spin up a new thread
  {
    std::lock_guard<std::mutex> lock(m_notifierMutex);
    m_stopBackgroundThread = false;
  }

  m_backgroundSavingThread = createBackgroundThread();
}

/// Stops any existing background threads which are running
void ProjectRecovery::stopProjectSaving() {
  {
    std::lock_guard<std::mutex> lock(m_notifierMutex);
    m_stopBackgroundThread = true;
    m_threadNotifier.notify_all();
  }

  if (m_backgroundSavingThread.joinable()) {
    m_backgroundSavingThread.join();
  }
}

bool ProjectRecovery::openInEditor(const Poco::Path &inputFolder) {
  auto destFilename =
      Poco::Path(Mantid::Kernel::ConfigService::Instance().getAppDataDir());
  destFilename.append("ordered_recovery.py");
  compileRecoveryScript(inputFolder, destFilename);

  // Force application window to create the script window first
  const bool forceVisible = true;
  m_windowPtr->showScriptWindow(forceVisible);

  ScriptingWindow *scriptWindow = m_windowPtr->getScriptWindowHandle();
  if (!scriptWindow) {
    throw std::runtime_error("Could not get handle to scripting window");
  }

  scriptWindow->open(QString::fromStdString(destFilename.toString()));
  return true;
}

/// Top level thread wrapper which catches all exceptions to gracefully handle
/// them
void ProjectRecovery::projectSavingThreadWrapper() {
  try {
    projectSavingThread();
  } catch (std::exception const &e) {
    std::string preamble("Project recovery has stopped. Please report"
                         " this to the development team.\nException:\n");
    g_log.warning(preamble + e.what());
  } catch (...) {
    g_log.warning("Project recovery has stopped. Please report"
                  " this to the development team.");
  }
}

/**
 * Main thread body which is run to save out projects. A member mutex is
 * locked and monitored on a timeout to indicate if the thread should
 * exit early. After the timeout elapses, if the thread has not been
 * requested to exit, it will save the project out
 */
void ProjectRecovery::projectSavingThread() {
  while (!m_stopBackgroundThread) {
    std::unique_lock<std::mutex> lock(m_notifierMutex);
    // The condition variable releases the lock until the var changes
    if (m_threadNotifier.wait_for(lock, TIME_BETWEEN_SAVING, [this]() {
          return m_stopBackgroundThread.load();
        })) {
      // Exit thread
      g_log.debug("Project Recovery: Stopping background saving thread");
      return;
    }

    // "Timeout" - Save out again
    const auto &ads = Mantid::API::AnalysisDataService::Instance();
    if (ads.size() == 0) {
      g_log.debug("Nothing to save");
      continue;
    }

    g_log.debug("Project Recovery: Saving started");
    const auto basePath = getOutputPath();

    Poco::File(basePath).createDirectories();
    auto projectFile = Poco::Path(basePath).append(OUTPUT_PROJ_NAME);

    saveWsHistories(basePath);
    saveOpenWindows(projectFile.toString());

    // Purge any excessive folders
    deleteExistingCheckpoints(NO_OF_CHECKPOINTS);
    g_log.debug("Project Recovery: Saving finished");
  }
}

/**
 * Saves open all open windows using the main GUI thread
 *
 * @param projectDestFile :: The full path to write to
 * @throws If saving fails in the main GUI thread
 */
void ProjectRecovery::saveOpenWindows(const std::string &projectDestFile) {
  bool saveCompleted = false;
  if (!QMetaObject::invokeMethod(m_windowPtr, "saveProjectRecovery",
                                 Qt::BlockingQueuedConnection,
                                 Q_RETURN_ARG(bool, saveCompleted),
                                 Q_ARG(const std::string, projectDestFile))) {
    throw std::runtime_error(
        "Project Recovery: Failed to save project windows - Qt binding failed");
  }

  if (!saveCompleted) {
    throw std::runtime_error(
        "Project Recovery: Failed to write out project file");
  }
}

/**
 * Saves all workspace histories by using an external python script
 *
 * @param historyDestFolder:: The folder to write all histories to
 * @throw If saving fails in the script
 */
void ProjectRecovery::saveWsHistories(const Poco::Path &historyDestFolder) {
  const auto &ads = Mantid::API::AnalysisDataService::Instance();
  using Mantid::Kernel::DataServiceHidden;
  using Mantid::Kernel::DataServiceSort;

  const auto wsHandles =
      ads.getObjectNames(DataServiceSort::Unsorted, DataServiceHidden::Include);

  if (wsHandles.empty()) {
    return;
  }

  static auto startTime =
      Mantid::Kernel::UsageService::Instance().getStartTime().toISO8601String();

  const std::string algName = "GeneratePythonScript";
  auto *alg =
      Mantid::API::FrameworkManager::Instance().createAlgorithm(algName, 1);

  if (!alg) {
    throw std::runtime_error("Could not get pointer to alg: " + algName);
  }

  alg->setLogging(false);

  for (const auto &ws : wsHandles) {
    std::string filename = ws;
    filename.append(".py");

    Poco::Path destFilename = historyDestFolder;
    destFilename.append(filename);

    alg->initialize();
    alg->setProperty("AppendTimestamp", true);
    alg->setPropertyValue("InputWorkspace", ws);
    alg->setPropertyValue("Filename", destFilename.toString());
    alg->setPropertyValue("StartTimestamp", startTime);

    alg->execute();
  }
}

} // namespace MantidQt
