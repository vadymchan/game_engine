#ifndef GAME_ENGINE_MESH_H
#define GAME_ENGINE_MESH_H

#include "ecs/components/vertex.h"

#include <string>
#include <vector>

namespace game_engine {

// This is the geometry data on CPU side (imported from assimp)
struct Mesh {
  std::string           meshName;
  std::vector<Vertex>   vertices;
  std::vector<uint32_t> indices;
  math::Matrix4f<>      transformMatrix = math::Matrix4f<>::Identity();
};

}  // namespace game_engine

#endif  // GAME_ENGINE_MESH_H
