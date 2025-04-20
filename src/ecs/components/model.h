#ifndef GAME_ENGINE_MODEL_H
#define GAME_ENGINE_MODEL_H

#include "ecs/components/mesh.h"

#include <filesystem>
#include <memory>

namespace game_engine {

// TODO: consider adding Node functionality (if that is needed)

// This is the geometry data on CPU side (imported from assimp)
struct Model {
  // TODO: no modelNames (consider adding it, maybe filename will be correct)
  std::filesystem::path              filePath;
  std::vector<Mesh*> meshes;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_MODEL_H
