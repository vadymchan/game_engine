#ifndef ARISE_ASSIMP_RENDER_MODEL_LOADER_H
#define ARISE_ASSIMP_RENDER_MODEL_LOADER_H

// TODO: assimp will be deprecated soon from the project
#ifdef ARISE_USE_ASSIMP

#include "gfx/rhi/interface/device.h"
#include "resources/i_render_model_loader.h"

namespace arise {

class AssimpRenderModelLoader : public IRenderModelLoader {
  public:
  AssimpRenderModelLoader()  = default;
  ~AssimpRenderModelLoader() = default;

  std::unique_ptr<RenderModel> loadRenderModel(const std::filesystem::path& filePath,
                                               Model**                      outModel = nullptr) override;

  private:
  // GPU-side geometry mesh
  std::unique_ptr<RenderGeometryMesh> createRenderGeometryMesh(Mesh* mesh);

  gfx::rhi::Buffer* createVertexBuffer(const Mesh* mesh);
  gfx::rhi::Buffer* createIndexBuffer(const Mesh* mesh);
};

}  // namespace arise

#endif  // ARISE_USE_ASSIMP

#endif  // ARISE_ASSIMP_RENDER_MODEL_LOADER_H