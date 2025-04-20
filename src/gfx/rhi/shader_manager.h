#ifndef GAME_ENGINE_SHADER_MANAGER_H
#define GAME_ENGINE_SHADER_MANAGER_H

#include "gfx/rhi/backends/dx12/dxc_util.h"
#include "gfx/rhi/common/rhi_enums.h"
#include "gfx/rhi/interface/device.h"
#include "gfx/rhi/interface/shader.h"
#include "utils/hot_reload/hot_reload_manager.h"
#include "utils/logger/global_logger.h"
#include "utils/service/service_locator.h"

#include <algorithm>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace game_engine {
namespace gfx {
namespace rhi {

/**
 * ShaderManager class for loading, compiling, and managing shaders
 *
 * Features:
 * - Automatic shader compilation from HLSL source
 * - Caching to prevent redundant loading
 * - Hot reloading of changed shaders during development
 * - Automatic shader stage detection from file extension
 */
class ShaderManager {
  public:
  ShaderManager(Device* device, bool enableHotReload = true)
      : m_device_(device)
      , m_enableHotReload_(enableHotReload) {}

  ~ShaderManager() { release(); }

  /**
   * Get a shader from path
   *
   * Loads and compiles the shader if not already loaded, otherwise
   * returns the cached shader.
   */
  Shader* getShader(const std::filesystem::path& path, const std::string& entryPoint = "main") {
    std::lock_guard<std::mutex> lock(m_mutex_);

    auto it = m_loadedShaders_.find(path);
    if (it != m_loadedShaders_.end()) {
      return it->second.get();
    }

    return createShader(path, entryPoint);
  }

  void reloadShader(const std::filesystem::path& path) {
    std::lock_guard<std::mutex> lock(m_mutex_);

    auto it = m_loadedShaders_.find(path);
    if (it == m_loadedShaders_.end()) {
      GlobalLogger::Log(LogLevel::Warning, "Cannot reload shader, not loaded: " + path.string());
      return;
    }

    // Use entry point from the existing shader
    std::string entryPoint = it->second->getEntryPoint();

    auto newShader = createShaderObject(path, entryPoint);
    if (newShader) {
      GlobalLogger::Log(LogLevel::Info, "Reloaded shader: " + path.string());
      it->second = std::move(newShader);
    } else {
      GlobalLogger::Log(LogLevel::Error, "Failed to reload shader: " + path.string());
    }
  }

  void release() {
    std::lock_guard<std::mutex> lock(m_mutex_);

    m_loadedShaders_.clear();
    m_watchedDirs_.clear();
  }

  private:
  Shader* createShader(const std::filesystem::path& path, const std::string& entryPoint) {
    auto shader = createShaderObject(path, entryPoint);
    if (!shader) {
      GlobalLogger::Log(LogLevel::Error, "Failed to create shader: " + path.string());
      return nullptr;
    }

    Shader* rawPtr = shader.get();

    m_loadedShaders_[path] = std::move(shader);

    if (m_enableHotReload_ && ServiceLocator::s_get<HotReloadManager>()) {
      watchDirectoryForChanges(path);
    }

    return rawPtr;
  }

    auto createShaderObject(const std::filesystem::path& path, const std::string& entryPoint) -> std::unique_ptr<Shader> {
    ShaderStageFlag stage = deduceStageFromPath(path);

    auto backend = (m_device_->getApiType() == RenderingApi::Vulkan) ? ShaderBackend::SPIRV : ShaderBackend::DXIL;

    // string -> wstring
    std::wstring wEntryPoint(entryPoint.begin(), entryPoint.end());

    auto compiledShader = DxcUtil::s_get().compileHlslFile(path, stage, wEntryPoint, backend);

    if (!compiledShader) {
      GlobalLogger::Log(LogLevel::Error, "Shader compilation failed: " + path.string());
      return nullptr;
    }

    ShaderDesc desc;
    desc.stage      = stage;
    desc.entryPoint = entryPoint;

    // Copy shader bytecode
    auto   data = static_cast<const uint8_t*>(compiledShader->GetBufferPointer());
    size_t size = compiledShader->GetBufferSize();
    desc.code.assign(data, data + size);

    return m_device_->createShader(desc);
  }

  ShaderStageFlag deduceStageFromPath(const std::filesystem::path& path) {
    // Get first extension from filename (e.g., ".vs.hlsl" -> ".vs")
    std::filesystem::path stem     = path.stem();
    std::string           stageExt = stem.extension().string();

    // Convert to lowercase
    std::transform(stageExt.begin(), stageExt.end(), stageExt.begin(), ::tolower);

    if (stageExt == ".vs") {
      return ShaderStageFlag::Vertex;
    } else if (stageExt == ".ps" || stageExt == ".fs") {
      return ShaderStageFlag::Fragment;
    } else if (stageExt == ".gs") {
      return ShaderStageFlag::Geometry;
    } else if (stageExt == ".cs") {
      return ShaderStageFlag::Compute;
    } else if (stageExt == ".hs") {
      return ShaderStageFlag::TessellationControl;
    } else if (stageExt == ".ds") {
      return ShaderStageFlag::TessellationEvaluation;
    } else if (stageExt == ".rgen") {
      return ShaderStageFlag::RaytracingRaygen;
    } else if (stageExt == ".rmiss") {
      return ShaderStageFlag::RaytracingMiss;
    } else if (stageExt == ".rchit") {
      return ShaderStageFlag::RaytracingClosesthit;
    } else if (stageExt == ".rahit") {
      return ShaderStageFlag::RaytracingAnyhit;
    }

    // Default to All if unable to determine
    GlobalLogger::Log(LogLevel::Warning,
                      "Could not determine shader stage from extension: " + stageExt + ". Using All as default.");
    return ShaderStageFlag::All;
  }

  /**
   * Set up directory watching for hot reload
   */
  void watchDirectoryForChanges(const std::filesystem::path& filePath) {
    auto dir = filePath.parent_path();
    if (dir.empty()) {
      dir = ".";
    }

    // Check if already being watched
    if (m_watchedDirs_.find(dir) != m_watchedDirs_.end()) {
      return;
    }

    // Add to watched directories
    m_watchedDirs_.insert(dir);

    auto hotReloadManager = ServiceLocator::s_get<HotReloadManager>();
    if (hotReloadManager) {
      hotReloadManager->watchFileModifications(dir.string(), [this](const wtr::event& e) {
        auto changedPath = std::filesystem::path(e.path_name);
        reloadShader(changedPath);
      });
    }
  }

  private:
  Device*                                                            m_device_;
  bool                                                               m_enableHotReload_;
  std::mutex                                                         m_mutex_;
  std::unordered_map<std::filesystem::path, std::unique_ptr<Shader>> m_loadedShaders_;
  std::unordered_set<std::filesystem::path>                          m_watchedDirs_;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_SHADER_MANAGER_H