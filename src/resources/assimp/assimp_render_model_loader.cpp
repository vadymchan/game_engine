#ifdef GAME_ENGINE_USE_ASSIMP

#include "resources/assimp/assimp_render_model_loader.h"

#include "resources/assimp/assimp_material_loader.h"
#include "resources/assimp/assimp_model_loader.h"
#include "resources/assimp/asssimp_common.h"
#include "utils/buffer/buffer_manager.h"
#include "utils/material/material_manager.h"
#include "utils/model/mesh_manager.h"
#include "utils/model/render_geometry_mesh_manager.h"
#include "utils/model/render_mesh_manager.h"
#include "utils/service/service_locator.h"

namespace game_engine {

std::unique_ptr<RenderModel> AssimpRenderModelLoader::loadRenderModel(const std::filesystem::path& filePath,
                                                                      std::optional<Model*>        outModel) {
  Assimp::Importer importer;

    auto scenePtr = AssimpSceneCache::getOrLoad(filePath);
  if (!scenePtr) {
    return nullptr;
  }
  const aiScene* scene = scenePtr.get();

  auto materialManager           = ServiceLocator::s_get<MaterialManager>();
  auto renderGeometryMeshManager = ServiceLocator::s_get<RenderGeometryMeshManager>();
  auto renderMeshManager         = ServiceLocator::s_get<RenderMeshManager>();
  auto meshManager               = ServiceLocator::s_get<MeshManager>();

  if (!materialManager || !renderGeometryMeshManager || !renderMeshManager || !meshManager) {
    GlobalLogger::Log(LogLevel::Error, "Required managers not available in ServiceLocator.");
    return nullptr;
  }

  AssimpModelLoader modelLoader;

  std::vector<Material*> materialPointers = materialManager->getMaterials(filePath);

  auto cpuModel = modelLoader.loadModel(filePath);
  if (!cpuModel) {
    GlobalLogger::Log(LogLevel::Error, "Failed to load CPU model from " + filePath.string());
    return nullptr;
  }

  auto& meshes = cpuModel->meshes;

  auto renderModel      = std::make_unique<RenderModel>();
  renderModel->filePath = filePath;
  renderModel->renderMeshes.reserve(meshes.size());

  for (size_t i = 0; i < meshes.size(); ++i) {
    Mesh*   meshPtr = meshes[i];
    aiMesh* ai_mesh = scene->mMeshes[i];

    auto renderGeometryMesh = createRenderGeometryMesh(meshPtr);

    RenderGeometryMesh* gpuMeshPtr
        = renderGeometryMeshManager->addRenderGeometryMesh(std::move(renderGeometryMesh), meshPtr);

    Material*    materialPtr   = nullptr;
    unsigned int materialIndex = ai_mesh->mMaterialIndex;
    if (materialIndex < materialPointers.size()) {
      materialPtr = materialPointers[materialIndex];
    }

    RenderMesh* renderMeshPtr = renderMeshManager->addRenderMesh(gpuMeshPtr, materialPtr, meshPtr);

    renderModel->renderMeshes.push_back(renderMeshPtr);
  }

  if (outModel.has_value()) {
    *outModel = cpuModel.release();
  }

  return renderModel;
}

std::unique_ptr<RenderGeometryMesh> AssimpRenderModelLoader::createRenderGeometryMesh(Mesh* mesh) {
  auto renderGeometryMesh = std::make_unique<RenderGeometryMesh>();

  renderGeometryMesh->vertexBuffer = createVertexBuffer(mesh);
  renderGeometryMesh->indexBuffer  = createIndexBuffer(mesh);

  return renderGeometryMesh;
}

gfx::rhi::Buffer* AssimpRenderModelLoader::createVertexBuffer(const Mesh* mesh) {
  auto bufferManager = ServiceLocator::s_get<BufferManager>();
  if (!bufferManager) {
    GlobalLogger::Log(LogLevel::Error, "Cannot create vertex buffer, BufferManager not found");
    return nullptr;
  }

  // TODO: consider rewrite unique ID key
  std::string bufferName  = "VertexBuffer_";
  bufferName             += mesh->meshName.empty() ? "Unnamed" : mesh->meshName;

  return bufferManager->createVertexBuffer(mesh->vertices.data(), mesh->vertices.size(), sizeof(Vertex), bufferName);
}

gfx::rhi::Buffer* AssimpRenderModelLoader::createIndexBuffer(const Mesh* mesh) {
  auto bufferManager = ServiceLocator::s_get<BufferManager>();
  if (!bufferManager) {
    GlobalLogger::Log(LogLevel::Error, "Cannot create index buffer, BufferManager not found");
    return nullptr;
  }

  // TODO: consider rewrite unique ID key
  std::string bufferName  = "IndexBuffer_";
  bufferName             += mesh->meshName.empty() ? "Unnamed" : mesh->meshName;

  return bufferManager->createIndexBuffer(mesh->indices.data(), mesh->indices.size(), sizeof(uint32_t), bufferName);
}

}  // namespace game_engine

#endif  // GAME_ENGINE_USE_ASSIMP