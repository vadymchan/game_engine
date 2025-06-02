#ifndef ARISE_ASSIMP_MATERIAL_LOADER_H
#define ARISE_ASSIMP_MATERIAL_LOADER_H

#ifdef ARISE_USE_ASSIMP


#include "gfx/rhi/interface/device.h"
#include "resources/i_material_loader.h"
#include "resources/image.h"

struct aiMaterial;
struct aiScene;
enum aiTextureType;

namespace arise {

class AssimpMaterialLoader : public IMaterialLoader {
  public:
  AssimpMaterialLoader()  = default;
  ~AssimpMaterialLoader() = default;

  std::vector<std::unique_ptr<Material>> loadMaterials(const std::filesystem::path& filePath) override;

  private:
  std::vector<std::unique_ptr<Material>> processMaterials(const aiScene* scene, const std::filesystem::path& filePath);

  void loadTextures(aiMaterial* mat, aiTextureType type, Material* material, const std::filesystem::path& basePath);

  std::string aiTextureTypeToString(aiTextureType type);
};

}  // namespace arise

#endif  // ARISE_USE_ASSIMP

#endif  // ARISE_ASSIMP_MATERIAL_LOADER_H
