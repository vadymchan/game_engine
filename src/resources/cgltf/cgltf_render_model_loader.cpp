#ifdef ARISE_USE_CGLTF

#include "resources/cgltf/cgltf_render_model_loader.h"

#include "resources/cgltf/cgltf_common.h"
#include "resources/cgltf/cgltf_material_loader.h"
#include "resources/cgltf/cgltf_model_loader.h"
#include "utils/buffer/buffer_manager.h"
#include "utils/logger/global_logger.h"
#include "utils/material/material_manager.h"
#include "utils/model/mesh_manager.h"
#include "utils/model/render_geometry_mesh_manager.h"
#include "utils/model/render_mesh_manager.h"
#include "utils/service/service_locator.h"

#include <cgltf.h>
#include <math_library/matrix.h>

namespace arise {

std::unique_ptr<RenderModel> CgltfRenderModelLoader::loadRenderModel(const std::filesystem::path& filePath,
                                                                     std::optional<Model*>        outModel) {
  CgltfModelLoader modelLoader;
  auto             cpuModel = modelLoader.loadModel(filePath);
  if (!cpuModel) {
    GlobalLogger::Log(LogLevel::Error, "Failed to load CPU model from " + filePath.string());
    return nullptr;
  }

  auto scene = CgltfSceneCache::getOrLoad(filePath);
  if (!scene) {
    GlobalLogger::Log(LogLevel::Error, "Failed to load GLTF scene for material mapping: " + filePath.string());
    return nullptr;
  }
  const cgltf_data* data = scene.get();

  auto materialManager           = ServiceLocator::s_get<MaterialManager>();
  auto renderGeometryMeshManager = ServiceLocator::s_get<RenderGeometryMeshManager>();
  auto renderMeshManager         = ServiceLocator::s_get<RenderMeshManager>();
  auto bufferManager             = ServiceLocator::s_get<BufferManager>();

  if (!materialManager || !renderGeometryMeshManager || !renderMeshManager || !bufferManager) {
    GlobalLogger::Log(LogLevel::Error, "Required managers not available in ServiceLocator.");
    return nullptr;
  }

  std::vector<Material*> materialPointers = materialManager->getMaterials(filePath);

  auto renderModel      = std::make_unique<RenderModel>();
  renderModel->filePath = filePath;

  auto& meshes = cpuModel->meshes;
  renderModel->renderMeshes.reserve(meshes.size());

  size_t meshIndex = 0;

  for (size_t i = 0; i < data->meshes_count && meshIndex < meshes.size(); ++i) {
    cgltf_mesh* gltf_mesh = &data->meshes[i];

    for (size_t j = 0; j < gltf_mesh->primitives_count && meshIndex < meshes.size(); ++j) {
      Mesh* meshPtr = meshes[meshIndex++];

      auto                renderGeometryMesh = createRenderGeometryMesh(meshPtr);
      RenderGeometryMesh* gpuMeshPtr
          = renderGeometryMeshManager->addRenderGeometryMesh(std::move(renderGeometryMesh), meshPtr);

      Material* materialPtr = nullptr;

      const cgltf_primitive* primitive = &gltf_mesh->primitives[j];
      if (primitive->material) {
        for (size_t materialIndex = 0; materialIndex < data->materials_count; ++materialIndex) {
          if (&data->materials[materialIndex] == primitive->material) {
            if (materialIndex < materialPointers.size()) {
              materialPtr = materialPointers[materialIndex];
            } else {
              GlobalLogger::Log(
                  LogLevel::Warning,
                  "Material index " + std::to_string(materialIndex) + " out of range for mesh " + meshPtr->meshName);
            }
            break;
          }
        }
      }

      RenderMesh* renderMeshPtr = renderMeshManager->addRenderMesh(gpuMeshPtr, materialPtr, meshPtr);

      std::string bufferName = "transform_matrix_" + meshPtr->meshName;

      if (bufferManager) {
        renderMeshPtr->transformMatrixBuffer
            = bufferManager->createUniformBuffer(sizeof(math::Matrix4f<>), &meshPtr->transformMatrix, bufferName);

        if (renderMeshPtr->transformMatrixBuffer) {
          GlobalLogger::Log(LogLevel::Debug, "Created transform matrix buffer for mesh " + meshPtr->meshName);
        } else {
          GlobalLogger::Log(LogLevel::Warning,
                            "Failed to create transform matrix buffer for mesh " + meshPtr->meshName);
        }
      }

      renderModel->renderMeshes.push_back(renderMeshPtr);
    }
  }

  if (outModel.has_value()) {
    *outModel = cpuModel.release();
  }

  return renderModel;
}

std::unique_ptr<RenderGeometryMesh> CgltfRenderModelLoader::createRenderGeometryMesh(Mesh* mesh) {
  auto renderGeometryMesh = std::make_unique<RenderGeometryMesh>();

  renderGeometryMesh->vertexBuffer = createVertexBuffer(mesh);
  renderGeometryMesh->indexBuffer  = createIndexBuffer(mesh);

  return renderGeometryMesh;
}

gfx::rhi::Buffer* CgltfRenderModelLoader::createVertexBuffer(const Mesh* mesh) {
  auto bufferManager = ServiceLocator::s_get<BufferManager>();
  if (!bufferManager) {
    GlobalLogger::Log(LogLevel::Error, "Cannot create vertex buffer, BufferManager not found");
    return nullptr;
  }

  std::string bufferName  = "VertexBuffer_";
  bufferName             += mesh->meshName.empty() ? "Unnamed" : mesh->meshName;

  return bufferManager->createVertexBuffer(mesh->vertices.data(), mesh->vertices.size(), sizeof(Vertex), bufferName);
}

gfx::rhi::Buffer* CgltfRenderModelLoader::createIndexBuffer(const Mesh* mesh) {
  auto bufferManager = ServiceLocator::s_get<BufferManager>();
  if (!bufferManager) {
    GlobalLogger::Log(LogLevel::Error, "Cannot create index buffer, BufferManager not found");
    return nullptr;
  }

  std::string bufferName  = "IndexBuffer_";
  bufferName             += mesh->meshName.empty() ? "Unnamed" : mesh->meshName;

  return bufferManager->createIndexBuffer(mesh->indices.data(), mesh->indices.size(), sizeof(uint32_t), bufferName);
}

}  // namespace arise

#endif  // ARISE_USE_CGLTF