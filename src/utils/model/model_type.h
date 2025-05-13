#ifndef GAME_ENGINE_MODEL_TYPES_H
#define GAME_ENGINE_MODEL_TYPES_H

#include <algorithm>
#include <string>
#include <unordered_map>

namespace game_engine {

enum class ModelType {
  OBJ,
  FBX,
  GLTF,
  GLB, 
  UNKNOWN,
};

ModelType getModelTypeFromExtension(const std::string& extension);

}  // namespace game_engine

#endif  // GAME_ENGINE_MODEL_TYPES_H
