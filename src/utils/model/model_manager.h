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

  Model* getModel(const std::filesystem::path& filepath);

  private:
  std::unordered_map<std::filesystem::path, std::unique_ptr<Model>> modelCache_;
  std::mutex                                                        mutex_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_MODEL_MANAGER_H
