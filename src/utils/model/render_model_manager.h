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

  RenderModel* getRenderModel(const std::filesystem::path& filepath, std::optional<Model*> outModel = std::nullopt);

  private:
  std::unordered_map<std::filesystem::path, std::unique_ptr<RenderModel>> renderModelCache_;
  std::mutex                                                              mutex_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_MODEL_MANAGER_H
