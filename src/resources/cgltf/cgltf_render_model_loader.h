#ifndef GAME_ENGINE_GLTF_RENDER_MODEL_LOADER_H
#define GAME_ENGINE_GLTF_RENDER_MODEL_LOADER_H

#ifdef GAME_ENGINE_USE_CGLTF

#include "gfx/rhi/interface/device.h"
#include "resources/i_render_model_loader.h"

namespace game_engine {

class CgltfRenderModelLoader : public IRenderModelLoader {
  public:
  CgltfRenderModelLoader()  = default;
  ~CgltfRenderModelLoader() = default;

  std::unique_ptr<RenderModel> loadRenderModel(const std::filesystem::path& filePath,
                                               std::optional<Model*>        outModel = std::nullopt) override;

  private:
  // GPU-side geometry mesh
  std::unique_ptr<RenderGeometryMesh> createRenderGeometryMesh(Mesh* mesh);

  gfx::rhi::Buffer* createVertexBuffer(const Mesh* mesh);
  gfx::rhi::Buffer* createIndexBuffer(const Mesh* mesh);
};

}  // namespace game_engine

#endif  // GAME_ENGINE_USE_CGLTF

#endif  // GAME_ENGINE_GLTF_RENDER_MODEL_LOADER_H