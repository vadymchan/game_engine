#ifndef GAME_ENGINE_PRIMITIVE_UTIL_H
#define GAME_ENGINE_PRIMITIVE_UTIL_H

#include "gfx/scene/object.h"
// TODO: consider removing this include
#include "gfx/rhi/buffer.h"
#include "gfx/rhi/pipeline_state_info.h"
#include "gfx/rhi/shader.h"
#include "gfx/scene/camera.h"
#include "utils/math/plane.h"

#include <math_library/vector.h>

#include <algorithm>

namespace game_engine {
class QuadPrimitive : public Object {
  public:
  void setPlane(const math::Plane& plane);

  math::Plane m_plane_;
};

class ConePrimitive : public Object {
  public:
  float           m_height_ = 0.0f;
  float           m_radius_ = 0.0f;
  math::Vector4Df m_color_;
};

class CylinderPrimitive : public Object {
  public:
  float           m_height_ = 0.0f;
  float           m_radius_ = 0.0f;
  math::Vector4Df m_color_;
};

class BillboardQuadPrimitive : public QuadPrimitive {
  public:
  Camera* m_camera_ = nullptr;

  virtual void update(float deltaTime) override;
};

class UIQuadPrimitive : public Object {
  public:
  math::Vector2Df m_position_;
  math::Vector2Df m_size_;

  // virtual void draw(const std::shared_ptr<RenderFrameContext>&
  // renderFrameContext, const Camera* camera, const Shader* shader, const
  // std::list<const Light*>& lights, int32_t instanceCount = 0) const
  // override;
  void           setTexture(const Texture* texture);
  void           setUniformParams(const Shader* shader) const;
  const Texture* getTexture() const;
};

class FullscreenQuadPrimitive : public Object {
  public:
  // virtual void draw(const std::shared_ptr<RenderFrameContext>&
  // renderFrameContext, const Camera* camera, const Shader* shader, const
  // std::list<const Light*>& lights, int32_t instanceCount = 0) const
  // override;
  void setUniformBuffer(const Shader* shader) const;
  void setTexture(int                     index,
                  const Texture*          texture,
                  const SamplerStateInfo* samplerState);
};

class BoundBoxObject : public Object {
  public:
  void setUniformBuffer(const Shader* shader);
  void updateBoundBox(const BoundBox& boundBox);
  void updateBoundBox();

  math::Vector4Df m_color_ = math::Vector4Df(0.0f, 0.0f, 0.0f, 1.0f);
  BoundBox        m_boundBox_;
  Object*         m_ownerObject_ = nullptr;
};

class BoundSphereObject : public Object {
  public:
  void setUniformBuffer(const Shader* shader);

  math::Vector4Df m_color_ = math::Vector4Df(0.0f, 0.0f, 0.0f, 1.0f);
  BoundSphere     m_boundSphere_;
  Object*         m_ownerObject_ = nullptr;
};

class SegmentPrimitive : public Object {
  public:
  math::Vector3Df getCurrentEnd() const {
    float           t   = std::clamp(m_time_, 0.0f, 1.0f);
    math::Vector3Df end = (m_end_ - m_start_);
    return end * t + m_start_;
  }

  math::Vector3Df getDirectionNormalized() const {
    auto result = (m_end_ - m_start_).normalized();
    return result;
  }

  void updateSegment();
  void updateSegment(const math::Vector3Df& start,
                     const math::Vector3Df& end,
                     const math::Vector4Df& color,
                     float                  time = 1.0f);

  math::Vector3Df m_start_;
  math::Vector3Df m_end_;
  math::Vector4Df m_color_ = math::Vector4Df(0.0f, 0.0f, 0.0f, 1.0f);
  float           m_time_  = 0.0f;

  virtual void update(float deltaTime) override;
};

class ArrowSegmentPrimitive : public Object {
  public:
  virtual ~ArrowSegmentPrimitive() {
    delete m_segmentObject_;
    delete m_coneObject_;
  }

  virtual void update(float deltaTime) override;

  void setPosition(const math::Vector3Df& pos);
  void setStart(const math::Vector3Df& start);
  void setEnd(const math::Vector3Df& end);
  void setTime(float time);

  SegmentPrimitive* m_segmentObject_ = nullptr;
  ConePrimitive*    m_coneObject_    = nullptr;
};

// class DirectionalLightPrimitive : public Object {
//   public:
//   virtual ~DirectionalLightPrimitive() {
//     delete m_billboardObject_;
//     delete m_arrowSegementObject_;
//   }
//
//   virtual void update(float deltaTime) override;
//
//   BillboardQuadPrimitive* m_billboardObject_     = nullptr;
//   ArrowSegmentPrimitive*  m_arrowSegementObject_ = nullptr;
//   math::Vector3Df         m_position_            = Vector(0.0f);
//   DirectionalLight*       m_light_               = nullptr;
// };
//
// class PointLightPrimitive : public Object {
//   public:
//   virtual ~PointLightPrimitive() {
//     delete m_billboardObject_;
//     delete m_sphereObject_;
//   }
//
//   virtual void update(float deltaTime) override;
//
//   BillboardQuadPrimitive* m_billboardObject_ = nullptr;
//   Object*                 m_sphereObject_    = nullptr;
//   PointLight*             m_light_           = nullptr;
// };
//
// class SpotLightPrimitive : public Object {
//   public:
//   virtual ~SpotLightPrimitive() {
//     delete m_billboardObject_;
//     delete m_umbraConeObject_;
//     delete m_penumbraConeObject_;
//   }
//
//   virtual void update(float deltaTime) override;
//
//   BillboardQuadPrimitive* m_billboardObject_    = nullptr;
//   ConePrimitive*          m_umbraConeObject_    = nullptr;
//   ConePrimitive*          m_penumbraConeObject_ = nullptr;
//   SpotLight*              m_light_              = nullptr;
// };

class FrustumPrimitive : public Object {
  public:
  FrustumPrimitive() = default;

  FrustumPrimitive(const Camera* targetCamera)
      : m_targetCamera_(targetCamera) {}

  virtual ~FrustumPrimitive() {
    for (int32_t i = 0; i < std::size(m_segments_); ++i) {
      delete m_segments_[i];
    }

    for (int32_t i = 0; i < std::size(m_plane_); ++i) {
      delete m_plane_[i];
    }
  }

  virtual void update(float deltaTime) override;
  // virtual void draw(const std::shared_ptr<RenderFrameContext>&
  // renderFrameContext, const Camera* camera, const Shader* shader, const
  // std::list<const Light*>& lights, int32_t instanceCount = 0) const
  // override;

  SegmentPrimitive* m_segments_[16]    = {};
  QuadPrimitive*    m_plane_[6]        = {};
  const Camera*     m_targetCamera_    = nullptr;
  bool              m_postPerspective_ = false;
  bool              m_drawPlane_       = false;
  math::Vector3Df   m_offset_;
  math::Vector3Df   m_scale_ = math::g_oneVector<float, 3>();
};

class Graph2D : public Object {
  public:
  virtual void update(float deltaTime) override;

  void sethPosition(const math::Vector2Df& pos);
  void setPoints(const std::vector<math::Vector2Df>& points);

  void setGuardLineSize(const math::Vector2Df& size) {
    m_guardLineSize_ = size;
  }

  void updateBuffer();

  int32_t getMaxInstancCount() const {
    return static_cast<int32_t>(m_resultMatrices_.size());
  }

  bool            m_dirtyFlag_ = false;
  math::Vector2Df m_position_;
  math::Vector2Df m_guardLineSize_ = math::Vector2Df(100.0f, 100.0f);
  std::vector<math::Vector2Df> m_points_;
  std::vector<math::Vector2Df> m_resultPoints_;
  std::vector<math::Matrix4f>  m_resultMatrices_;
};

// TODO: consider moving this global functions
std::vector<float> g_generateColor(const math::Vector4Df& color,
                                   int32_t                elementCount);

BoundBox g_generateBoundBox(const std::vector<float>& vertices);

BoundSphere g_generateBoundSphere(const std::vector<float>& vertices);

void g_createBoundObjects(const std::vector<float>& vertices,
                          Object*                   ownerObject);

BoundBoxObject* g_createBoundBox(BoundBox               boundBox,
                                 Object*                ownerObject,
                                 const math::Vector4Df& color
                                 = math::Vector4Df(0.0f, 0.0f, 0.0f, 1.0f));

BoundSphereObject* g_createBoundSphere(
    BoundSphere            boundSphere,
    Object*                ownerObject,
    const math::Vector4Df& color = math::Vector4Df(0.0f, 0.0f, 0.0f, 1.0f));

QuadPrimitive* g_createQuad(const math::Vector3Df& pos,
                            const math::Vector3Df& size,
                            const math::Vector3Df& scale,
                            const math::Vector4Df& color);

Object* g_createGizmo(const math::Vector3Df& pos,
                      const math::Vector3Df& rot,
                      const math::Vector3Df& scale);

Object* g_createTriangle(const math::Vector3Df& pos,
                         const math::Vector3Df& size,
                         const math::Vector3Df& scale,
                         const math::Vector4Df& color);

Object* g_createCube(const math::Vector3Df& pos,
                     const math::Vector3Df& size,
                     const math::Vector3Df& scale,
                     const math::Vector4Df& color);

Object* g_createCapsule(const math::Vector3Df& pos,
                        float                  height,
                        float                  radius,
                        int32_t                slice,
                        const math::Vector3Df& scale,
                        const math::Vector4Df& color);

ConePrimitive* g_createCone(const math::Vector3Df& pos,
                            float                  height,
                            float                  radius,
                            int32_t                slice,
                            const math::Vector3Df& scale,
                            const math::Vector4Df& color,
                            bool                   isWireframe     = false,
                            bool                   createBoundInfo = true);

Object* g_createCylinder(const math::Vector3Df& pos,
                         float                  height,
                         float                  radius,
                         int32_t                slice,
                         const math::Vector3Df& scale,
                         const math::Vector4Df& color);

Object* g_createSphere(const math::Vector3Df& pos,
                       float                  radius,
                       uint32_t               slices,
                       uint32_t               stacks,
                       const math::Vector3Df& scale,
                       const math::Vector4Df& color,
                       bool                   isWireframe     = false,
                       bool                   createBoundInfo = true);

BillboardQuadPrimitive* g_createBillobardQuad(const math::Vector3Df& pos,
                                              const math::Vector3Df& size,
                                              const math::Vector3Df& scale,
                                              const math::Vector4Df& color,
                                              Camera*                camera);

UIQuadPrimitive* g_createUIQuad(const math::Vector2Df& pos,
                                const math::Vector2Df& size,
                                Texture*               texture);

FullscreenQuadPrimitive* g_createFullscreenQuad(Texture* texture);

SegmentPrimitive* g_createSegment(const math::Vector3Df& start,
                                  const math::Vector3Df& end,
                                  float                  time,
                                  const math::Vector4Df& color);

ArrowSegmentPrimitive* g_createArrowSegment(const math::Vector3Df& start,
                                            const math::Vector3Df& end,
                                            float                  time,
                                            float                  coneHeight,
                                            float                  coneRadius,
                                            const math::Vector4Df& color);

//////////////////////////////////////////////////////////////////////////
// DirectionalLightPrimitive* g_createDirectionalLightDebug(
//     const math::Vector3Df& pos,
//     const math::Vector3Df& scale,
//     float                  length,
//     Camera*               targetCamera,
//     DirectionalLight*     light,
//     const char*            textureFilename);
// 
// PointLightPrimitive* g_createPointLightDebug(const math::Vector3Df& scale,
//                                             Camera* targetCamera,
//                                             PointLight*           light,
//                                             const char* textureFilename);
// 
// SpotLightPrimitive*  g_createSpotLightDebug(const math::Vector3Df& scale,
//                                            Camera* targetCamera, SpotLight*
//                                            light, const char*
//                                            textureFilename);
//
//////////////////////////////////////////////////////////////////////////
FrustumPrimitive* g_createFrustumDebug(const Camera* targetCamera);

Graph2D* g_createGraph2D(const math::Vector2Df&              pos,
                         const math::Vector2Df&              size,
                         const std::vector<math::Vector2Df>& points);

}  // namespace game_engine

#endif  // GAME_ENGINE_PRIMITIVE_UTIL_H