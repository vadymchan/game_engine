#ifndef GAME_ENGINE_ASSIMP_MATERIAL_LOADER_H
#define GAME_ENGINE_ASSIMP_MATERIAL_LOADER_H

#include "resources/i_material_loader.h"
#include "resources/image.h"

// TODO: move to cpp file
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <assimp/Importer.hpp>

namespace game_engine {

class AssimpMaterialLoader : public IMaterialLoader {
  public:
  std::vector<std::shared_ptr<Material>> loadMaterials(
      const std::filesystem::path& filePath) override;

  std::vector<std::shared_ptr<Material>> processMaterials(
      const aiScene* scene, const std::filesystem::path& filePath);

  private:
  // Function to load textures and add to the material's textures map
  void loadTextures(aiMaterial*                  mat,
                    aiTextureType                type,
                    std::shared_ptr<Material>& material,
                    const std::filesystem::path& basePath);

  std::string aiTextureTypeToString(aiTextureType type);

  // TODO: should be implemented and somewhere else
  std::shared_ptr<Texture> createTextureFromImage(
      const std::shared_ptr<Image>& image);
};

}  // namespace game_engine

#endif  // GAME_ENGINE_ASSIMP_MATERIAL_LOADER_H
