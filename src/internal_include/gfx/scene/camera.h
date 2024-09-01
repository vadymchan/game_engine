// TODO:
// - move to more appropriate folder (e.g. scene)
// - consider moving scene folder to a more appropriate location (gfx is not the
// very best place for it)

#ifndef GAME_ENGINE_CAMERA_H
#define GAME_ENGINE_CAMERA_H

#include "gfx/rhi/feature_switch.h"
#include "utils/math/math_util.h"
#include "utils/math/plane.h"

#include <math_library/graphics.h>
#include <math_library/matrix.h>
#include <math_library/vector.h>

#include <array>
#include <map>

namespace game_engine {

struct FrustumPlane {
  // ======= BEGIN: public misc methods =======================================

  bool isInFrustum(const math::Vector3Df& pos, float radius) const {
    for (const auto& plane : m_planes_) {
      const float r = pos.dot(plane.m_n_) - plane.m_d_ + radius;
      if (r < 0.0f) {
        return false;
      }
    }
    return true;
  }

  bool isInFrustumWithDirection(const math::Vector3Df& pos,
                                const math::Vector3Df& direction,
                                float                  radius) const {
    for (const auto& plane : m_planes_) {
      const float r = pos.dot(plane.m_n_) - plane.m_d_ + radius;
      if (r < 0.0f) {
        if (direction.dot(plane.m_n_) <= 0.0) {
          return false;
        }
      }
    }
    return true;
  }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  std::array<math::Plane, 6> m_planes_;

  // ======= END: public misc fields   ========================================
};

class Camera {
  public:
  // ======= BEGIN: public nested types =======================================

  enum ECameraType {
    NORMAL = 0,
    ORTHO,
    MAX
  };

  // ======= END: public nested types   =======================================

    // ======= BEGIN: public static methods =====================================

  static void s_addCamera(int32_t id, Camera* camera) {
    s_cameraMap.insert(std::make_pair(id, camera));
  }

  static Camera* s_getCamera(int32_t id) {
    // TODO: consider better name
    auto itFind = s_cameraMap.find(id);
    return (s_cameraMap.end() != itFind) ? itFind->second : nullptr;
  }

  static void s_removeCamera(int32_t id) { s_cameraMap.erase(id); }

  static Camera* s_getMainCamera() { return s_getCamera(0); }

  static Camera* s_createCamera(const math::Vector3Df& pos,
                                const math::Vector3Df& target,
                                const math::Vector3Df& up,
                                float                  fovRad,
                                float                  nearDist,
                                float                  farDist,
                                float                  width,
                                float                  height,
                                bool isPerspectiveProjection) {
    Camera* camera = new Camera();
    s_setCamera(camera,
                pos,
                target,
                up,
                fovRad,
                nearDist,
                farDist,
                width,
                height,
                isPerspectiveProjection);
    return camera;
  }

  static void s_getForwardRightUpFromEulerAngle(
      math::Vector3Df&       forward,
      math::Vector3Df&       right,
      math::Vector3Df&       up,
      const math::Vector3Df& eulerAngle) {
    forward = math::g_getDirectionFromEulerAngle(eulerAngle).normalized();

    const bool kIsInvert = (eulerAngle.x() < 0 || math::g_kPi < eulerAngle.x());

    const math::Vector3Df kUpVector
        = (kIsInvert ? math::g_downVector<float, 3>()
                     : math::g_upVector<float, 3>());
    right = (kIsInvert ? math::g_downVector<float, 3>()
                       : math::g_upVector<float, 3>())
                .cross(forward)
                .normalized();
    if (math::g_isNearlyZero(right.magnitudeSquared())) {
      right = (kIsInvert ? math::g_backwardVector<float, 3>()
                         : math::g_forwardVector<float, 3>())
                  .cross(forward)
                  .normalized();
    }
    up = forward.cross(right).normalized();
  }

  static void s_setCamera(Camera*                camera,
                          const math::Vector3Df& pos,
                          const math::Vector3Df& target,
                          const math::Vector3Df& up,
                          float                  fovRad,
                          float                  nearDist,
                          float                  farDist,
                          float                  width,
                          float                  height,
                          bool                   isPerspectiveProjection,
                          float                  distance = 300.0f) {
    const auto kToTarget = (target - pos);
    camera->m_position_  = pos;
    camera->m_target_    = target;
    camera->m_up_        = up;
    camera->m_distance_  = distance;
    camera->setEulerAngle(math::g_getEulerAngleFrom(kToTarget));

    camera->m_FOVRad_                  = fovRad;
    camera->m_Near_                    = nearDist;
    camera->m_far_                     = farDist;
    camera->m_width_                   = static_cast<int32_t>(width);
    camera->m_height_                  = static_cast<int32_t>(height);
    camera->m_isPerspectiveProjection_ = isPerspectiveProjection;
  }

  // ======= END: public static methods   =====================================

  // ======= BEGIN: public static fields ======================================

  static std::map<int32_t, Camera*> s_cameraMap;

  // ======= END: public static fields   ======================================


  // ======= BEGIN: public constructors =======================================

  Camera()
      : m_type_(ECameraType::NORMAL) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~Camera() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public getters ============================================

  // TODO: remove Vector postfix
  math::Vector3Df getForwardVector() const {
    return m_view_.getColumn<2>().resizedCopy<3>();
  }

  math::Vector3Df getUpVector() const {
    return m_view_.getColumn<1>().resizedCopy<3>();
  }

  math::Vector3Df getRightVector() const {
    return m_view_.getColumn<0>().resizedCopy<3>();
  }

  math::Matrix4f getViewProjectionMatrix() const { return m_viewProjection_; }

  math::Matrix4f getInverseViewProjectionMatrix() const {
    return m_invViewProjection_;
  }

  // TODO: seems not used
  math::Vector3Df getEulerAngle() const { return m_eulerAngle_; }

  void getRectInNDCSpace(math::Vector3Df&      minPosition,
                         math::Vector3Df&      maxPosition,
                         const math::Matrix4f& vp) const {
    // TODO: consider more descriptive name
    math::Vector3Df farLt;
    math::Vector3Df farRt;
    math::Vector3Df farLb;
    math::Vector3Df farRb;

    math::Vector3Df nearLt;
    math::Vector3Df nearRt;
    math::Vector3Df nearLb;
    math::Vector3Df nearRb;

    // TODO: consider better naming convention
    const auto  kOrigin = m_position_;
    const float kNear   = m_Near_;
    const float kFar    = m_far_;

    if (m_isPerspectiveProjection_) {
      const float     kInvAspect = ((float)m_width_ / (float)m_height_);
      const float     kLength    = tanf(m_FOVRad_ * 0.5f);
      math::Vector3Df targetVec  = getForwardVector().normalized();
      math::Vector3Df rightVec
          = getRightVector().normalized() * kLength * kInvAspect;
      math::Vector3Df upVec = getUpVector().normalized() * kLength;

      math::Vector3Df rightUp   = (targetVec + rightVec + upVec);
      math::Vector3Df leftUp    = (targetVec - rightVec + upVec);
      math::Vector3Df rightDown = (targetVec + rightVec - upVec);
      math::Vector3Df leftDown  = (targetVec - rightVec - upVec);

      farLt = kOrigin + leftUp * kFar;
      farRt = kOrigin + rightUp * kFar;
      farLb = kOrigin + leftDown * kFar;
      farRb = kOrigin + rightDown * kFar;

      nearLt = kOrigin + leftUp * kNear;
      nearRt = kOrigin + rightUp * kNear;
      nearLb = kOrigin + leftDown * kNear;
      nearRb = kOrigin + rightDown * kNear;
    } else {
      const float kWidth  = (float)m_width_;
      const float kHeight = (float)m_height_;

      math::Vector3Df targetVec = getForwardVector().normalized();
      math::Vector3Df rightVec  = getRightVector().normalized();
      math::Vector3Df upVec     = getUpVector().normalized();

      farLt = kOrigin + targetVec * kFar - rightVec * kWidth * 0.5f
            + upVec * kHeight * 0.5f;
      farRt = kOrigin + targetVec * kFar + rightVec * kWidth * 0.5f
            + upVec * kHeight * 0.5f;
      farLb = kOrigin + targetVec * kFar - rightVec * kWidth * 0.5f
            - upVec * kHeight * 0.5f;
      farRb = kOrigin + targetVec * kFar + rightVec * kWidth * 0.5f
            - upVec * kHeight * 0.5f;

      nearLt = kOrigin + targetVec * kNear - rightVec * kWidth * 0.5f
             + upVec * kHeight * 0.5f;
      nearRt = kOrigin + targetVec * kNear + rightVec * kWidth * 0.5f
             + upVec * kHeight * 0.5f;
      nearLb = kOrigin + targetVec * kNear - rightVec * kWidth * 0.5f
             - upVec * kHeight * 0.5f;
      nearRb = kOrigin + targetVec * kNear + rightVec * kWidth * 0.5f
             - upVec * kHeight * 0.5f;
    }

    // Transform to NDC space
    {
      farLt = math::g_transformPoint(farLt, vp);
      farRt = math::g_transformPoint(farRt, vp);
      farLb = math::g_transformPoint(farLb, vp);
      farRb = math::g_transformPoint(farRb, vp);

      nearLt = math::g_transformPoint(nearLt, vp);
      nearRt = math::g_transformPoint(nearRt, vp);
      nearLb = math::g_transformPoint(nearLb, vp);
      nearRb = math::g_transformPoint(nearRb, vp);
    }

    minPosition = math::Vector3Df(FLT_MAX);
    minPosition = std::min(minPosition, farLt);
    minPosition = std::min(minPosition, farRt);
    minPosition = std::min(minPosition, farLb);
    minPosition = std::min(minPosition, farRb);
    minPosition = std::min(minPosition, nearLt);
    minPosition = std::min(minPosition, nearRt);
    minPosition = std::min(minPosition, nearLb);
    minPosition = std::min(minPosition, nearRb);

    maxPosition = math::Vector3Df(FLT_MIN);
    maxPosition = std::max(maxPosition, farLt);
    maxPosition = std::max(maxPosition, farRt);
    maxPosition = std::max(maxPosition, farLb);
    maxPosition = std::max(maxPosition, farRb);
    maxPosition = std::max(maxPosition, nearLt);
    maxPosition = std::max(maxPosition, nearRt);
    maxPosition = std::max(maxPosition, nearLb);
    maxPosition = std::max(maxPosition, nearRb);
  }

  // TODO: seems not used
  void getRectInScreenSpace(math::Vector3Df&       minPosition,
                            math::Vector3Df&       maxPosition,
                            const math::Matrix4f&  vp,
                            const math::Vector2Df& screenSize
                            = math::Vector2Df(1.0f, 1.0f)) const {
    getRectInNDCSpace(minPosition, maxPosition, vp);

    // Min XY
    minPosition = std::max(minPosition, math::Vector3Df(-1.0f, -1.0f, -1.0f));
    minPosition.x() = (minPosition.x() * 0.5f + 0.5f) * screenSize.x();
    minPosition.y() = (minPosition.y() * 0.5f + 0.5f) * screenSize.y();

    // Max XY
    maxPosition     = std::min(maxPosition, math::Vector3Df(1.0f, 1.0f, 1.0f));
    maxPosition.x() = (maxPosition.x() * 0.5f + 0.5f) * screenSize.x();
    maxPosition.y() = (maxPosition.y() * 0.5f + 0.5f) * screenSize.y();
  }

  // TODO: seems not used
  void getFrustumVertexInWorld(math::Vector3Df* vertexArray) const {
    math::Vector3Df farLt;
    math::Vector3Df farRt;
    math::Vector3Df farLb;
    math::Vector3Df farRb;

    math::Vector3Df nearLt;
    math::Vector3Df nearRt;
    math::Vector3Df nearLb;
    math::Vector3Df nearRb;

    const auto  kOrigin = m_position_;
    const float kNear   = m_Near_;
    const float kFar    = m_far_;

    if (m_isPerspectiveProjection_) {
      const float     kInvAspect = ((float)m_width_ / (float)m_height_);
      const float     kLength    = tanf(m_FOVRad_ * 0.5f);
      math::Vector3Df targetVec  = getForwardVector().normalized();
      math::Vector3Df rightVec
          = getRightVector().normalized() * kLength * kInvAspect;
      math::Vector3Df upVec = getUpVector().normalized() * kLength;

      math::Vector3Df rightUp   = (targetVec + rightVec + upVec);
      math::Vector3Df leftUp    = (targetVec - rightVec + upVec);
      math::Vector3Df rightDown = (targetVec + rightVec - upVec);
      math::Vector3Df leftDown  = (targetVec - rightVec - upVec);

      farLt = kOrigin + leftUp * kFar;
      farRt = kOrigin + rightUp * kFar;
      farLb = kOrigin + leftDown * kFar;
      farRb = kOrigin + rightDown * kFar;

      nearLt = kOrigin + leftUp * kNear;
      nearRt = kOrigin + rightUp * kNear;
      nearLb = kOrigin + leftDown * kNear;
      nearRb = kOrigin + rightDown * kNear;
    } else {
      const float kWidth  = (float)m_width_;
      const float kHeight = (float)m_height_;

      math::Vector3Df targetVec = getForwardVector().normalized();
      math::Vector3Df rightVec  = getRightVector().normalized();
      math::Vector3Df upVec     = getUpVector().normalized();

      farLt = kOrigin + targetVec * kFar - rightVec * kWidth * 0.5f
            + upVec * kHeight * 0.5f;
      farRt = kOrigin + targetVec * kFar + rightVec * kWidth * 0.5f
            + upVec * kHeight * 0.5f;
      farLb = kOrigin + targetVec * kFar - rightVec * kWidth * 0.5f
            - upVec * kHeight * 0.5f;
      farRb = kOrigin + targetVec * kFar + rightVec * kWidth * 0.5f
            - upVec * kHeight * 0.5f;

      nearLt = kOrigin + targetVec * kNear - rightVec * kWidth * 0.5f
             + upVec * kHeight * 0.5f;
      nearRt = kOrigin + targetVec * kNear + rightVec * kWidth * 0.5f
             + upVec * kHeight * 0.5f;
      nearLb = kOrigin + targetVec * kNear - rightVec * kWidth * 0.5f
             - upVec * kHeight * 0.5f;
      nearRb = kOrigin + targetVec * kNear + rightVec * kWidth * 0.5f
             - upVec * kHeight * 0.5f;
    }

    vertexArray[0] = farLt;
    vertexArray[1] = farRt;
    vertexArray[2] = farLb;
    vertexArray[3] = farRb;

    vertexArray[4] = nearLt;
    vertexArray[5] = nearRt;
    vertexArray[6] = nearLb;
    vertexArray[7] = nearRb;
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public setters ============================================

  void setEulerAngle(const math::Vector3Df& eulerAngle) {
    if (m_eulerAngle_ != eulerAngle) {
      m_eulerAngle_ = eulerAngle;
      updateCameraParameters();
    }
  }

  // ======= END: public setters   ============================================

  // ======= BEGIN: public misc methods =======================================

  bool isInFrustum(const math::Vector3Df& pos, float radius) {
    for (auto& iter : m_frustum_.m_planes_) {
      // TODO: consider better naming
      const float kD = pos.dot(iter.m_n_) - iter.m_d_ + radius;
      if (kD < 0.0f) {
        return false;
      }
    }
    return true;
  }

  bool isInFrustumWithDirection(const math::Vector3Df& pos,
                                const math::Vector3Df& dir,
                                float                  radius) const {
    for (auto& iter : m_frustum_.m_planes_) {
      const float kD = pos.dot(iter.m_n_) - iter.m_d_ + radius;
      if (kD < 0.0f) {
        if (dir.dot(iter.m_n_) <= 0) {
          return false;
        }
      }
    }
    return true;
  }

  virtual math::Matrix4f createView() const {
    // TODO: consider adding RH / LH selection logic
    return math::g_lookAtLh(m_position_, m_target_, m_up_);
  }

  virtual math::Matrix4f createProjection() const {
    const auto kRatio
        = static_cast<float>(m_width_) / static_cast<float>(m_height_);
    if (m_isPerspectiveProjection_) {
      if (m_isInfinityFar_) {
        return math::g_perspectiveLhZoInf(m_FOVRad_, kRatio, m_Near_);
      }
      return math::g_perspectiveLhZo(m_FOVRad_, kRatio, m_Near_, m_far_);
    }
    return math::g_orthoLhZo(
        (float)m_width_, (float)m_height_, m_Near_, m_far_);
  }

  void updateCameraFrustum() {
    auto       toTarget = (m_target_ - m_position_).normalized();
    const auto kLength  = tanf(m_FOVRad_) * m_far_;
    auto       toRight  = getRightVector() * kLength;
    auto       toUp     = getUpVector() * kLength;

    auto rightUp   = (toTarget * m_far_ + toRight + toUp).normalized();
    auto leftUp    = (toTarget * m_far_ - toRight + toUp).normalized();
    auto rightDown = (toTarget * m_far_ + toRight - toUp).normalized();
    auto leftDown  = (toTarget * m_far_ - toRight - toUp).normalized();

    auto farLt = m_position_ + leftUp * m_far_;
    auto farRt = m_position_ + rightUp * m_far_;
    auto farLb = m_position_ + leftDown * m_far_;
    auto farRb = m_position_ + rightDown * m_far_;

    auto nearLt = m_position_ + leftUp * m_Near_;
    auto nearRt = m_position_ + rightUp * m_Near_;
    auto nearLb = m_position_ + leftDown * m_Near_;
    auto nearRb = m_position_ + rightDown * m_Near_;

    m_frustum_.m_planes_[0] = math::Plane::s_createFrustumFromThreePoints(
        nearLb, farLb, nearLt);   // left
    m_frustum_.m_planes_[1] = math::Plane::s_createFrustumFromThreePoints(
        nearRt, farRt, nearRb);   // right
    m_frustum_.m_planes_[2] = math::Plane::s_createFrustumFromThreePoints(
        nearLt, farLt, nearRt);   // top
    m_frustum_.m_planes_[3] = math::Plane::s_createFrustumFromThreePoints(
        nearRb, farRb, nearLb);   // bottom
    m_frustum_.m_planes_[4] = math::Plane::s_createFrustumFromThreePoints(
        nearLb, nearLt, nearRb);  // near
    m_frustum_.m_planes_[5] = math::Plane::s_createFrustumFromThreePoints(
        farRb, farRt, farLb);     // far

    // debug object update
  }

  void updateCamera() {
    m_prevViewProjection_ = m_projection_ * m_view_;

    m_view_              = createView();
    m_projection_        = createProjection();
    m_viewProjection_    = m_projection_ * m_view_;
    m_invViewProjection_ = m_viewProjection_.inverse();
    if (m_isPerspectiveProjection_) {
#if USE_REVERSEZ_PERSPECTIVE_SHADOW
        // TODO: add implementation
#else
      m_reverseZProjection_ = Projection;
#endif
    }
  }

  void updateCameraParameters() {
    math::Vector3Df forwardDir;
    math::Vector3Df rightDir;
    math::Vector3Df upDir;
    s_getForwardRightUpFromEulerAngle(
        forwardDir, rightDir, upDir, m_eulerAngle_);
    m_target_ = m_position_ + forwardDir * m_distance_;
    m_up_     = m_position_ + upDir;
  }

  void moveShift(float dist) {
    auto toRight  = getRightVector() * dist;
    m_position_  += toRight;
    m_target_    += toRight;
    m_up_        += toRight;
  }

  void moveForward(float dist) {
    auto toForward  = getForwardVector() * dist;
    m_position_    += toForward;
    m_target_      += toForward;
    m_up_          += toForward;
  }

  void rotateCameraAxis(const math::Vector3Df& axis, float radian) {
    const auto kTransformMatrix = math::g_translate(m_position_)
                                * math::g_rotateLh(axis, radian)
                                * math::g_translate(-m_position_);
    m_position_ = math::g_transformPoint(m_position_, kTransformMatrix);
    m_target_   = math::g_transformPoint(m_target_, kTransformMatrix);
    m_up_       = math::g_transformPoint(m_up_, kTransformMatrix);
  }

  void rotateForwardAxis(float radian) {
    rotateCameraAxis(getForwardVector(), radian);
  }

  void rotateUpAxis(float radian) { rotateCameraAxis(getUpVector(), radian); }

  void rotateRightAxis(float radian) {
    rotateCameraAxis(getRightVector(), radian);
  }

  void rotateXAxis(float radian) {
    rotateCameraAxis(math::Vector3Df(1.0f, 0.0f, 0.0f), radian);
  }

  void rotateYAxis(float radian) {
    rotateCameraAxis(math::Vector3Df(0.0f, 1.0f, 0.0f), radian);
  }

  void rotateZAxis(float radian) {
    rotateCameraAxis(math::Vector3Df(0.0f, 0.0f, 1.0f), radian);
  }

  // TODO: remove (currently not used)
  // virtual void bindCamera(const Shader* shader) const {
  //  auto VP = Projection * view;
  //  auto P  = Projection;
  //  SET_UNIFORM_BUFFER_STATIC("P", P, shader);
  //  SET_UNIFORM_BUFFER_STATIC("VP", VP, shader);
  //  SET_UNIFORM_BUFFER_STATIC("Eye", m_position_, shader);

  //
  //}

  // TODO: currently not used
  // void AddLight(Light* light);
  // Light* GetLight(int32_t index) const;
  // Light* GetLight(ELightType type) const;
  // void RemoveLight(int32_t index);
  // void RemoveLight(ELightType type);
  // void RemoveLight(Light* light);

  // int32_t GetNumOfLight() const;

  // Camera Uniform buffer
  // struct UniformBufferCamera
  //{
  //	math::Matrix4f VP;
  //	math::Matrix4f V;
  //	float Near;
  //	float Far;
  //};
  // UniformBufferCamera Data;
  // IUniformBufferBlock* UniformBufferBlock = nullptr;

  // void UpdateUniformBuffer()
  //{
  //	if (!UniformBufferBlock)
  //	{
  //		UniformBufferBlock = g_rhi->createUniformBufferBlock("Camera",
  // sizeof(UniformBufferCamera));
  //	}

  //	const auto& V = view;
  //	const auto& VP = Projection * view;

  //	UniformBufferCamera ubo = {};
  //	ubo.V = V;
  //	ubo.VP = VP;
  //	ubo.Far = Far;
  //	ubo.Near = Near;

  //	UniformBufferBlock->updateBufferData(&ubo, sizeof(ubo));
  //}
  //////////////////////////////////////////////////////////////////////////

  // ======= END: public misc methods   =======================================


  // ======= BEGIN: public misc fields ========================================

  ECameraType m_type_;

  math::Vector3Df m_position_;
  math::Vector3Df m_target_;
  math::Vector3Df m_up_;

  math::Vector3Df m_eulerAngle_ = math::g_zeroVector<float, 3>();
  float           m_distance_   = 300.0f;
  math::Matrix4f  m_view_;
  math::Matrix4f  m_projection_;
  math::Matrix4f  m_viewProjection_;
  math::Matrix4f  m_invViewProjection_;
  math::Matrix4f  m_prevViewProjection_;
  math::Matrix4f  m_reverseZProjection_;
  bool            m_isPerspectiveProjection_ = true;
  bool            m_isInfinityFar_           = false;

  // debug object

  // TODO: consider renaming
  float m_FOVRad_ = 0.0f;  // Radian
  float m_Near_   = 0.0f;
  float m_far_    = 0.0f;

  // std::vector<Light*> LightList;
  // AmbientLight* Ambient = nullptr;
  // TODO: consider remove (not used
  bool         m_useAmbient_ = false;
  FrustumPlane m_frustum_;
  int32_t      m_width_  = 0;
  int32_t      m_height_ = 0;

  // TODO: currently not implemented
  // bool  m_isEnableCullMode_       = false;
  // float m_pcfSizeDirectional_   = 2.0f;
  // float m_pcfSizeOmnidirectional_ = 8.0f;

  // ======= END: public misc fields   ========================================
};

class OrthographicCamera : public Camera {
  public:
  // ======= BEGIN: public static methods =====================================

  static OrthographicCamera* s_createCamera(const math::Vector3Df& pos,
                                            const math::Vector3Df& target,
                                            const math::Vector3Df& up,
                                            float                  minX,
                                            float                  minY,
                                            float                  maxX,
                                            float                  maxY,
                                            float                  nearDist,
                                            float                  farDist) {
    OrthographicCamera* camera = new OrthographicCamera();
    s_setCamera(
        camera, pos, target, up, minX, minY, maxX, maxY, nearDist, farDist);
    return camera;
  }

  static void s_setCamera(OrthographicCamera*    camera,
                          const math::Vector3Df& pos,
                          const math::Vector3Df& target,
                          const math::Vector3Df& up,
                          float                  minX,
                          float                  minY,
                          float                  maxX,
                          float                  maxY,
                          float                  nearDist,
                          float                  farDist,
                          float                  distance = 300.0f) {
    const auto kToTarget = (target - pos);
    camera->m_position_  = pos;
    camera->m_target_    = target;
    camera->m_up_        = up;
    camera->m_distance_  = distance;
    camera->setEulerAngle(math::g_getEulerAngleFrom(kToTarget));

    camera->m_Near_                    = nearDist;
    camera->m_far_                     = farDist;
    camera->m_isPerspectiveProjection_ = false;

    camera->m_minX_ = minX;
    camera->m_minY_ = minY;
    camera->m_maxX_ = maxX;
    camera->m_maxY_ = maxY;
  }

  // ======= END: public static methods   =====================================

  // ======= BEGIN: public constructors =======================================

  OrthographicCamera() { m_type_ = ECameraType::ORTHO; }

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public overridden methods =================================

  virtual math::Matrix4f createProjection() const {
    return math::g_orthoLhZo(
        m_minX_, m_maxX_, m_maxY_, m_minY_, m_Near_, m_far_);
  }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public getters ============================================

  float getMinX() const { return m_minX_; }

  float getMinY() const { return m_minY_; }

  float getMaxX() const { return m_maxX_; }

  float getMaxY() const { return m_maxY_; }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public setters ============================================

  void setMinX(float minX) { m_minX_ = minX; }

  void setMinY(float minY) { m_minY_ = minY; }

  void setMaxX(float maxX) { m_maxX_ = maxX; }

  void setMaxY(float maxY) { m_maxY_ = maxY; }

  // ======= END: public setters   ============================================



  private:
  // ======= BEGIN: private misc fields =======================================

  float m_minX_ = 0.0f;
  float m_minY_ = 0.0f;

  float m_maxX_ = 0.0f;
  float m_maxY_ = 0.0f;

  // ======= END: private misc fields   =======================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_CAMERA_H