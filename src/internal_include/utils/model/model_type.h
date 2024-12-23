#ifndef GAME_ENGINE_MODEL_TYPES_H
#define GAME_ENGINE_MODEL_TYPES_H

#include <algorithm>
#include <string>
#include <unordered_map>

namespace game_engine {

enum class EModelType {
  OBJ,
  FBX,
  UNKNOWN,
};

inline EModelType getModelTypeFromExtension(const std::string& extension) {
  static const std::unordered_map<std::string, EModelType> extensionToType = {
    {".obj", EModelType::OBJ},
    {".fbx", EModelType::FBX},
  };

  std::string ext = extension;
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  auto it = extensionToType.find(ext);
  if (it != extensionToType.end()) {
    return it->second;
  }

  return EModelType::UNKNOWN;
}

}  // namespace game_engine

#endif  // GAME_ENGINE_MODEL_TYPES_H
