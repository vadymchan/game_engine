#include "utils/model/model_type.h"

namespace arise {
ModelType getModelTypeFromExtension(const std::string& extension) {
  static const std::unordered_map<std::string, ModelType> extensionToType = {
    { ".obj",  ModelType::OBJ},
    { ".fbx",  ModelType::FBX},
    {".gltf", ModelType::GLTF},
    { ".glb",  ModelType::GLB},
  };

  std::string ext = extension;
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  auto it = extensionToType.find(ext);
  if (it != extensionToType.end()) {
    return it->second;
  }

  return ModelType::UNKNOWN;
}
}  // namespace arise