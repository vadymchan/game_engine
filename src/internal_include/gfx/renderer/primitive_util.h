#ifndef GAME_ENGINE_PRIMITIVE_UTIL_H
#define GAME_ENGINE_PRIMITIVE_UTIL_H

#include "gfx/scene/object.h"
// TODO: consider removing this include
#include "gfx/rhi/vulkan/buffer_vk.h"
#include "gfx/rhi/vulkan/pipeline_state_info_vk.h"
#include "gfx/rhi/vulkan/shader_vk.h"
#include "gfx/scene/camera.h"
#include "utils/math/plane.h"

#include <math_library/vector.h>

#include <algorithm>

namespace game_engine {
class QuadPrimitive : public Object {
  public:
  void SetPlane(const math::Plane& plane);

  math::Plane plane;
};

class ConePrimitive : public Object {
  public:
  float           Height = 0.0f;
  float           Radius = 0.0f;
  math::Vector4Df Color;
};

class CylinderPrimitive : public Object {
  public:
  float           Height = 0.0f;
  float           Radius = 0.0f;
  math::Vector4Df Color;
};

class BillboardQuadPrimitive : public QuadPrimitive {
  public:
  Camera* camera = nullptr;

  virtual void Update(float deltaTime) override;
};

class UIQuadPrimitive : public Object {
  public:
  math::Vector2Df Pos;
  math::Vector2Df Size;

  // virtual void Draw(const std::shared_ptr<jRenderFrameContext>&
  // InRenderFrameContext, const Camera* camera, const Shader* shader, const
  // std::list<const Light*>& lights, int32_t instanceCount = 0) const
  // override;
  void             SetTexture(const TextureVk* texture);
  void             SetUniformParams(const Shader* shader) const;
  const TextureVk* GetTexture() const;
};

class FullscreenQuadPrimitive : public Object {
  public:
  // virtual void Draw(const std::shared_ptr<jRenderFrameContext>&
  // InRenderFrameContext, const Camera* camera, const Shader* shader, const
  // std::list<const Light*>& lights, int32_t instanceCount = 0) const
  // override;
  void SetUniformBuffer(const Shader* shader) const;
  void SetTexture(int                       index,
                  const TextureVk*          texture,
                  const SamplerStateInfoVk* samplerState);
};

class BoundBoxObject : public Object {
  public:
  void SetUniformBuffer(const Shader* shader);
  void UpdateBoundBox(const BoundBox& boundBox);
  void UpdateBoundBox();

  math::Vector4Df Color = math::Vector4Df(0.0f, 0.0f, 0.0f, 1.0f);
  BoundBox        boundBox;
  Object*         OwnerObject = nullptr;
};

class BoundSphereObject : public Object {
  public:
  void SetUniformBuffer(const Shader* shader);

  math::Vector4Df Color = math::Vector4Df(0.0f, 0.0f, 0.0f, 1.0f);
  BoundSphere     boundSphere;
  Object*         OwnerObject = nullptr;
};

class SegmentPrimitive : public Object {
  public:
  math::Vector3Df GetCurrentEnd() const {
    float           t   = std::clamp(Time, 0.0f, 1.0f);
    math::Vector3Df end = (End - Start);
    return end * t + Start;
  }

  math::Vector3Df GetDirectionNormalized() const {
    auto result = (End - Start).normalize();
    return result;
  }

  void UpdateSegment();
  void UpdateSegment(const math::Vector3Df& start,
                     const math::Vector3Df& end,
                     const math::Vector4Df& color,
                     float                  time = 1.0f);

  math::Vector3Df Start;
  math::Vector3Df End;
  math::Vector4Df Color = math::Vector4Df(0.0f, 0.0f, 0.0f, 1.0f);
  float           Time  = 0.0f;

  virtual void Update(float deltaTime) override;
};

class ArrowSegmentPrimitive : public Object {
  public:
  virtual ~ArrowSegmentPrimitive() {
    delete SegmentObject;
    delete ConeObject;
  }

  virtual void Update(float deltaTime) override;

  void SetPos(const math::Vector3Df& pos);
  void SetStart(const math::Vector3Df& start);
  void SetEnd(const math::Vector3Df& end);
  void SetTime(float time);

  SegmentPrimitive* SegmentObject = nullptr;
  ConePrimitive*    ConeObject    = nullptr;
};

// class DirectionalLightPrimitive : public Object {
//   public:
//   virtual ~DirectionalLightPrimitive() {
//     delete BillboardObject;
//     delete ArrowSegementObject;
//   }
//
//   virtual void Update(float deltaTime) override;
//
//   BillboardQuadPrimitive* BillboardObject     = nullptr;
//   ArrowSegmentPrimitive*  ArrowSegementObject = nullptr;
//   math::Vector3Df          Pos                 = Vector(0.0f);
//   DirectionalLight*       Light               = nullptr;
// };
//
// class PointLightPrimitive : public Object {
//   public:
//   virtual ~PointLightPrimitive() {
//     delete BillboardObject;
//     delete SphereObject;
//   }
//
//   virtual void Update(float deltaTime) override;
//
//   BillboardQuadPrimitive* BillboardObject = nullptr;
//   Object*                  SphereObject    = nullptr;
//   PointLight*             Light           = nullptr;
// };
//
// class SpotLightPrimitive : public Object {
//   public:
//   virtual ~SpotLightPrimitive() {
//     delete BillboardObject;
//     delete UmbraConeObject;
//     delete PenumbraConeObject;
//   }
//
//   virtual void Update(float deltaTime) override;
//
//   BillboardQuadPrimitive* BillboardObject    = nullptr;
//   ConePrimitive*          UmbraConeObject    = nullptr;
//   ConePrimitive*          PenumbraConeObject = nullptr;
//   SpotLight*              Light              = nullptr;
// };

class FrustumPrimitive : public Object {
  public:
  FrustumPrimitive() = default;

  FrustumPrimitive(const Camera* targetCamera)
      : TargetCamera(targetCamera) {}

  virtual ~FrustumPrimitive() {
    for (int32_t i = 0; i < std::size(Segments); ++i) {
      delete Segments[i];
    }

    for (int32_t i = 0; i < std::size(Plane); ++i) {
      delete Plane[i];
    }
  }

  virtual void Update(float deltaTime) override;
  // virtual void Draw(const std::shared_ptr<jRenderFrameContext>&
  // InRenderFrameContext, const Camera* camera, const Shader* shader, const
  // std::list<const Light*>& lights, int32_t instanceCount = 0) const
  // override;

  SegmentPrimitive* Segments[16]    = {};
  QuadPrimitive*    Plane[6]        = {};
  const Camera*     TargetCamera    = nullptr;
  bool              PostPerspective = false;
  bool              DrawPlane       = false;
  math::Vector3Df   Offset;
  math::Vector3Df   Scale = math::g_oneVector<float, 3>();
};

class Graph2D : public Object {
  public:
  virtual void Update(float deltaTime) override;

  void SethPos(const math::Vector2Df& pos);
  void SetPoints(const std::vector<math::Vector2Df>& points);

  void SetGuardLineSize(const math::Vector2Df& size) { GuardLineSize = size; }

  void UpdateBuffer();

  int32_t GetMaxInstancCount() const {
    return static_cast<int32_t>(ResultMatrices.size());
  }

  bool                         DirtyFlag = false;
  math::Vector2Df              Pos;
  math::Vector2Df              GuardLineSize = math::Vector2Df(100.0f, 100.0f);
  std::vector<math::Vector2Df> Points;
  std::vector<math::Vector2Df> ResultPoints;
  std::vector<math::Matrix4f>  ResultMatrices;
};

std::vector<float> GenerateColor(const math::Vector4Df& color,
                                 int32_t                elementCount);
BoundBox           GenerateBoundBox(const std::vector<float>& vertices);
BoundSphere        GenerateBoundSphere(const std::vector<float>& vertices);
void               CreateBoundObjects(const std::vector<float>& vertices,
                                      Object*                   ownerObject);
BoundBoxObject*    CreateBoundBox(BoundBox               boundBox,
                                  Object*                ownerObject,
                                  const math::Vector4Df& color
                                  = math::Vector4Df(0.0f, 0.0f, 0.0f, 1.0f));
BoundSphereObject* CreateBoundSphere(BoundSphere            boundSphere,
                                     Object*                ownerObject,
                                     const math::Vector4Df& color
                                     = math::Vector4Df(0.0f, 0.0f, 0.0f, 1.0f));

QuadPrimitive*           CreateQuad(const math::Vector3Df& pos,
                                    const math::Vector3Df& size,
                                    const math::Vector3Df& scale,
                                    const math::Vector4Df& color);
Object*                  CreateGizmo(const math::Vector3Df& pos,
                                     const math::Vector3Df& rot,
                                     const math::Vector3Df& scale);
Object*                  CreateTriangle(const math::Vector3Df& pos,
                                        const math::Vector3Df& size,
                                        const math::Vector3Df& scale,
                                        const math::Vector4Df& color);
Object*                  CreateCube(const math::Vector3Df& pos,
                                    const math::Vector3Df& size,
                                    const math::Vector3Df& scale,
                                    const math::Vector4Df& color);
Object*                  CreateCapsule(const math::Vector3Df& pos,
                                       float                  height,
                                       float                  radius,
                                       int32_t                slice,
                                       const math::Vector3Df& scale,
                                       const math::Vector4Df& color);
ConePrimitive*           CreateCone(const math::Vector3Df& pos,
                                    float                  height,
                                    float                  radius,
                                    int32_t                slice,
                                    const math::Vector3Df& scale,
                                    const math::Vector4Df& color,
                                    bool                   isWireframe = false,
                                    bool                   createBoundInfo = true);
Object*                  CreateCylinder(const math::Vector3Df& pos,
                                        float                  height,
                                        float                  radius,
                                        int32_t                slice,
                                        const math::Vector3Df& scale,
                                        const math::Vector4Df& color);
Object*                  CreateSphere(const math::Vector3Df& pos,
                                      float                  radius,
                                      uint32_t               slices,
                                      uint32_t               stacks,
                                      const math::Vector3Df& scale,
                                      const math::Vector4Df& color,
                                      bool                   isWireframe = false,
                                      bool                   createBoundInfo = true);
BillboardQuadPrimitive*  CreateBillobardQuad(const math::Vector3Df& pos,
                                             const math::Vector3Df& size,
                                             const math::Vector3Df& scale,
                                             const math::Vector4Df& color,
                                             Camera*                camera);
UIQuadPrimitive*         CreateUIQuad(const math::Vector2Df& pos,
                                      const math::Vector2Df& size,
                                      TextureVk*             texture);
FullscreenQuadPrimitive* CreateFullscreenQuad(TextureVk* texture);
SegmentPrimitive*        CreateSegment(const math::Vector3Df& start,
                                       const math::Vector3Df& end,
                                       float                  time,
                                       const math::Vector4Df& color);
ArrowSegmentPrimitive*   CreateArrowSegment(const math::Vector3Df& start,
                                            const math::Vector3Df& end,
                                            float                  time,
                                            float                  coneHeight,
                                            float                  coneRadius,
                                            const math::Vector4Df& color);

//////////////////////////////////////////////////////////////////////////
// DirectionalLightPrimitive* CreateDirectionalLightDebug(
//     const math::Vector3Df& pos,
//     const math::Vector3Df& scale,
//     float                  length,
//     Camera*               targetCamera,
//     DirectionalLight*     light,
//     const char*            textureFilename);
// PointLightPrimitive* CreatePointLightDebug(const math::Vector3Df& scale,
//                                             Camera* targetCamera,
//                                             PointLight*           light,
//                                             const char* textureFilename);
// SpotLightPrimitive*  CreateSpotLightDebug(const math::Vector3Df& scale,
//                                            Camera* targetCamera, SpotLight*
//                                            light, const char*
//                                            textureFilename);
//
//////////////////////////////////////////////////////////////////////////
FrustumPrimitive* CreateFrustumDebug(const Camera* targetCamera);
Graph2D*          CreateGraph2D(const math::Vector2Df&              pos,
                                const math::Vector2Df&              size,
                                const std::vector<math::Vector2Df>& points);

}  // namespace game_engine

#endif  // GAME_ENGINE_PRIMITIVE_UTIL_H