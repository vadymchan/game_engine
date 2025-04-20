#ifndef GAME_ENGINE_I_MODEL_LOADER_H
#define GAME_ENGINE_I_MODEL_LOADER_H

#include "ecs/components/model.h"

#include <filesystem>
#include <memory>

namespace game_engine {

class IModelLoader {
  public:
  virtual ~IModelLoader()                                                         = default;
  virtual std::unique_ptr<Model> loadModel(const std::filesystem::path& filepath) = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_I_MODEL_LOADER_H
