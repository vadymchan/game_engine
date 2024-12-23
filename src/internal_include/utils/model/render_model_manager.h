#ifndef GAME_ENGINE_RENDER_MODEL_MANAGER_H
#define GAME_ENGINE_RENDER_MODEL_MANAGER_H

#include "utils/model/render_model_loader_manager.h"
#include "utils/service/service_locator.h"

#include <filesystem>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace game_engine {

class RenderModelManager {
  public:
  RenderModelManager() = default;

  std::shared_ptr<RenderModel> getRenderModel(
      const std::filesystem::path&          filepath,
      std::optional<std::shared_ptr<Model>> outModel = std::nullopt) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto                        it = renderModelCache_.find(filepath);
    if (it != renderModelCache_.end()) {
      return it->second;
    }

    auto renderModelLoaderManager
        = ServiceLocator::s_get<RenderModelLoaderManager>();
    if (!renderModelLoaderManager) {
      // TODO: Logging if needed
      return nullptr;
    }

    auto renderModel
        = renderModelLoaderManager->loadRenderModel(filepath, outModel);
    if (renderModel) {
      renderModelCache_[filepath] = renderModel;
    }
    return renderModel;
  }

  private:
  std::unordered_map<std::filesystem::path, std::shared_ptr<RenderModel>>
             renderModelCache_;
  std::mutex mutex_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_MODEL_MANAGER_H
