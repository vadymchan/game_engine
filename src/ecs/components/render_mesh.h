#ifndef GAME_ENGINE_RENDER_MESH_H
#define GAME_ENGINE_RENDER_MESH_H

#include "ecs/components/material.h"
#include "ecs/components/render_geometry_mesh.h"

#include <memory>

namespace game_engine {

struct RenderMesh {
  RenderGeometryMesh* gpuMesh;
  Material*           material;
  gfx::rhi::Buffer*   transformMatrixBuffer = nullptr;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_MESH_H
