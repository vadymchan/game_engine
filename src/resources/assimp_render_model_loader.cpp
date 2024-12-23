#include "resources/assimp_render_model_loader.h"

#include "resources/assimp_material_loader.h"
#include "resources/assimp_model_loader.h"

#include <gfx/rhi/rhi.h>

namespace game_engine {
std::shared_ptr<RenderModel> AssimpRenderModelLoader::loadRenderModel(
    const std::filesystem::path&          filePath,
    std::optional<std::shared_ptr<Model>> outModel) {
  Assimp::Importer importer;

  const aiScene* scene
      = importer.ReadFile(filePath.string(),
                          aiProcess_Triangulate | aiProcess_GenSmoothNormals
                              | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

  if (!scene || !scene->mRootNode
      || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
    // TODO: Log an error here
    return nullptr;
  }

  // Create instances of the loaders
  // TODO: consider moving to other place
  AssimpModelLoader    modelLoader;
  AssimpMaterialLoader materialLoader;

  // Process materials
  auto materials = materialLoader.processMaterials(scene, filePath);

  // Process meshes
  std::vector<std::shared_ptr<Mesh>> meshes = modelLoader.processMeshes(scene);

  // Create RenderMeshes by mapping meshes to materials
  std::vector<std::shared_ptr<RenderMesh>> renderMeshes;

  for (size_t i = 0; i < meshes.size(); ++i) {
    auto&   mesh    = meshes[i];
    aiMesh* ai_mesh = scene->mMeshes[i];

    // Get the material index from aiMesh
    unsigned int materialIndex = ai_mesh->mMaterialIndex;

    // Get the corresponding material
    std::shared_ptr<Material> material;
    if (materialIndex < materials.size()) {
      material = materials[materialIndex];
    } else {
      // Handle error or use a default material
      material = nullptr;  // Or create a default material
    }

    // Create RenderGeometryMesh
    auto renderGeometryMesh = createRenderGeometryMesh(mesh);

    // Create RenderMesh
    auto renderMesh      = std::make_shared<RenderMesh>();
    renderMesh->gpuMesh  = renderGeometryMesh;
    renderMesh->material = material;

    renderMeshes.emplace_back(renderMesh);
  }

  // Create the RenderModel
  auto renderModel          = std::make_shared<RenderModel>();
  renderModel->filePath     = filePath;
  renderModel->renderMeshes = renderMeshes;

  // Optionally assign the CPU-side geometry model
  if (outModel.has_value()) {
    auto model = std::make_shared<Model>();
    // model->modelName = filePath.stem().string();
    model->filePath = filePath;
    model->meshes   = meshes;
    outModel        = model;
  }

  return renderModel;
}

// Function to create GPU-side geometry mesh

std::shared_ptr<RenderGeometryMesh>
    AssimpRenderModelLoader::createRenderGeometryMesh(
        const std::shared_ptr<Mesh>& mesh) {
  auto vertexStreamData = createVertexStreamData(mesh);
  auto indexStreamData  = createIndexStreamData(mesh);

  auto vertexBuffer = createVertexBuffer(vertexStreamData);
  auto indexBuffer  = createIndexBuffer(indexStreamData);

  auto renderGeometryMesh          = std::make_shared<RenderGeometryMesh>();
  renderGeometryMesh->vertexBuffer = vertexBuffer;
  renderGeometryMesh->indexBuffer  = indexBuffer;

  return renderGeometryMesh;
}

std::shared_ptr<VertexStreamData>
    AssimpRenderModelLoader::createVertexStreamData(
        const std::shared_ptr<Mesh>& mesh) {
  // TODO: currently not the best implementation (we will store both in ECS -
  // Model and in VertexStreamData / IndexStreamData). This is not memory
  // efficient + it's error prone to store the same data in two places.

  auto vertexStreamData = std::make_shared<VertexStreamData>();

  // Positions
  {
    std::vector<float> positions;
    positions.reserve(mesh->vertices.size() * 3);
    for (const auto& vertex : mesh->vertices) {
      positions.push_back(vertex.position.x());
      positions.push_back(vertex.position.y());
      positions.push_back(vertex.position.z());
    }
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("POSITION"),
        EBufferType::Static,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        std::move(positions));
    vertexStreamData->m_streams_.push_back(streamParam);
  }

  // Normals
  {
    std::vector<float> normals;
    normals.reserve(mesh->vertices.size() * 3);
    for (const auto& vertex : mesh->vertices) {
      normals.push_back(vertex.normal.x());
      normals.push_back(vertex.normal.y());
      normals.push_back(vertex.normal.z());
    }
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("NORMAL"),
        EBufferType::Static,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        std::move(normals));
    vertexStreamData->m_streams_.push_back(streamParam);
  }

  // TexCoords
  {
    std::vector<float> texCoords;
    texCoords.reserve(mesh->vertices.size() * 2);
    for (const auto& vertex : mesh->vertices) {
      texCoords.push_back(vertex.texCoords.x());
      texCoords.push_back(vertex.texCoords.y());
    }
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("TEXCOORD"),
        EBufferType::Static,
        sizeof(float) * 2,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 2)},
        std::move(texCoords));
    vertexStreamData->m_streams_.push_back(streamParam);
  }

  // Tangents
  {
    std::vector<float> tangents;
    tangents.reserve(mesh->vertices.size() * 3);
    for (const auto& vertex : mesh->vertices) {
      tangents.push_back(vertex.tangent.x());
      tangents.push_back(vertex.tangent.y());
      tangents.push_back(vertex.tangent.z());
    }
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("TANGENT"),
        EBufferType::Static,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        std::move(tangents));
    vertexStreamData->m_streams_.push_back(streamParam);
  }

  // Bitangents (optional)
  {
    std::vector<float> bitangents;
    bitangents.reserve(mesh->vertices.size() * 3);
    for (const auto& vertex : mesh->vertices) {
      bitangents.push_back(vertex.bitangent.x());
      bitangents.push_back(vertex.bitangent.y());
      bitangents.push_back(vertex.bitangent.z());
    }
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("BITANGENT"),
        EBufferType::Static,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        std::move(bitangents));
    vertexStreamData->m_streams_.push_back(streamParam);
  }

  // Colors (if present)
  {
    std::vector<float> colors;
    colors.reserve(mesh->vertices.size() * 4);
    for (const auto& vertex : mesh->vertices) {
      colors.push_back(vertex.color.x());
      colors.push_back(vertex.color.y());
      colors.push_back(vertex.color.z());
      colors.push_back(vertex.color.w());
    }
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("COLOR"),
        EBufferType::Static,
        sizeof(float) * 4,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 4)},
        std::move(colors));
    vertexStreamData->m_streams_.push_back(streamParam);
  }

  // Set primitive type and element count
  vertexStreamData->m_primitiveType_ = EPrimitiveType::TRIANGLES;
  vertexStreamData->m_elementCount_
      = static_cast<int32_t>(mesh->vertices.size());

  return vertexStreamData;
}

std::shared_ptr<IndexStreamData> AssimpRenderModelLoader::createIndexStreamData(
    const std::shared_ptr<Mesh>& mesh) {
  auto indexStreamData = std::make_shared<IndexStreamData>();
  indexStreamData->m_elementCount_
      = static_cast<uint32_t>(mesh->indices.size());

  auto streamParam = new BufferAttributeStream<uint32_t>(
      Name("Index"),
      EBufferType::Static,
      sizeof(uint32_t),
      {IBufferAttribute::Attribute(
          EBufferElementType::UINT32, 0, sizeof(uint32_t))},
      mesh->indices);

  indexStreamData->m_stream_ = streamParam;

  return indexStreamData;
}

std::shared_ptr<VertexBuffer> AssimpRenderModelLoader::createVertexBuffer(
    const std::shared_ptr<VertexStreamData>& vertexStreamData) {
  auto vertexBuffer = g_rhi->createVertexBuffer(vertexStreamData);

  return vertexBuffer;
}

std::shared_ptr<IndexBuffer> AssimpRenderModelLoader::createIndexBuffer(
    const std::shared_ptr<IndexStreamData>& indexStreamData) {
  auto indexBuffer = g_rhi->createIndexBuffer(indexStreamData);

  return indexBuffer;
}

}  // namespace game_engine
