#ifndef GAME_ENGINE_MATERIAL_H
#define GAME_ENGINE_MATERIAL_H

#include "gfx/rhi/texture.h"

#include <math_library/vector.h>

#include <filesystem>
#include <string>
#include <unordered_map>

namespace game_engine {

struct Material {
  std::string materialName;

  std::filesystem::path filePath;

  // TODO: consider adding tags to materials (so we can set specific
  // characteristics of material (e.g. "PBR", "transparent", etc.)
  // std::set<std::string> tags;

  // Scalar parameters (e.g., roughness, metallic)
  std::unordered_map<std::string, float> scalarParameters;

  // Vector parameters (e.g., base color)
  std::unordered_map<std::string, math::Vector4Df> vectorParameters;

  // Textures associated with the material
  // Keyed by texture name (e.g., "albedo", "normal")
  // TODO: consider create enum class and use it as key
  std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_MATERIAL_H
