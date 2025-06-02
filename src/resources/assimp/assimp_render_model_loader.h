#ifndef GAME_ENGINE_ASSIMP_RENDER_MODEL_LOADER_H
#define GAME_ENGINE_ASSIMP_RENDER_MODEL_LOADER_H

#ifdef GAME_ENGINE_USE_ASSIMP


#include "resources/i_render_model_loader.h"

#include "gfx/rhi/interface/device.h"

namespace game_engine {

class AssimpRenderModelLoader : public IRenderModelLoader {
  public:
  AssimpRenderModelLoader() = default;
  ~AssimpRenderModelLoader() = default;

  std::unique_ptr<RenderModel> loadRenderModel(const std::filesystem::path& filePath,
                                               std::optional<Model*>        outModel = std::nullopt) override;

  private:
  // GPU-side geometry mesh
  std::unique_ptr<RenderGeometryMesh> createRenderGeometryMesh(Mesh* mesh);

  gfx::rhi::Buffer* createVertexBuffer(const Mesh* mesh);
  gfx::rhi::Buffer* createIndexBuffer(const Mesh* mesh);
};

}  // namespace game_engine

#endif  // GAME_ENGINE_USE_ASSIMP

#endif  // GAME_ENGINE_ASSIMP_RENDER_MODEL_LOADER_H