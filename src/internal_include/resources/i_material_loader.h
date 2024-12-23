#ifndef GAME_ENGINE_I_MATERIAL_LOADER_H
#define GAME_ENGINE_I_MATERIAL_LOADER_H

#include "ecs/components/material.h"

namespace game_engine {

class IMaterialLoader {
  public:
  virtual ~IMaterialLoader() = default;
  virtual std::vector<std::shared_ptr<Material>> loadMaterials(
      const std::filesystem::path& filePath)
      = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_I_MATERIAL_LOADER_H
