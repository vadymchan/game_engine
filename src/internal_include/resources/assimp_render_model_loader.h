#ifndef GAME_ENGINE_ASSIMP_RENDER_MODEL_LOADER_H
#define GAME_ENGINE_ASSIMP_RENDER_MODEL_LOADER_H

#include "resources/i_render_model_loader.h"

namespace game_engine {

class AssimpRenderModelLoader : public IRenderModelLoader {
  public:
  std::shared_ptr<RenderModel> loadRenderModel(
      const std::filesystem::path&          filePath,
      std::optional<std::shared_ptr<Model>> outModel = std::nullopt) override;

  private:
  // Function to create GPU-side geometry mesh
  std::shared_ptr<RenderGeometryMesh> createRenderGeometryMesh(
      const std::shared_ptr<Mesh>& mesh);

  std::shared_ptr<VertexStreamData> createVertexStreamData(
      const std::shared_ptr<Mesh>& mesh);

  std::shared_ptr<IndexStreamData> createIndexStreamData(
      const std::shared_ptr<Mesh>& mesh);

  std::shared_ptr<VertexBuffer> createVertexBuffer(
      const std::shared_ptr<VertexStreamData>& vertexStreamData);

  std::shared_ptr<IndexBuffer> createIndexBuffer(
      const std::shared_ptr<IndexStreamData>& indexStreamData);
};

}  // namespace game_engine

#endif  // GAME_ENGINE_ASSIMP_RENDER_MODEL_LOADER_H
