#ifndef GAME_ENGINE_GLTF_MODEL_LOADER_H
#define GAME_ENGINE_GLTF_MODEL_LOADER_H

#ifdef GAME_ENGINE_USE_CGLTF

#include "resources/i_model_loader.h"

#include <math_library/vector.h>

// Forward declarations
struct cgltf_data;
struct cgltf_node;
struct cgltf_mesh;
struct cgltf_primitive;

namespace game_engine {

class CgltfModelLoader : public IModelLoader {
  public:
  std::unique_ptr<Model> loadModel(const std::filesystem::path& filePath) override;

  private:
  std::vector<std::unique_ptr<Mesh>> processMeshes(const cgltf_data* data);
  math::Matrix4f<>                   calculateWorldMatrix(cgltf_node* node, const math::Matrix4f<>& localMatrix);
  bool                               containsMesh(cgltf_node* node);
  math::Matrix4f<>                   getNodeTransformMatrix(const cgltf_node* node);
  std::unique_ptr<Mesh>              processMesh(const cgltf_mesh* mesh);
  std::unique_ptr<Mesh>              processPrimitive(const cgltf_primitive* primitive);
  void                               processVertices(const cgltf_primitive* primitive, Mesh* mesh);
  void                               processIndices(const cgltf_primitive* primitive, Mesh* mesh);
  void                               calculateTangents(Mesh* mesh);

#ifdef GAME_ENGINE_USE_MIKKTS
  void generateMikkTSpaceTangents(Mesh* mesh);
#endif
};

}  // namespace game_engine

#endif  // GAME_ENGINE_USE_CGLTF

#endif  // GAME_ENGINE_GLTF_MODEL_LOADER_H