#ifndef GAME_ENGINE_SHADER_MANAGER_H
#define GAME_ENGINE_SHADER_MANAGER_H

#include "gfx/rhi/rhi.h"
#include "utils/hot_reload/hot_reload_manager.h"
#include "utils/path_manager/path_manager.h"
#include "utils/service/service_locator.h"

#include <algorithm>
#include <filesystem>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

namespace game_engine {

class ShaderManager {
  public:
  ShaderManager(bool enableHotReload = true)
      : m_enableHotReload_(enableHotReload) {}

  ~ShaderManager() { release(); }

  std::shared_ptr<Shader> getShader(const std::filesystem::path& path) {
    std::lock_guard<std::mutex> lock(m_mutex_);

    auto it = m_loadedShaders_.find(path);
    if (it != m_loadedShaders_.end()) {
      return it->second;
    }

    return createShader_(path);
  }

  void reloadShader(const std::filesystem::path& path) {
    std::lock_guard<std::mutex> lock(m_mutex_);

    auto it = m_loadedShaders_.find(path);
    if (it == m_loadedShaders_.end()) {
      // TODO: log
      return;
    }

    std::shared_ptr<Shader> newShader = createShaderObject_(path);
    it->second                        = newShader;
  }

  void release() {
    std::lock_guard<std::mutex> lock(m_mutex_);
    m_loadedShaders_.clear();
    m_watchedDirs_.clear();
  }

  private:
  bool       m_enableHotReload_;
  std::mutex m_mutex_;

  std::unordered_map<std::filesystem::path, std::shared_ptr<Shader>>
      m_loadedShaders_;

  std::unordered_set<std::filesystem::path> m_watchedDirs_;

  private:
  std::shared_ptr<Shader> createShaderObject_(
      const std::filesystem::path& path) {
    EShaderAccessStageFlag stage = deduceStageFromPath_(path);

    ShaderInfo info;
    info.setName(Name(path.filename().string().c_str()));
    info.setShaderFilepath(Name(path.string().c_str()));
    info.setShaderType(stage);

    std::shared_ptr<Shader> newShader = g_rhi->createShader(info);
    return newShader;
  }

  std::shared_ptr<Shader> createShader_(const std::filesystem::path& path) {
    std::shared_ptr<Shader> newShader = createShaderObject_(path);
    m_loadedShaders_[path]            = newShader;

    if (m_enableHotReload_ && ServiceLocator::s_get<HotReloadManager>()) {
      watchDirectoryForChanges_(path);
    }
    return newShader;
  }

  EShaderAccessStageFlag deduceStageFromPath_(const std::filesystem::path& p) {
    // get first extension from full file extension (e.g. ".vs.hlsl") in
    // lowercase
    std::filesystem::path stem     = p.stem();
    std::string           stageExt = stem.extension().string();
    std::transform(
        stageExt.begin(), stageExt.end(), stageExt.begin(), ::tolower);

    if (stageExt == ".vs") {
      return EShaderAccessStageFlag::VERTEX;
    } else if (stageExt == ".ps") {
      return EShaderAccessStageFlag::FRAGMENT;
    } else if (stageExt == ".gs") {
      return EShaderAccessStageFlag::GEOMETRY;
    } else if (stageExt == ".cs") {
      return EShaderAccessStageFlag::COMPUTE;
    }

    return EShaderAccessStageFlag::ALL;
  }

  void watchDirectoryForChanges_(const std::filesystem::path& filePath) {
    auto dir = filePath.parent_path();
    if (dir.empty()) {
      dir = ".";
    }

    if (m_watchedDirs_.find(dir) != m_watchedDirs_.end()) {
      return;
    }
    m_watchedDirs_.insert(dir);

    auto hotReloadManager = ServiceLocator::s_get<HotReloadManager>();
    if (hotReloadManager) {
      hotReloadManager->watchFileModifications(
          dir.string(), [this](const wtr::event& e) {
            auto changedPath = std::filesystem::path(e.path_name);
            reloadShader(changedPath);
          });
    }
  }
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_MANAGER_H
