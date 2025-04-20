#ifndef GAME_ENGINE_RENDER_GEOMETRY_MESH_MANAGER_H
#define GAME_ENGINE_RENDER_GEOMETRY_MESH_MANAGER_H

#include "ecs/components/mesh.h"
#include "ecs/components/render_geometry_mesh.h"

#include <memory>
#include <mutex>
#include <unordered_map>

namespace game_engine {

class RenderGeometryMeshManager {
  public:
  RenderGeometryMeshManager()  = default;
  ~RenderGeometryMeshManager() = default;

  /**
   * @param sourceMesh Associated CPU mesh for identification
   */
  RenderGeometryMesh* addRenderGeometryMesh(std::unique_ptr<RenderGeometryMesh> renderGeometryMesh, Mesh* sourceMesh);

  RenderGeometryMesh* getRenderGeometryMesh(Mesh* sourceMesh);

  private:
  // Map of GPU geometry meshes keyed by CPU mesh pointer
  std::unordered_map<Mesh*, std::unique_ptr<RenderGeometryMesh>> m_renderGeometryMeshes;
  mutable std::mutex                                             m_mutex;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_GEOMETRY_MESH_MANAGER_H