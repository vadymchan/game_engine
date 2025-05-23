#include "utils/model/render_model_manager.h"

namespace game_engine {
RenderModel* RenderModelManager::getRenderModel(const std::filesystem::path& filepath, std::optional<Model*> outModel) {
  std::lock_guard<std::mutex> lock(mutex_);

  // Check if render model is already loaded
  auto it = renderModelCache_.find(filepath);
  if (it != renderModelCache_.end()) {
    return it->second.get();
  }

  // Get loader manager from service locator
  auto renderModelLoaderManager = ServiceLocator::s_get<RenderModelLoaderManager>();
  if (!renderModelLoaderManager) {
    GlobalLogger::Log(LogLevel::Error, "RenderModelLoaderManager not available in ServiceLocator.");
    return nullptr;
  }

  // Load render model
  auto renderModel = renderModelLoaderManager->loadRenderModel(filepath, outModel);
  if (renderModel) {
    // Save pointer to return before we transfer ownership to the cache
    RenderModel* modelPtr       = renderModel.get();
    renderModelCache_[filepath] = std::move(renderModel);
    return modelPtr;
  }

  GlobalLogger::Log(LogLevel::Warning, "Failed to load render model: " + filepath.string());
  return nullptr;
}
}  // namespace game_engine