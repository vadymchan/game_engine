#ifndef GAME_ENGINE_RENDER_MESH_MANAGER_H
#define GAME_ENGINE_RENDER_MESH_MANAGER_H

#include "ecs/components/material.h"
#include "ecs/components/mesh.h"
#include "ecs/components/render_geometry_mesh.h"
#include "ecs/components/render_mesh.h"

#include <memory>
#include <mutex>
#include <unordered_map>

namespace game_engine {

class RenderMeshManager {
  public:
  RenderMeshManager()  = default;
  ~RenderMeshManager() = default;

  /**
   * @param sourceMesh Associated CPU mesh for identification
   */
  RenderMesh* addRenderMesh(RenderGeometryMesh* gpuMesh, Material* material, Mesh* sourceMesh);

  RenderMesh* getRenderMesh(Mesh* sourceMesh);

  bool removeRenderMesh(RenderMesh* renderMesh);

  private:
  // Map of render meshes keyed by CPU mesh pointer
  std::unordered_map<Mesh*, std::unique_ptr<RenderMesh>> m_renderMeshes;
  mutable std::mutex                                     m_mutex;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_MESH_MANAGER_H