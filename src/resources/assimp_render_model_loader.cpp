#include "resources/assimp_render_model_loader.h"

#include "resources/assimp_material_loader.h"
#include "resources/assimp_model_loader.h"
#include "utils/buffer/buffer_manager.h"
#include "utils/material/material_manager.h"
#include "utils/model/mesh_manager.h"
#include "utils/model/render_geometry_mesh_manager.h"
#include "utils/model/render_mesh_manager.h"
#include "utils/service/service_locator.h"

namespace game_engine {

AssimpRenderModelLoader::AssimpRenderModelLoader(gfx::rhi::Device* device)
    : m_device(device) {
  if (!m_device) {
    GlobalLogger::Log(LogLevel::Error, "Device is null");
  }
}

std::unique_ptr<RenderModel> AssimpRenderModelLoader::loadRenderModel(const std::filesystem::path& filePath,
                                                                      std::optional<Model*>        outModel) {
  Assimp::Importer importer;

  const aiScene* scene = importer.ReadFile(
      filePath.string(),
      aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

  if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
    GlobalLogger::Log(LogLevel::Error,
                      "Failed to load file \"" + filePath.string() + "\". Error: " + importer.GetErrorString());
    return nullptr;
  }

  // Get required managers from service locator
  auto materialManager           = ServiceLocator::s_get<MaterialManager>();
  auto renderGeometryMeshManager = ServiceLocator::s_get<RenderGeometryMeshManager>();
  auto renderMeshManager         = ServiceLocator::s_get<RenderMeshManager>();
  auto meshManager               = ServiceLocator::s_get<MeshManager>();

  if (!materialManager || !renderGeometryMeshManager || !renderMeshManager || !meshManager) {
    GlobalLogger::Log(LogLevel::Error, "Required managers not available in ServiceLocator.");
    return nullptr;
  }

  // Create instances of the loaders
  AssimpModelLoader modelLoader;

  // Create a material loader with the device
  AssimpMaterialLoader materialLoader(m_device);

  // Store processed materials in material manager
  std::vector<Material*> materialPointers = materialManager->getMaterials(filePath);

  // Create CPU model
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

    // Create GPU-side geometry
    auto renderGeometryMesh = createRenderGeometryMesh(meshPtr);

    // Add to manager
    RenderGeometryMesh* gpuMeshPtr
        = renderGeometryMeshManager->addRenderGeometryMesh(std::move(renderGeometryMesh), meshPtr);

    // Get corresponding material
    Material*    materialPtr   = nullptr;
    unsigned int materialIndex = ai_mesh->mMaterialIndex;
    if (materialIndex < materialPointers.size()) {
      materialPtr = materialPointers[materialIndex];
    }

    // Create render mesh and add to manager
    RenderMesh* renderMeshPtr = renderMeshManager->addRenderMesh(gpuMeshPtr, materialPtr, meshPtr);

    // Add to render model
    renderModel->renderMeshes.push_back(renderMeshPtr);
  }

  // If the caller wants the CPU model, provide it
  if (outModel.has_value()) {
    *outModel = cpuModel.release();
  }

  return renderModel;
}

// Function to create GPU-side geometry mesh
std::unique_ptr<RenderGeometryMesh> AssimpRenderModelLoader::createRenderGeometryMesh(Mesh* mesh) {
  auto renderGeometryMesh = std::make_unique<RenderGeometryMesh>();

  // Create vertex and index buffers
  renderGeometryMesh->vertexBuffer = createVertexBuffer(mesh);
  renderGeometryMesh->indexBuffer  = createIndexBuffer(mesh);

  // Store counts for convenience
  // renderGeometryMesh->vertexCount = static_cast<uint32_t>(mesh->vertices.size());
  // renderGeometryMesh->indexCount  = static_cast<uint32_t>(mesh->indices.size());

  // Store stride information
  // renderGeometryMesh->vertexStride = sizeof(Vertex);

  return renderGeometryMesh;
}

gfx::rhi::Buffer* AssimpRenderModelLoader::createVertexBuffer(const Mesh* mesh) {
  if (!m_device) {
    GlobalLogger::Log(LogLevel::Error, "Cannot create vertex buffer, device is null");
    return nullptr;
  }

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
  if (!m_device) {
    GlobalLogger::Log(LogLevel::Error, "Cannot create index buffer, device is null");
    return nullptr;
  }

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