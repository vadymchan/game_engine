#ifndef GAME_ENGINE_RENDER_GEOMETRY_MESH_H
#define GAME_ENGINE_RENDER_GEOMETRY_MESH_H

#include <gfx/rhi/buffer.h>

#include <cstdint>

namespace game_engine {

// GPU-Side Mesh Geometry data
// TODO: can we somehow manage vertex and index buffers? so we can have 1 big
// vertex buffer for model and each mesh will reffer to the part of it
struct RenderGeometryMesh {
  std::shared_ptr<VertexBuffer> vertexBuffer;
  std::shared_ptr<IndexBuffer>  indexBuffer;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_GEOMETRY_MESH_H
