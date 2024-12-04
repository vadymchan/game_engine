#include "utils/file_watcher/file_watcher_manager.h"

#include <iostream>

namespace game_engine {

void FileWatcherManager::addWatcher(const std::filesystem::path& dirPath,
                                    const Callback&              callback) {
  std::filesystem::path modifiedDirPath = "./" / dirPath;

  if (m_watchers_.find(modifiedDirPath) != m_watchers_.end()) {
    // TODO: use logger
    std::cerr << "Watcher for this path already exists!" << std::endl;
    return;
  }

  m_watchers_[modifiedDirPath]
      = std::make_unique<wtr::watcher::watch>(modifiedDirPath, callback);
}

void FileWatcherManager::removeWatcher(const std::filesystem::path& dirPath) {
  if (m_watchers_.find(dirPath) != m_watchers_.end()) {
    m_watchers_.erase(dirPath);
  } else {
    // TODO: use logger
    std::cerr << "No watcher for this path." << std::endl;
  }
}

void FileWatcherManager::removeAllWatchers() {
  m_watchers_.clear();
}

}  // namespace game_engine
