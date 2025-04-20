#ifndef GAME_ENGINE_FILE_WATCHER_MANAGER_H
#define GAME_ENGINE_FILE_WATCHER_MANAGER_H

#define NOMINMAX

#include <wtr/watcher.hpp>

namespace game_engine {

class FileWatcherManager {
  public:
  using Callback = std::function<void(const wtr::event&)>;

  void addWatcher(const std::filesystem::path& dirPath,
                  const Callback&              callback);

  void removeWatcher(const std::filesystem::path& dirPath);

  void removeAllWatchers();

  private:
  std::unordered_map<std::filesystem::path,
                     std::unique_ptr<wtr::watcher::watch>>
      m_watchers_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_FILE_WATCHER_MANAGER_H
