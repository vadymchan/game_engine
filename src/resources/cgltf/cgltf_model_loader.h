#ifndef ARISE_CGLTF_MODEL_LOADER_H
#define ARISE_CGLTF_MODEL_LOADER_H
#ifdef ARISE_USE_CGLTF

#include "resources/i_model_loader.h"

#include <math_library/matrix.h>

#include <filesystem>
#include <memory>

struct cgltf_data;
struct cgltf_node;
struct cgltf_primitive;
struct cgltf_accessor;

namespace arise {
struct Mesh;
struct Model;
struct BoundingBox;

class CgltfModelLoader : public IModelLoader {
  public:
  CgltfModelLoader()  = default;
  ~CgltfModelLoader() = default;

  std::unique_ptr<Model> loadModel(const std::filesystem::path& filePath) override;

  private:
  math::Matrix4f<>      calculateWorldMatrix(cgltf_node* node, const math::Matrix4f<>& localMatrix);
  bool                  containsMesh(cgltf_node* node);
  math::Matrix4f<>      getNodeTransformMatrix(const cgltf_node* node);
  std::unique_ptr<Mesh> processPrimitive(const cgltf_primitive* primitive);
  void                  processVertices(const cgltf_primitive* primitive, Mesh* mesh);
  void                  processIndices(const cgltf_primitive* primitive, Mesh* mesh);
  void                  calculateTangents(Mesh* mesh);
  BoundingBox           extractBoundingBoxFromAccessor(const cgltf_accessor* positionAccessor);
  BoundingBox           calculateBoundingBoxFromVertices(const Mesh* mesh);
  void                  processBoundingBox(Mesh* mesh, const cgltf_primitive* primitive);

#ifdef ARISE_USE_MIKKTS
  void generateMikkTSpaceTangents(Mesh* mesh);
#endif
};
}  // namespace arise

#endif  // ARISE_USE_CGLTF
#endif  // ARISE_CGLTF_MODEL_LOADER_H