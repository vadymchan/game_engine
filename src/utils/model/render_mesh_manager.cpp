#include "utils/model/render_mesh_manager.h"

#include "utils/logger/global_logger.h"

namespace game_engine {

RenderMesh* RenderMeshManager::addRenderMesh(RenderGeometryMesh* gpuMesh, Material* material, Mesh* sourceMesh) {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (!gpuMesh || !sourceMesh) {
    GlobalLogger::Log(LogLevel::Error, "Cannot create render mesh with null GPU mesh or source mesh");
    return nullptr;
  }

  // Check if render mesh already exists for this mesh
  auto it = m_renderMeshes.find(sourceMesh);
  if (it != m_renderMeshes.end()) {
    GlobalLogger::Log(LogLevel::Warning, "Render mesh already exists for this mesh. Overwriting.");
  }

  auto renderMesh      = std::make_unique<RenderMesh>();
  renderMesh->gpuMesh  = gpuMesh;
  renderMesh->material = material;

  RenderMesh* meshPtr = renderMesh.get();

  m_renderMeshes[sourceMesh] = std::move(renderMesh);

  return meshPtr;
}

RenderMesh* RenderMeshManager::getRenderMesh(Mesh* sourceMesh) {
  std::lock_guard<std::mutex> lock(m_mutex);

  auto it = m_renderMeshes.find(sourceMesh);
  if (it != m_renderMeshes.end()) {
    return it->second.get();
  }

  GlobalLogger::Log(LogLevel::Warning, "No render mesh found for the specified mesh");
  return nullptr;
}

}  // namespace game_engine