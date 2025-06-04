#ifdef ARISE_USE_ASSIMP

#include "resources/assimp/assimp_model_loader.h"

#include "resources/assimp/asssimp_common.h"
#include "utils/logger/global_logger.h"
#include "utils/model/mesh_manager.h"
#include "utils/service/service_locator.h"

namespace arise {

std::unique_ptr<Model> AssimpModelLoader::loadModel(const std::filesystem::path& filePath) {
  Assimp::Importer importer;

  auto scenePtr = AssimpSceneCache::getOrLoad(filePath);
  if (!scenePtr) {
    return nullptr;
  }
  const aiScene* scene = scenePtr.get();

  auto model      = std::make_unique<Model>();
  model->filePath = filePath;

  auto meshManager = ServiceLocator::s_get<MeshManager>();
  if (!meshManager) {
    GlobalLogger::Log(LogLevel::Error, "MeshManager not available in ServiceLocator.");
    return nullptr;
  }

  model->meshes.reserve(scene->mNumMeshes);

  for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
    aiMesh* ai_mesh = scene->mMeshes[i];

    auto mesh = processMesh(ai_mesh);

    Mesh* meshPtr = meshManager->addMesh(std::move(mesh), filePath);

    model->meshes.push_back(meshPtr);
  }

  return model;
}

std::vector<std::unique_ptr<Mesh>> AssimpModelLoader::processMeshes(const aiScene* scene) {
  std::vector<std::unique_ptr<Mesh>> meshes;
  // TODO: consider that there exists aiNode, so current implementation with
  // aiMesh may be incorrect + consider that there are mesh transforms exists
  meshes.reserve(scene->mNumMeshes);

  for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
    aiMesh* ai_mesh = scene->mMeshes[i];
    meshes.push_back(processMesh(ai_mesh));
  }
  return meshes;
}

std::unique_ptr<Mesh> AssimpModelLoader::processMesh(aiMesh* ai_mesh) {
  auto mesh      = std::make_unique<Mesh>();
  mesh->meshName = ai_mesh->mName.C_Str();

  processVertices(ai_mesh, mesh.get());
  processIndices(ai_mesh, mesh.get());

  return mesh;
}

void AssimpModelLoader::processVertices(aiMesh* ai_mesh, Mesh* mesh) {
  mesh->vertices.reserve(ai_mesh->mNumVertices);
  for (unsigned int i = 0; i < ai_mesh->mNumVertices; ++i) {
    Vertex vertex = processVertex(ai_mesh, i);
    mesh->vertices.emplace_back(vertex);
  }
  if (!ai_mesh->HasTangentsAndBitangents()) {
    calculateTangentsAndBitangents(ai_mesh, mesh->vertices);
  }
}

Vertex AssimpModelLoader::processVertex(aiMesh* ai_mesh, unsigned int index) {
  Vertex vertex;
  vertex.position  = processPosition(ai_mesh, index);
  vertex.normal    = processNormal(ai_mesh, index);
  vertex.texCoords = processTexCoords(ai_mesh, index);

  auto [tangent, bitangent] = processTangentBitangent(ai_mesh, index);
  vertex.tangent            = tangent;
  vertex.bitangent          = bitangent;

  vertex.color = processColor(ai_mesh, index);
  return vertex;
}

math::Vector3f AssimpModelLoader::processPosition(aiMesh* ai_mesh, unsigned int index) {
  aiVector3D pos = ai_mesh->mVertices[index];
  return math::Vector3f(pos.x, pos.y, pos.z);
}

math::Vector3f AssimpModelLoader::processNormal(aiMesh* ai_mesh, unsigned int index) {
  bool       hasNormals = ai_mesh->HasNormals();
  aiVector3D normal     = hasNormals ? ai_mesh->mNormals[index] : aiVector3D(0.0f, 0.0f, 0.0f);
  return math::Vector3f(normal.x, normal.y, normal.z);
}

math::Vector2f AssimpModelLoader::processTexCoords(aiMesh* ai_mesh, unsigned int index) {
  // TODO: currently only one channel is used, but in future it may change
  bool       hasTexCoords = ai_mesh->HasTextureCoords(0);
  aiVector3D texCoord     = hasTexCoords ? ai_mesh->mTextureCoords[0][index] : aiVector3D(0.0f, 0.0f, 0.0f);
  return math::Vector2f(texCoord.x, texCoord.y);
}

std::pair<math::Vector3f, math::Vector3f> AssimpModelLoader::processTangentBitangent(aiMesh*      ai_mesh,
                                                                                       unsigned int index) {
  bool            hasTangents = ai_mesh->HasTangentsAndBitangents();
  math::Vector3f tangent, bitangent;

  if (hasTangents) {
    aiVector3D aiTangent   = ai_mesh->mTangents[index];
    aiVector3D aiBitangent = ai_mesh->mBitangents[index];
    tangent                = math::Vector3f(aiTangent.x, aiTangent.y, aiTangent.z);
    bitangent              = math::Vector3f(aiBitangent.x, aiBitangent.y, aiBitangent.z);
  } else {
    GlobalLogger::Log(LogLevel::Warning, "No tangents found in mesh. Computing manually.");
  }

  return {tangent, bitangent};
}

math::Vector4f AssimpModelLoader::processColor(aiMesh* ai_mesh, unsigned int index) {
  bool      hasColors = ai_mesh->HasVertexColors(0);
  aiColor4D color     = hasColors ? ai_mesh->mColors[0][index] : aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
  return math::Vector4f(color.r, color.g, color.b, color.a);
}

void AssimpModelLoader::processIndices(aiMesh* ai_mesh, Mesh* mesh) {
  for (unsigned int i = 0; i < ai_mesh->mNumFaces; ++i) {
    aiFace face = ai_mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; ++j) {
      mesh->indices.emplace_back(face.mIndices[j]);
    }
  }
}

// Note: depend on normals in std::vector<Vertex>& vertices. Needs to be
// assigned beforehand
void AssimpModelLoader::calculateTangentsAndBitangents(aiMesh* ai_mesh, std::vector<Vertex>& vertices) {
  for (unsigned int i = 0; i < ai_mesh->mNumFaces; ++i) {
    const aiFace& face = ai_mesh->mFaces[i];
    // Only process triangles
    if (face.mNumIndices != 3) {
      GlobalLogger::Log(LogLevel::Debug,
                        "Skipping face with " + std::to_string(face.mNumIndices) + " indices (not a triangle).");
      continue;
    }

    uint32_t i0 = face.mIndices[0];
    uint32_t i1 = face.mIndices[1];
    uint32_t i2 = face.mIndices[2];

    Vertex& v0 = vertices[i0];
    Vertex& v1 = vertices[i1];
    Vertex& v2 = vertices[i2];

    const math::Vector3f& p0 = v0.position;
    const math::Vector3f& p1 = v1.position;
    const math::Vector3f& p2 = v2.position;

    const math::Vector2f& uv0 = v0.texCoords;
    const math::Vector2f& uv1 = v1.texCoords;
    const math::Vector2f& uv2 = v2.texCoords;

    math::Vector3f deltaPos1 = p1 - p0;
    math::Vector3f deltaPos2 = p2 - p0;

    math::Vector2f deltaUV1 = uv1 - uv0;
    math::Vector2f deltaUV2 = uv2 - uv0;

    constexpr float epsilon = 1e-6f;
    float           r       = deltaUV1.x() * deltaUV2.y() - deltaUV1.y() * deltaUV2.x();
    r                       = (std::abs(r) > epsilon) ? 1.0f / r : 1.0f;

    math::Vector3f tangent   = r * (deltaPos1 * deltaUV2.y() - deltaPos2 * deltaUV1.y());
    math::Vector3f bitangent = r * (deltaPos2 * deltaUV1.x() - deltaPos1 * deltaUV2.x());

    tangent = (tangent - v0.normal * v0.normal.dot(tangent)).normalized();

    float handedness = (v0.normal.cross(tangent).dot(bitangent) < 0.0f) ? -1.0f : 1.0f;
    bitangent        = (v0.normal.cross(tangent) * handedness).normalized();

    v0.tangent = tangent;
    v1.tangent = tangent;
    v2.tangent = tangent;

    v0.bitangent = bitangent;
    v1.bitangent = bitangent;
    v2.bitangent = bitangent;
  }
}

}  // namespace arise

#endif  // ARISE_USE_ASSIMP