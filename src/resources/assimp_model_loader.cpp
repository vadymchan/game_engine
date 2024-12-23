#include "resources/assimp_model_loader.h"

namespace game_engine {

std::shared_ptr<Model> AssimpModelLoader::loadModel(
    const std::filesystem::path& filePath) {
  Assimp::Importer importer;

  // TODO: make flags configurable
  const aiScene* scene
      = importer.ReadFile(filePath.string(),
                          aiProcess_Triangulate | aiProcess_GenSmoothNormals
                              | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

  // Check if the scene or root node is null, or if there are no meshes
  if (!scene || !scene->mRootNode
      || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
    // TODO: Log an error here
    return nullptr;
  }

  auto model = std::make_shared<Model>();
  // model->modelName = filePath.stem().string();
  model->filePath = filePath;
  model->meshes   = std::move(processMeshes(scene));

  return model;
}

std::vector<std::shared_ptr<Mesh>> AssimpModelLoader::processMeshes(
    const aiScene* scene) {
  std::vector<std::shared_ptr<Mesh>> meshes;
  // TODO: consider that there exists aiNode, so current implementation with
  // aiMesh may be incorrect + consider that there are mesh transforms exists
  for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
    aiMesh* ai_mesh = scene->mMeshes[i];
    meshes.emplace_back(processMesh(ai_mesh));
  }
  return meshes;
}

std::shared_ptr<Mesh> AssimpModelLoader::processMesh(aiMesh* ai_mesh) {
  auto mesh      = std::make_shared<Mesh>();
  mesh->meshName = ai_mesh->mName.C_Str();

  processVertices(ai_mesh, mesh);
  processIndices(ai_mesh, mesh);

  return mesh;
}

void AssimpModelLoader::processVertices(aiMesh*                ai_mesh,
                                        std::shared_ptr<Mesh>& mesh) {
  mesh->vertices.reserve(ai_mesh->mNumVertices);
  for (unsigned int i = 0; i < ai_mesh->mNumVertices; ++i) {
    Vertex vertex = processVertex(ai_mesh, i);
    mesh->vertices.emplace_back(vertex);
  }
  calculateTangentsAndBitangents(ai_mesh, mesh->vertices);
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

math::Vector3Df AssimpModelLoader::processPosition(aiMesh*      ai_mesh,
                                                   unsigned int index) {
  aiVector3D pos = ai_mesh->mVertices[index];
  return math::Vector3Df(pos.x, pos.y, pos.z);
}

math::Vector3Df AssimpModelLoader::processNormal(aiMesh*      ai_mesh,
                                                 unsigned int index) {
  bool       hasNormals = ai_mesh->HasNormals();
  aiVector3D normal
      = hasNormals ? ai_mesh->mNormals[index] : aiVector3D(0.0f, 0.0f, 0.0f);
  return math::Vector3Df(normal.x, normal.y, normal.z);
}

math::Vector2Df AssimpModelLoader::processTexCoords(aiMesh*      ai_mesh,
                                                    unsigned int index) {
  // TODO: currently only one channel is used, but in future it may change
  bool       hasTexCoords = ai_mesh->HasTextureCoords(0);
  aiVector3D texCoord     = hasTexCoords ? ai_mesh->mTextureCoords[0][index]
                                         : aiVector3D(0.0f, 0.0f, 0.0f);
  return math::Vector2Df(texCoord.x, texCoord.y);
}

std::pair<math::Vector3Df, math::Vector3Df>
    AssimpModelLoader::processTangentBitangent(aiMesh*      ai_mesh,
                                               unsigned int index) {
  bool            hasTangents = ai_mesh->HasTangentsAndBitangents();
  math::Vector3Df tangent, bitangent;

  if (hasTangents) {
    aiVector3D aiTangent   = ai_mesh->mTangents[index];
    aiVector3D aiBitangent = ai_mesh->mBitangents[index];
    tangent   = math::Vector3Df(aiTangent.x, aiTangent.y, aiTangent.z);
    bitangent = math::Vector3Df(aiBitangent.x, aiBitangent.y, aiBitangent.z);
  } else {
    // log warning that tangent computed manually
  }

  return {tangent, bitangent};
}

math::Vector4Df AssimpModelLoader::processColor(aiMesh*      ai_mesh,
                                                unsigned int index) {
  bool      hasColors = ai_mesh->HasVertexColors(0);
  aiColor4D color     = hasColors ? ai_mesh->mColors[0][index]
                                  : aiColor4D(1.0f, 1.0f, 1.0f, 1.0f);
  return math::Vector4Df(color.r, color.g, color.b, color.a);
}

void AssimpModelLoader::processIndices(aiMesh*                ai_mesh,
                                       std::shared_ptr<Mesh>& mesh) {
  for (unsigned int i = 0; i < ai_mesh->mNumFaces; ++i) {
    aiFace face = ai_mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; ++j) {
      mesh->indices.emplace_back(face.mIndices[j]);
    }
  }
}

// Note: depend on normals in std::vector<Vertex>& vertices. Needs to be
// assigned beforehand
void AssimpModelLoader::calculateTangentsAndBitangents(
    aiMesh* ai_mesh, std::vector<Vertex>& vertices) {
  for (unsigned int i = 0; i < ai_mesh->mNumFaces; ++i) {
    const aiFace& face = ai_mesh->mFaces[i];
    if (face.mNumIndices != 3) {
      // TODO: log warning
      continue;  // Only process triangles
    }

    // Indices for the triangle vertices
    uint32_t i0 = face.mIndices[0];
    uint32_t i1 = face.mIndices[1];
    uint32_t i2 = face.mIndices[2];

    // Get the vertices of the triangle
    Vertex& v0 = vertices[i0];
    Vertex& v1 = vertices[i1];
    Vertex& v2 = vertices[i2];

    // Positions
    const math::Vector3Df& p0 = v0.position;
    const math::Vector3Df& p1 = v1.position;
    const math::Vector3Df& p2 = v2.position;

    // Texture coordinates
    const math::Vector2Df& uv0 = v0.texCoords;
    const math::Vector2Df& uv1 = v1.texCoords;
    const math::Vector2Df& uv2 = v2.texCoords;

    // Edge vectors
    math::Vector3Df deltaPos1 = p1 - p0;
    math::Vector3Df deltaPos2 = p2 - p0;

    // UV delta vectors
    math::Vector2Df deltaUV1 = uv1 - uv0;
    math::Vector2Df deltaUV2 = uv2 - uv0;

    constexpr float epsilon = 1e-6f;
    float r = deltaUV1.x() * deltaUV2.y() - deltaUV1.y() * deltaUV2.x();
    r = (std::abs(r) > epsilon) ? 1.0f / r : 1.0f;  // Prevent division by zero

    math::Vector3Df tangent
        = r * (deltaPos1 * deltaUV2.y() - deltaPos2 * deltaUV1.y());
    math::Vector3Df bitangent
        = r * (deltaPos2 * deltaUV1.x() - deltaPos1 * deltaUV2.x());

    // Orthogonalize tangent with respect to the normal and normalize it
    tangent = (tangent - v0.normal * v0.normal.dot(tangent)).normalized();

    // Determine handedness to make tangent and bitangent perpendicular
    float handedness
        = (v0.normal.cross(tangent).dot(bitangent) < 0.0f) ? -1.0f : 1.0f;
    bitangent = (v0.normal.cross(tangent) * handedness).normalized();

    v0.tangent = tangent;
    v1.tangent = tangent;
    v2.tangent = tangent;

    v0.bitangent = bitangent;
    v1.bitangent = bitangent;
    v2.bitangent = bitangent;
  }
}

}  // namespace game_engine
