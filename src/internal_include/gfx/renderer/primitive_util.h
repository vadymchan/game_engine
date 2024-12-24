#ifndef GAME_ENGINE_PRIMITIVE_UTIL_H
#define GAME_ENGINE_PRIMITIVE_UTIL_H

#include "gfx/scene/object.h"
// TODO: consider removing this include
#include "gfx/rhi/buffer.h"
#include "gfx/rhi/pipeline_state_info.h"
#include "gfx/rhi/shader.h"
#include "gfx/scene/camera_old.h"
#include "utils/math/plane.h"

#include <math_library/vector.h>

#include <algorithm>

namespace game_engine {
class QuadPrimitive : public Object {
  public:
  // ======= BEGIN: public setters ============================================

  void setPlane(const math::Plane& plane);

  // ======= END: public setters   ============================================

  // ======= BEGIN: public misc fields ========================================

  math::Plane m_plane_;

  // ======= END: public misc fields   ========================================
};

class ConePrimitive : public Object {
  public:
  // ======= BEGIN: public misc fields ========================================

  float           m_height_ = 0.0f;
  float           m_radius_ = 0.0f;
  math::Vector4Df m_color_;

  // ======= END: public misc fields   ========================================
};

class CylinderPrimitive : public Object {
  public:
  // ======= BEGIN: public misc fields ========================================

  float           m_height_ = 0.0f;
  float           m_radius_ = 0.0f;
  math::Vector4Df m_color_;

  // ======= END: public misc fields   ========================================
};

class BillboardQuadPrimitive : public QuadPrimitive {
  public:
  // ======= BEGIN: public overridden methods =================================

  virtual void update(float deltaTime) override;

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  std::shared_ptr<CameraOld> m_camera_ = nullptr;

  // ======= END: public misc fields   ========================================
};

class UIQuadPrimitive : public Object {
  public:
  // ======= BEGIN: public overridden methods =================================

  // virtual void draw(const std::shared_ptr<RenderFrameContext>&
  // renderFrameContext, const std::shared_ptr<CameraOld> camera, const Shader*
  // shader, const std::list<const Light*>& lights, int32_t instanceCount = 0)
  // const override;

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public getters ============================================

  const Texture* getTexture() const;

  // ======= END: public getters   ============================================

  // ======= BEGIN: public setters ============================================

  void setTexture(const Texture* texture);
  void setUniformParams(const Shader* shader) const;

  // ======= END: public setters   ============================================

  // ======= BEGIN: public misc fields ========================================

  math::Vector2Df m_position_;
  math::Vector2Df m_size_;

  // ======= END: public misc fields   ========================================
};

class FullscreenQuadPrimitive : public Object {
  public:
  // ======= BEGIN: public overridden methods =================================

  // virtual void draw(const std::shared_ptr<RenderFrameContext>&
  // renderFrameContext, const std::shared_ptr<CameraOld> camera, const Shader*
  // shader, const std::list<const Light*>& lights, int32_t instanceCount = 0)
  // const override;

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public setters ============================================

  void setUniformBuffer(const Shader* shader) const;
  void setTexture(int                     index,
                  const Texture*          texture,
                  const SamplerStateInfo* samplerState);

  // ======= END: public setters   ============================================
};

class BoundBoxObject : public Object {
  public:
  // ======= BEGIN: public setters ============================================

  void setUniformBuffer(const Shader* shader);

  // ======= END: public setters   ============================================

  // ======= BEGIN: public misc methods =======================================

  void updateBoundBox(const BoundBox& boundBox);
  void updateBoundBox();

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  math::Vector4Df m_color_ = math::Vector4Df(0.0f, 0.0f, 0.0f, 1.0f);
  BoundBox        m_boundBox_;
  Object*         m_ownerObject_ = nullptr;

  // ======= END: public misc fields   ========================================
};

class BoundSphereObject : public Object {
  public:
  // ======= BEGIN: public setters ============================================

  void setUniformBuffer(const Shader* shader);

  // ======= END: public setters   ============================================

  // ======= BEGIN: public misc fields ========================================

  math::Vector4Df m_color_ = math::Vector4Df(0.0f, 0.0f, 0.0f, 1.0f);
  BoundSphere     m_boundSphere_;
  Object*         m_ownerObject_ = nullptr;

  // ======= END: public misc fields   ========================================
};

class SegmentPrimitive : public Object {
  public:
  // ======= BEGIN: public overridden methods =================================

  virtual void update(float deltaTime) override;

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public getters ============================================

  math::Vector3Df getCurrentEnd() const {
    float           t   = std::clamp(m_time_, 0.0f, 1.0f);
    math::Vector3Df end = (m_end_ - m_start_);
    return end * t + m_start_;
  }

  math::Vector3Df getDirectionNormalized() const {
    auto result = (m_end_ - m_start_).normalized();
    return result;
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  void updateSegment();
  void updateSegment(const math::Vector3Df& start,
                     const math::Vector3Df& end,
                     const math::Vector4Df& color,
                     float                  time = 1.0f);

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  math::Vector3Df m_start_;
  math::Vector3Df m_end_;
  math::Vector4Df m_color_ = math::Vector4Df(0.0f, 0.0f, 0.0f, 1.0f);
  float           m_time_  = 0.0f;

  // ======= END: public misc fields   ========================================
};

class ArrowSegmentPrimitive : public Object {
  public:
  // ======= BEGIN: public destructor =========================================

  virtual ~ArrowSegmentPrimitive() {
    delete m_segmentObject_;
    delete m_coneObject_;
  }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void update(float deltaTime) override;

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public setters ============================================

  void setPosition(const math::Vector3Df& pos);
  void setStart(const math::Vector3Df& start);
  void setEnd(const math::Vector3Df& end);
  void setTime(float time);

  // ======= END: public setters   ============================================

  // ======= BEGIN: public misc fields ========================================

  SegmentPrimitive* m_segmentObject_ = nullptr;
  ConePrimitive*    m_coneObject_    = nullptr;

  // ======= END: public misc fields   ========================================
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
  // ======= BEGIN: public constructors =======================================

  FrustumPrimitive() = default;

  FrustumPrimitive(const std::shared_ptr<CameraOld> targetCamera)
      : m_targetCamera_(targetCamera) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~FrustumPrimitive() {
    for (int32_t i = 0; i < std::size(m_segments_); ++i) {
      delete m_segments_[i];
    }

    for (int32_t i = 0; i < std::size(m_plane_); ++i) {
      delete m_plane_[i];
    }
  }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void update(float deltaTime) override;
  // virtual void draw(const std::shared_ptr<RenderFrameContext>&
  // renderFrameContext, const std::shared_ptr<CameraOld> camera, const Shader*
  // shader, const std::list<const Light*>& lights, int32_t instanceCount = 0)
  // const override;

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  SegmentPrimitive*             m_segments_[16]    = {};
  QuadPrimitive*                m_plane_[6]        = {};
  const std::shared_ptr<CameraOld> m_targetCamera_    = nullptr;
  bool                          m_postPerspective_ = false;
  bool                          m_drawPlane_       = false;
  math::Vector3Df               m_offset_;
  math::Vector3Df               m_scale_ = math::g_oneVector<float, 3>();

  // ======= END: public misc fields   ========================================
};

class Graph2D : public Object {
  public:
  // ======= BEGIN: public overridden methods =================================

  virtual void update(float deltaTime) override;

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public getters ============================================

  int32_t getMaxInstancCount() const {
    return static_cast<int32_t>(m_resultMatrices_.size());
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public setters ============================================

  void sethPosition(const math::Vector2Df& pos);
  void setPoints(const std::vector<math::Vector2Df>& points);

  void setGuardLineSize(const math::Vector2Df& size) {
    m_guardLineSize_ = size;
  }

  // ======= END: public setters   ============================================

  // ======= BEGIN: public misc methods =======================================

  void updateBuffer();

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  bool            m_dirtyFlag_ = false;
  math::Vector2Df m_position_;
  math::Vector2Df m_guardLineSize_ = math::Vector2Df(100.0f, 100.0f);
  std::vector<math::Vector2Df>  m_points_;
  std::vector<math::Vector2Df>  m_resultPoints_;
  std::vector<math::Matrix4f<>> m_resultMatrices_;

  // ======= END: public misc fields   ========================================
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

BillboardQuadPrimitive* g_createBillobardQuad(const math::Vector3Df&  pos,
                                              const math::Vector3Df&  size,
                                              const math::Vector3Df&  scale,
                                              const math::Vector4Df&  color,
                                              std::shared_ptr<CameraOld> camera);

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
//     std::shared_ptr<CameraOld>               targetCamera,
//     DirectionalLight*     light,
//     const char*            textureFilename);
//
// PointLightPrimitive* g_createPointLightDebug(const math::Vector3Df& scale,
//                                             std::shared_ptr<CameraOld>
//                                             targetCamera, PointLight* light,
//                                             const char* textureFilename);
//
// SpotLightPrimitive*  g_createSpotLightDebug(const math::Vector3Df& scale,
//                                            std::shared_ptr<CameraOld>
//                                            targetCamera, SpotLight* light,
//                                            const char* textureFilename);
//
//////////////////////////////////////////////////////////////////////////
FrustumPrimitive* g_createFrustumDebug(
    const std::shared_ptr<CameraOld> targetCamera);

Graph2D* g_createGraph2D(const math::Vector2Df&              pos,
                         const math::Vector2Df&              size,
                         const std::vector<math::Vector2Df>& points);

}  // namespace game_engine

#endif  // GAME_ENGINE_PRIMITIVE_UTIL_H
