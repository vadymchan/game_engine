#ifndef GAME_ENGINE_ASSIMP_RENDER_MODEL_LOADER_H
#define GAME_ENGINE_ASSIMP_RENDER_MODEL_LOADER_H

#include "resources/i_render_model_loader.h"

#include "gfx/rhi/interface/device.h"

namespace game_engine {

class AssimpRenderModelLoader : public IRenderModelLoader {
  public:
  AssimpRenderModelLoader(gfx::rhi::Device* device);
  ~AssimpRenderModelLoader() = default;

  std::unique_ptr<RenderModel> loadRenderModel(const std::filesystem::path& filePath,
                                               std::optional<Model*>        outModel = std::nullopt) override;

  private:
  // Function to create GPU-side geometry mesh
  std::unique_ptr<RenderGeometryMesh> createRenderGeometryMesh(Mesh* mesh);

  gfx::rhi::Buffer* createVertexBuffer(const Mesh* mesh);
  gfx::rhi::Buffer* createIndexBuffer(const Mesh* mesh);
    

  // TODO: Probably move it to parent class
  gfx::rhi::Device* m_device;

};

}  // namespace game_engine

#endif  // GAME_ENGINE_ASSIMP_RENDER_MODEL_LOADER_H