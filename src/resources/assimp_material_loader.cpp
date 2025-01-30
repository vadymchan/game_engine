#include "resources/assimp_material_loader.h"

#include "gfx/rhi/rhi.h"
#include "utils/image/image_manager.h"
#include "utils/service/service_locator.h"

namespace game_engine {
std::vector<std::shared_ptr<Material>> AssimpMaterialLoader::loadMaterials(
    const std::filesystem::path& filePath) {
  Assimp::Importer importer;
  // TODO: make flags configurable
  const aiScene*   scene = importer.ReadFile(
      filePath.string(), aiProcess_Triangulate | aiProcess_FlipUVs);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE
      || !scene->mRootNode) {
    // TODO: log error
  }

  return processMaterials(scene, filePath);
}

std::vector<std::shared_ptr<Material>> AssimpMaterialLoader::processMaterials(
    const aiScene* scene, const std::filesystem::path& filePath) {
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

  std::vector<std::shared_ptr<Material>> materials;
  for (unsigned int i = 0; i < scene->mNumMaterials; ++i) {
    aiMaterial* ai_material = scene->mMaterials[i];
    auto        material    = std::make_shared<Material>();

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
      if (AI_SUCCESS
          == ai_material->Get(param.pKey, param.type, param.index, value)) {
        material->scalarParameters[param.name] = value;
      }
    }

    // Vector parameters
    for (const auto& param : vectorParametersToFetch) {
      aiColor3D color;
      if (AI_SUCCESS
          == ai_material->Get(param.pKey, param.type, param.index, color)) {
        material->vectorParameters[param.name]
            = math::Vector4Df(color.r, color.g, color.b, 1.0f);
      }
    }

    // Load textures
    // Currently assumes that textures are located in the same directory as
    // the material file. Consider adding logic to handle different texture
    // directories if needed.
    for (int type = aiTextureType_NONE + 1; type <= AI_TEXTURE_TYPE_MAX;
         ++type) {
      loadTextures(
          ai_material, static_cast<aiTextureType>(type), material, filePath);
    }

    materials.emplace_back(material);
  }
  return materials;
}

// Function to load textures and add to the material's textures map

void AssimpMaterialLoader::loadTextures(aiMaterial*                  mat,
                                        aiTextureType                type,
                                        std::shared_ptr<Material>&   material,
                                        const std::filesystem::path& basePath) {
  for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
    aiString path;
    if (mat->GetTexture(type, i, &path) == AI_SUCCESS) {
      std::filesystem::path fullPath = basePath.parent_path() / path.C_Str();
      // TODO: this is placeholder
      // Load the image data (CPU-side)
      auto image = ServiceLocator::s_get<ImageManager>()->getImage(fullPath);
      // TODO: this is placeholders
      // Allocate the texture on GPU from image data
      auto texture = createTextureFromImage(image);
      material->textures[aiTextureTypeToString(type)] = texture;
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

std::shared_ptr<Texture> AssimpMaterialLoader::createTextureFromImage(
    const std::shared_ptr<Image>& image) {
  return g_rhi->createTextureFromData(image);
}

}  // namespace game_engine
