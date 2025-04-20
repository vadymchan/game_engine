#ifndef GAME_ENGINE_MODEL_LOADER_MANAGER_H
#define GAME_ENGINE_MODEL_LOADER_MANAGER_H

#include "resources/i_model_loader.h"
#include "utils/logger/global_logger.h"
#include "utils/model/model_type.h"

#include <filesystem>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace game_engine {

class ModelLoaderManager {
  public:
  ModelLoaderManager() = default;

  void registerLoader(ModelType modelType, std::unique_ptr<IModelLoader> loader);

  std::unique_ptr<Model> loadModel(const std::filesystem::path& filePath);

  private:
  std::unordered_map<ModelType, std::unique_ptr<IModelLoader>> loaderMap_;
  std::mutex                                                    mutex_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_MODEL_LOADER_MANAGER_H
