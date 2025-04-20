#ifndef GAME_ENGINE_ASSIMP_MATERIAL_LOADER_H
#define GAME_ENGINE_ASSIMP_MATERIAL_LOADER_H

#include "resources/i_material_loader.h"
#include "resources/image.h"
#include "gfx/rhi/interface/device.h"

struct aiMaterial;
struct aiScene;
enum aiTextureType;

namespace game_engine {

class AssimpMaterialLoader : public IMaterialLoader {
  public:
  AssimpMaterialLoader(gfx::rhi::Device* device);
  ~AssimpMaterialLoader() = default;

  std::vector<std::unique_ptr<Material>> loadMaterials(const std::filesystem::path& filePath) override;

  std::vector<std::unique_ptr<Material>> processMaterials(const aiScene* scene, const std::filesystem::path& filePath);

  private:
  // Function to load textures and add to the material's textures map
  void loadTextures(aiMaterial* mat, aiTextureType type, Material* material, const std::filesystem::path& basePath);

  std::string aiTextureTypeToString(aiTextureType type);

  // Probably move it to parent class
  gfx::rhi::Device* m_device;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_ASSIMP_MATERIAL_LOADER_H