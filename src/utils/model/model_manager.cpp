#include "utils/model/model_manager.h"

namespace arise {
Model* ModelManager::getModel(const std::filesystem::path& filepath) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto                        it = modelCache_.find(filepath);
  if (it != modelCache_.end()) {
    return it->second.get();
  }

  auto modelLoaderManager = ServiceLocator::s_get<ModelLoaderManager>();
  if (!modelLoaderManager) {
    GlobalLogger::Log(LogLevel::Error, "ModelLoaderManager not available in ServiceLocator.");
    return nullptr;
  }

  auto model = modelLoaderManager->loadModel(filepath);
  if (model) {
    Model* modelPtr       = model.get();
    modelCache_[filepath] = std::move(model);
    return modelPtr;
  }

  GlobalLogger::Log(LogLevel::Warning, "Failed to load model: " + filepath.string());
  return nullptr;
}
}  // namespace arise