#ifndef GAME_ENGINE_RENDER_MODEL_LOADER_MANAGER_H
#define GAME_ENGINE_RENDER_MODEL_LOADER_MANAGER_H

#include "resources/i_render_model_loader.h"
#include "utils/model/model_type.h"
#include "utils/logger/global_logger.h"

#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>

namespace game_engine {

class RenderModelLoaderManager {
  public:
  RenderModelLoaderManager() = default;

  void registerLoader(ModelType modelType, std::shared_ptr<IRenderModelLoader> loader);

  std::unique_ptr<RenderModel> loadRenderModel(const std::filesystem::path& filePath,
                                               std::optional<Model*>        outModel = std::nullopt);

  private:
  std::unordered_map<ModelType, std::shared_ptr<IRenderModelLoader>> loaderMap_;
  std::mutex                                                          mutex_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_MODEL_LOADER_MANAGER_H
