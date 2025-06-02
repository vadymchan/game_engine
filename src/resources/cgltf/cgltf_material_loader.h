#ifndef GAME_ENGINE_GLTF_MATERIAL_LOADER_H
#define GAME_ENGINE_GLTF_MATERIAL_LOADER_H

#ifdef GAME_ENGINE_USE_CGLTF

#include "gfx/rhi/interface/device.h"
#include "resources/i_material_loader.h"
#include "resources/image.h"

// Forward declarations
struct cgltf_data;
struct cgltf_material;
struct cgltf_image;

namespace game_engine {

class CgltfMaterialLoader : public IMaterialLoader {
  public:
  CgltfMaterialLoader()  = default;
  ~CgltfMaterialLoader() = default;

  std::vector<std::unique_ptr<Material>> loadMaterials(const std::filesystem::path& filePath) override;

  private:
  std::vector<std::unique_ptr<Material>> processMaterials(const cgltf_data*            data,
                                                          const std::filesystem::path& filePath);

  std::unique_ptr<Material> processMaterial(const cgltf_material*        material,
                                            const std::filesystem::path& filePath,
                                            size_t                       materialIndex);

  void loadTextures(const cgltf_material* material, Material* outMaterial, const std::filesystem::path& basePath);

  gfx::rhi::Texture* loadTexture(const cgltf_image*           image,
                                 const std::filesystem::path& basePath,
                                 const std::string&           textureName);
};

}  // namespace game_engine

#endif  // GAME_ENGINE_USE_CGLTF

#endif  // GAME_ENGINE_GLTF_MATERIAL_LOADER_H