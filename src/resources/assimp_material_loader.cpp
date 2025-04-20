#include "resources/assimp_material_loader.h"

#include "utils/image/image_manager.h"
#include "utils/service/service_locator.h"
#include "utils/texture/texture_manager.h"

#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>

namespace game_engine {

AssimpMaterialLoader::AssimpMaterialLoader(gfx::rhi::Device* device)
    : m_device(device) {
  if (!m_device) {
    GlobalLogger::Log(LogLevel::Error, "Device is null");
  }
}

std::vector<std::unique_ptr<Material>> AssimpMaterialLoader::loadMaterials(const std::filesystem::path& filePath) {
  GlobalLogger::Log(LogLevel::Info, "Loading materials from " + filePath.string());

  Assimp::Importer importer;
  // TODO: make flags configurable
  const aiScene*   scene = importer.ReadFile(filePath.string(), aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    GlobalLogger::Log(LogLevel::Error,
                      "Assimp scene is invalid or incomplete for file \"" + filePath.string()
                          + "\". Error: " + importer.GetErrorString());
    return {};
  }

  auto materials = processMaterials(scene, filePath);

  GlobalLogger::Log(
      LogLevel::Info,
      "Successfully loaded " + std::to_string(materials.size()) + " material(s) from " + filePath.string());

  return materials;
}

std::vector<std::unique_ptr<Material>> AssimpMaterialLoader::processMaterials(const aiScene*               scene,
                                                                              const std::filesystem::path& filePath) {
  struct Parameter {
    const char*  name;   // Friendly name for mapping
    const char*  pKey;   // Property key
    unsigned int type;   // Property type
    unsigned int index;  // Property index
  };

  std::vector<Parameter> scalarParametersToFetch = {
    {"roughness", AI_MATKEY_ROUGHNESS_FACTOR},
    { "metallic",  AI_MATKEY_METALLIC_FACTOR},
    {  "opacity",          AI_MATKEY_OPACITY}
  };

  std::vector<Parameter> vectorParametersToFetch = {
    {"base_color", AI_MATKEY_BASE_COLOR}
  };

  std::vector<std::unique_ptr<Material>> materials;
  for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
    aiMaterial* ai_material = scene->mMaterials[i];
    auto        material    = std::make_unique<Material>();

    // Set the file path for the material
    material->filePath = filePath;

    // Material name
    aiString name;
    if (ai_material->Get(AI_MATKEY_NAME, name) == AI_SUCCESS) {
      material->materialName = name.C_Str();
    } else {
      material->materialName = "Material_" + std::to_string(i);
    }

    // Scalar parameters
    for (const auto& param : scalarParametersToFetch) {
      float value;
      if (AI_SUCCESS == ai_material->Get(param.pKey, param.type, param.index, value)) {
        material->scalarParameters[param.name] = value;
      }
    }

    // Vector parameters
    for (const auto& param : vectorParametersToFetch) {
      aiColor3D color;
      if (AI_SUCCESS == ai_material->Get(param.pKey, param.type, param.index, color)) {
        material->vectorParameters[param.name] = math::Vector4Df(color.r, color.g, color.b, 1.0f);
      }
    }

    // Load textures
    // Currently assumes that textures are located in the same directory as
    // the material file. Consider adding logic to handle different texture
    // directories if needed.
    for (int type = aiTextureType_NONE + 1; type <= AI_TEXTURE_TYPE_MAX; ++type) {
      loadTextures(ai_material, static_cast<aiTextureType>(type), material.get(), filePath);
    }

    materials.push_back(std::move(material));
  }
  return materials;
}

// Function to load textures and add to the material's textures map
void AssimpMaterialLoader::loadTextures(aiMaterial*                  mat,
                                        aiTextureType                type,
                                        Material*                    material,
                                        const std::filesystem::path& basePath) {
  for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
    aiString path;
    if (mat->GetTexture(type, i, &path) == AI_SUCCESS) {
      std::filesystem::path fullPath = basePath.parent_path() / path.C_Str();

      // Get the image from ImageManager (raw pointer, not owned by us)
      auto imageManager = ServiceLocator::s_get<ImageManager>();
      if (!imageManager) {
        GlobalLogger::Log(LogLevel::Error, "ImageManager not available");
        continue;
      }

      auto image = imageManager->getImage(fullPath);

      if (image) {
        // Get TextureManager from ServiceLocator
        auto textureManager = ServiceLocator::s_get<TextureManager>();
        if (!textureManager) {
          GlobalLogger::Log(LogLevel::Error, "TextureManager not found in ServiceLocator");
          continue;
        }

        // Create texture name based on file path
        std::string textureName = fullPath.filename().string();

        // Let texture manager create and own the texture
        auto texture = textureManager->createTexture(image, textureName);

        if (texture) {
          // Material now stores raw pointers to textures
          material->textures[aiTextureTypeToString(type)] = texture;
        }
      }
    }
  }
}

std::string AssimpMaterialLoader::aiTextureTypeToString(aiTextureType type) {
  switch (type) {
    case aiTextureType_BASE_COLOR:
    case aiTextureType_DIFFUSE:
      return "albedo";
    case aiTextureType_METALNESS:
      return "metalness";
    case aiTextureType_DIFFUSE_ROUGHNESS:
    case aiTextureType_MAYA_SPECULAR_ROUGHNESS:  // Maya PBR
      return "roughness";
    case aiTextureType_NORMALS:
    case aiTextureType_NORMAL_CAMERA:            // Maya PBR
      return "normal_map";
    default:
      return "unknown";
  }
}
}  // namespace game_engine