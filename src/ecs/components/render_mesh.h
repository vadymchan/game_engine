#ifndef ARISE_RENDER_MESH_H
#define ARISE_RENDER_MESH_H

#include "ecs/components/material.h"
#include "ecs/components/render_geometry_mesh.h"

#include <memory>

namespace arise {

struct RenderMesh {
  RenderGeometryMesh* gpuMesh;
  Material*           material;
  gfx::rhi::Buffer*   transformMatrixBuffer = nullptr;
};

}  // namespace arise

#endif  // ARISE_RENDER_MESH_H
