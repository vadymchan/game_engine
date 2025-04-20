#include "utils/model/render_geometry_mesh_manager.h"

#include "utils/logger/global_logger.h"

namespace game_engine {

RenderGeometryMesh* RenderGeometryMeshManager::addRenderGeometryMesh(
    std::unique_ptr<RenderGeometryMesh> renderGeometryMesh, Mesh* sourceMesh) {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!renderGeometryMesh || !sourceMesh) {
    GlobalLogger::Log(LogLevel::Error, "Attempting to add null render geometry mesh or source mesh");
    return nullptr;
  }

  // Check if GPU geometry already exists for this mesh
  auto it = m_renderGeometryMeshes.find(sourceMesh);
  if (it != m_renderGeometryMeshes.end()) {
    GlobalLogger::Log(LogLevel::Warning, "Render geometry already exists for this mesh. Overwriting.");
  }

  RenderGeometryMesh* meshPtr = renderGeometryMesh.get();

  m_renderGeometryMeshes[sourceMesh] = std::move(renderGeometryMesh);

  return meshPtr;
}

RenderGeometryMesh* RenderGeometryMeshManager::getRenderGeometryMesh(Mesh* sourceMesh) {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_renderGeometryMeshes.find(sourceMesh);
  if (it != m_renderGeometryMeshes.end()) {
    return it->second.get();
  }

  GlobalLogger::Log(LogLevel::Warning, "No render geometry found for the specified mesh");
  return nullptr;
}

}  // namespace game_engine