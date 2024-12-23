#ifndef GAME_ENGINE_MODEL_MANAGER_H
#define GAME_ENGINE_MODEL_MANAGER_H

#include "utils/model/model_loader_manager.h"
#include "utils/service/service_locator.h"

#include <filesystem>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace game_engine {

class ModelManager {
  public:
  ModelManager() = default;

  std::shared_ptr<Model> getModel(const std::filesystem::path& filepath) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto                        it = modelCache_.find(filepath);
    if (it != modelCache_.end()) {
      return it->second;
    }

    // Retrieve the ModelLoaderManager from ServiceLocator
    auto modelLoaderManager = ServiceLocator::s_get<ModelLoaderManager>();
    if (!modelLoaderManager) {
      // TODO: Logging if needed
      return nullptr;
    }

    auto model = modelLoaderManager->loadModel(filepath);
    if (model) {
      modelCache_[filepath] = model;
    }
    return model;
  }

  private:
  std::unordered_map<std::filesystem::path, std::shared_ptr<Model>> modelCache_;
  std::mutex                                                        mutex_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_MODEL_MANAGER_H
