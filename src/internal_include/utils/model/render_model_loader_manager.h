#ifndef GAME_ENGINE_RENDER_MODEL_LOADER_MANAGER_H
#define GAME_ENGINE_RENDER_MODEL_LOADER_MANAGER_H

#include "resources/i_render_model_loader.h"
#include "utils/model/model_type.h"

#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <unordered_map>

namespace game_engine {

class RenderModelLoaderManager {
  public:
  RenderModelLoaderManager() = default;

  void registerLoader(EModelType                          modelType,
                      std::shared_ptr<IRenderModelLoader> loader) {
    std::lock_guard<std::mutex> lock(mutex_);
    loaderMap_[modelType] = std::move(loader);
  }

  std::shared_ptr<RenderModel> loadRenderModel(
      const std::filesystem::path&          filePath,
      std::optional<std::shared_ptr<Model>> outModel = std::nullopt) {
    std::string extension = filePath.extension().string();
    std::transform(
        extension.begin(), extension.end(), extension.begin(), ::tolower);
    EModelType modelType = getModelTypeFromExtension(extension);

    if (modelType == EModelType::UNKNOWN) {
      // TODO: add logging
      std::cerr << "Unknown model type for extension: " << extension
                << std::endl;
      return {};
    }

    std::lock_guard<std::mutex> lock(mutex_);
    auto                        it = loaderMap_.find(modelType);
    if (it != loaderMap_.end()) {
      return it->second->loadRenderModel(filePath, outModel);
    }
    // TODO: Logging or error handling if no loader is found
    return nullptr;
  }

  private:
  std::unordered_map<EModelType, std::shared_ptr<IRenderModelLoader>>
             loaderMap_;
  std::mutex mutex_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_MODEL_LOADER_MANAGER_H
