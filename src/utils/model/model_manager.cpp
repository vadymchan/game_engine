#include "utils/model/model_manager.h"

namespace game_engine {
Model* ModelManager::getModel(const std::filesystem::path& filepath) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto                        it = modelCache_.find(filepath);
  if (it != modelCache_.end()) {
    return it->second.get();
  }

  // Retrieve the ModelLoaderManager from ServiceLocator
  auto modelLoaderManager = ServiceLocator::s_get<ModelLoaderManager>();
  if (!modelLoaderManager) {
    GlobalLogger::Log(LogLevel::Error, "ModelLoaderManager not available in ServiceLocator.");
    return nullptr;
  }

  auto model = modelLoaderManager->loadModel(filepath);
  if (model) {
    // Save pointer to return before we transfer ownership to the cache
    Model* modelPtr       = model.get();
    modelCache_[filepath] = std::move(model);
    return modelPtr;
  }

  GlobalLogger::Log(LogLevel::Warning, "Failed to load model: " + filepath.string());
  return nullptr;
}
}  // namespace game_engine