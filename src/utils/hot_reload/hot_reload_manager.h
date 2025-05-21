#ifndef GAME_ENGINE_HOT_RELOAD_MANAGER_H
#define GAME_ENGINE_HOT_RELOAD_MANAGER_H

#include "utils/file_watcher/file_watcher_manager.h"

namespace game_engine {

class HotReloadManager {
  public:
  // TODO: consider using FileWatcherManager Callback
  using Callback = std::function<void(const wtr::event&)>;

  void watchFileModifications(const std::filesystem::path& dirPath,
                              const Callback&              onChange);
};

}  // namespace game_engine

#endif  // GAME_ENGINE_HOT_RELOAD_MANAGER_H
