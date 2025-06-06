#ifndef ARISE_ASSIMP_MODEL_LOADER_H
#define ARISE_ASSIMP_MODEL_LOADER_H

// TODO: assimp will be deprecated soon from the project
#ifdef ARISE_USE_ASSIMP

#include "resources/i_model_loader.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <math_library/vector.h>

#include <assimp/Importer.hpp>

namespace arise {

class AssimpModelLoader : public IModelLoader {
  public:
  std::unique_ptr<Model> loadModel(const std::filesystem::path& filePath) override;

  std::vector<std::unique_ptr<Mesh>> processMeshes(const aiScene* scene);

  private:
  // TODO: maybe make aiMesh constant?
  std::unique_ptr<Mesh>                       processMesh(aiMesh* ai_mesh);
  void                                        processVertices(aiMesh* ai_mesh, Mesh* mesh);
  Vertex                                      processVertex(aiMesh* ai_mesh, unsigned int index);
  math::Vector3f                             processPosition(aiMesh* ai_mesh, unsigned int index);
  math::Vector3f                             processNormal(aiMesh* ai_mesh, unsigned int index);
  math::Vector2f                             processTexCoords(aiMesh* ai_mesh, unsigned int index);
  std::pair<math::Vector3f, math::Vector3f> processTangentBitangent(aiMesh* ai_mesh, unsigned int index);
  math::Vector4f                             processColor(aiMesh* ai_mesh, unsigned int index);
  void                                        processIndices(aiMesh* ai_mesh, Mesh* mesh);
  void calculateTangentsAndBitangents(aiMesh* ai_mesh, std::vector<Vertex>& vertices);
};

}  // namespace arise

#endif  // ARISE_USE_ASSIMP

#endif  // ARISE_ASSIMP_MODEL_LOADER_H