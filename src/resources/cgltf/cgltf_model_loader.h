#ifndef GAME_ENGINE_CGLTF_MODEL_LOADER_H
#define GAME_ENGINE_CGLTF_MODEL_LOADER_H
#ifdef GAME_ENGINE_USE_CGLTF

#include <math_library/matrix.h>

#include <filesystem>
#include <memory>

struct cgltf_data;
struct cgltf_node;
struct cgltf_primitive;

namespace game_engine {
struct Mesh;
struct Model;

class CgltfModelLoader {
  public:
  CgltfModelLoader()  = default;
  ~CgltfModelLoader() = default;

  std::unique_ptr<Model> loadModel(const std::filesystem::path& filePath);

  private:
  math::Matrix4f<>      calculateWorldMatrix(cgltf_node* node, const math::Matrix4f<>& localMatrix);
  bool                  containsMesh(cgltf_node* node);
  math::Matrix4f<>      getNodeTransformMatrix(const cgltf_node* node);
  std::unique_ptr<Mesh> processPrimitive(const cgltf_primitive* primitive);
  void                  processVertices(const cgltf_primitive* primitive, Mesh* mesh);
  void                  processIndices(const cgltf_primitive* primitive, Mesh* mesh);
  void                  calculateTangents(Mesh* mesh);

#ifdef GAME_ENGINE_USE_MIKKTS
  void generateMikkTSpaceTangents(Mesh* mesh);
#endif
};
}  // namespace game_engine

#endif  // GAME_ENGINE_USE_CGLTF
#endif  // GAME_ENGINE_CGLTF_MODEL_LOADER_H