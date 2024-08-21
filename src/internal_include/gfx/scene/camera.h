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

  std::array<math::Plane, 6> m_planes_;
};

class Camera {
  public:
  enum ECameraType {
    NORMAL = 0,
    ORTHO,
    MAX
  };

  Camera()
      : m_type_(ECameraType::NORMAL) {}

  virtual ~Camera() {}

  static std::map<int32_t, Camera*> s_cameraMap;

  static void s_addCamera(int32_t id, Camera* camera) {
    s_cameraMap.insert(std::make_pair(id, camera));
  }

  static Camera* s_getCamera(int32_t id) {
    auto it_find = s_cameraMap.find(id);
    return (s_cameraMap.end() != it_find) ? it_find->second : nullptr;
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
                              bool                   isPerspectiveProjection) {
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

    const bool IsInvert
        = (eulerAngle.x() < 0 || math::g_kPi < eulerAngle.x());

    const math::Vector3Df UpVector = (IsInvert ? math::g_downVector<float, 3>()
                                               : math::g_upVector<float, 3>());
    right                       = (IsInvert ? math::g_downVector<float, 3>()
                                               : math::g_upVector<float, 3>())
                   .cross(forward)
                   .normalized();
    if (math::g_isNearlyZero(right.magnitudeSquared())) {
      right = (IsInvert ? math::g_backwardVector<float, 3>()
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
    const auto toTarget    = (target - pos);
    camera->m_position_ = pos;
    camera->m_target_   = target;
    camera->m_up_       = up;
    camera->m_distance_ = distance;
    camera->setEulerAngle(math::g_getEulerAngleFrom(toTarget));

    camera->m_FOVRad_                  = fovRad;
    camera->m_Near_                    = nearDist;
    camera->m_far_                     = farDist;
    camera->m_width_                   = static_cast<int32_t>(width);
    camera->m_height_                  = static_cast<int32_t>(height);
    camera->m_isPerspectiveProjection_ = isPerspectiveProjection;
  }

  virtual math::Matrix4f createView() const {
    // TODO: consider adding RH / LH selection logic
    return math::g_lookAtLh(m_position_, m_target_, m_up_);
  }

  virtual math::Matrix4f createProjection() const {
    const auto ratio
        = static_cast<float>(m_width_) / static_cast<float>(m_height_);
    if (m_isPerspectiveProjection_) {
      if (m_isInfinityFar_) {
        return math::g_perspectiveLhZoInf(m_FOVRad_, ratio, m_Near_);
      }
      return math::g_perspectiveLhZo(m_FOVRad_, ratio, m_Near_, m_far_);
    }
    return math::g_orthoLhZo(
        (float)m_width_, (float)m_height_, m_Near_, m_far_);
  }

  // TODO: remove (currently not used)
  // virtual void BindCamera(const Shader* shader) const {
  //  auto VP = Projection * view;
  //  auto P  = Projection;
  //  SET_UNIFORM_BUFFER_STATIC("P", P, shader);
  //  SET_UNIFORM_BUFFER_STATIC("VP", VP, shader);
  //  SET_UNIFORM_BUFFER_STATIC("Eye", m_position_, shader);

  //
  //}

  void updateCameraFrustum() {
    auto       toTarget = (m_target_ - m_position_).normalized();
    const auto length   = tanf(m_FOVRad_) * m_far_;
    auto       toRight  = getRightVector() * length;
    auto       toUp     = getUpVector() * length;

    auto rightUp   = (toTarget * m_far_ + toRight + toUp).normalized();
    auto leftUp    = (toTarget * m_far_ - toRight + toUp).normalized();
    auto rightDown = (toTarget * m_far_ + toRight - toUp).normalized();
    auto leftDown  = (toTarget * m_far_ - toRight - toUp).normalized();

    auto far_lt = m_position_ + leftUp * m_far_;
    auto far_rt = m_position_ + rightUp * m_far_;
    auto far_lb = m_position_ + leftDown * m_far_;
    auto far_rb = m_position_ + rightDown * m_far_;

    auto near_lt = m_position_ + leftUp * m_Near_;
    auto near_rt = m_position_ + rightUp * m_Near_;
    auto near_lb = m_position_ + leftDown * m_Near_;
    auto near_rb = m_position_ + rightDown * m_Near_;

    m_frustum_.m_planes_[0] = math::Plane::s_createFrustumFromThreePoints(
        near_lb, far_lb, near_lt);   // left
    m_frustum_.m_planes_[1] = math::Plane::s_createFrustumFromThreePoints(
        near_rt, far_rt, near_rb);   // right
    m_frustum_.m_planes_[2] = math::Plane::s_createFrustumFromThreePoints(
        near_lt, far_lt, near_rt);   // top
    m_frustum_.m_planes_[3] = math::Plane::s_createFrustumFromThreePoints(
        near_rb, far_rb, near_lb);   // bottom
    m_frustum_.m_planes_[4] = math::Plane::s_createFrustumFromThreePoints(
        near_lb, near_lt, near_rb);  // near
    m_frustum_.m_planes_[5] = math::Plane::s_createFrustumFromThreePoints(
        far_rb, far_rt, far_lb);     // far

    // debug object update
  }

  void updateCamera() {
    m_prevViewProjection_ = m_projection_ * m_view_;

    m_view_                 = createView();
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
    math::Vector3Df ForwardDir;
    math::Vector3Df RightDir;
    math::Vector3Df UpDir;
    s_getForwardRightUpFromEulerAngle(ForwardDir, RightDir, UpDir, m_eulerAngle_);
    m_target_ = m_position_ + ForwardDir * m_distance_;
    m_up_     = m_position_ + UpDir;
  }

  void setEulerAngle(const math::Vector3Df& eulerAngle) {
    if (m_eulerAngle_ != eulerAngle) {
      m_eulerAngle_ = eulerAngle;
      updateCameraParameters();
    }
  }

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
    const auto transformMatrix = math::g_translate(m_position_)
                               * math::g_rotateLh(axis, radian)
                               * math::g_translate(-m_position_);
    m_position_ = math::g_transformPoint(m_position_, transformMatrix);
    m_target_   = math::g_transformPoint(m_target_, transformMatrix);
    m_up_       = math::g_transformPoint(m_up_, transformMatrix);
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

  math::Matrix4f getViewProjectionMatrix() const { return m_viewProjection_; }

  math::Matrix4f getInverseViewProjectionMatrix() const {
    return m_invViewProjection_;
  }

  bool isInFrustum(const math::Vector3Df& pos, float radius) {
    for (auto& iter : m_frustum_.m_planes_) {
      const float d = pos.dot(iter.m_n_) - iter.m_d_ + radius;
      if (d < 0.0f) {
        return false;
      }
    }
    return true;
  }

  bool isInFrustumWithDirection(const math::Vector3Df& pos,
                                const math::Vector3Df& dir,
                                float                  radius) const {
    for (auto& iter : m_frustum_.m_planes_) {
      const float d = pos.dot(iter.m_n_) - iter.m_d_ + radius;
      if (d < 0.0f) {
        if (dir.dot(iter.m_n_) <= 0) {
          return false;
        }
      }
    }
    return true;
  }

  // TODO: seems not used 
  math::Vector3Df getEulerAngle() const { return m_eulerAngle_; }

  void getRectInNDCSpace(math::Vector3Df&      minPosition,
                         math::Vector3Df&      maxPosition,
                         const math::Matrix4f& vp) const {
    math::Vector3Df far_lt;
    math::Vector3Df far_rt;
    math::Vector3Df far_lb;
    math::Vector3Df far_rb;

    math::Vector3Df near_lt;
    math::Vector3Df near_rt;
    math::Vector3Df near_lb;
    math::Vector3Df near_rb;

    const auto  origin = m_position_;
    const float n      = m_Near_;
    const float f      = m_far_;

    if (m_isPerspectiveProjection_) {
      const float     InvAspect = ((float)m_width_ / (float)m_height_);
      const float     length    = tanf(m_FOVRad_ * 0.5f);
      math::Vector3Df targetVec = getForwardVector().normalized();
      math::Vector3Df rightVec
          = getRightVector().normalized() * length * InvAspect;
      math::Vector3Df upVec = getUpVector().normalized() * length;

      math::Vector3Df rightUp   = (targetVec + rightVec + upVec);
      math::Vector3Df leftUp    = (targetVec - rightVec + upVec);
      math::Vector3Df rightDown = (targetVec + rightVec - upVec);
      math::Vector3Df leftDown  = (targetVec - rightVec - upVec);

      far_lt = origin + leftUp * f;
      far_rt = origin + rightUp * f;
      far_lb = origin + leftDown * f;
      far_rb = origin + rightDown * f;

      near_lt = origin + leftUp * n;
      near_rt = origin + rightUp * n;
      near_lb = origin + leftDown * n;
      near_rb = origin + rightDown * n;
    } else {
      const float w = (float)m_width_;
      const float h = (float)m_height_;

      math::Vector3Df targetVec = getForwardVector().normalized();
      math::Vector3Df rightVec  = getRightVector().normalized();
      math::Vector3Df upVec     = getUpVector().normalized();

      far_lt = origin + targetVec * f - rightVec * w * 0.5f + upVec * h * 0.5f;
      far_rt = origin + targetVec * f + rightVec * w * 0.5f + upVec * h * 0.5f;
      far_lb = origin + targetVec * f - rightVec * w * 0.5f - upVec * h * 0.5f;
      far_rb = origin + targetVec * f + rightVec * w * 0.5f - upVec * h * 0.5f;

      near_lt = origin + targetVec * n - rightVec * w * 0.5f + upVec * h * 0.5f;
      near_rt = origin + targetVec * n + rightVec * w * 0.5f + upVec * h * 0.5f;
      near_lb = origin + targetVec * n - rightVec * w * 0.5f - upVec * h * 0.5f;
      near_rb = origin + targetVec * n + rightVec * w * 0.5f - upVec * h * 0.5f;
    }

    // Transform to NDC space
    {
      far_lt = math::g_transformPoint(far_lt, vp);
      far_rt = math::g_transformPoint(far_rt, vp);
      far_lb = math::g_transformPoint(far_lb, vp);
      far_rb = math::g_transformPoint(far_rb, vp);

      near_lt = math::g_transformPoint(near_lt, vp);
      near_rt = math::g_transformPoint(near_rt, vp);
      near_lb = math::g_transformPoint(near_lb, vp);
      near_rb = math::g_transformPoint(near_rb, vp);
    }

    minPosition = math::Vector3Df(FLT_MAX);
    minPosition = std::min(minPosition, far_lt);
    minPosition = std::min(minPosition, far_rt);
    minPosition = std::min(minPosition, far_lb);
    minPosition = std::min(minPosition, far_rb);
    minPosition = std::min(minPosition, near_lt);
    minPosition = std::min(minPosition, near_rt);
    minPosition = std::min(minPosition, near_lb);
    minPosition = std::min(minPosition, near_rb);

    maxPosition = math::Vector3Df(FLT_MIN);
    maxPosition = std::max(maxPosition, far_lt);
    maxPosition = std::max(maxPosition, far_rt);
    maxPosition = std::max(maxPosition, far_lb);
    maxPosition = std::max(maxPosition, far_rb);
    maxPosition = std::max(maxPosition, near_lt);
    maxPosition = std::max(maxPosition, near_rt);
    maxPosition = std::max(maxPosition, near_lb);
    maxPosition = std::max(maxPosition, near_rb);
  }

  void getRectInScreenSpace(math::Vector3Df&       minPosition,
                            math::Vector3Df&       maxPosition,
                            const math::Matrix4f&  vp,
                            const math::Vector2Df& screenSize
                            = math::Vector2Df(1.0f, 1.0f)) const {
    getRectInNDCSpace(minPosition, maxPosition, vp);

    // Min XY
    minPosition     = std::max(minPosition, math::Vector3Df(-1.0f, -1.0f, -1.0f));
    minPosition.x() = (minPosition.x() * 0.5f + 0.5f) * screenSize.x();
    minPosition.y() = (minPosition.y() * 0.5f + 0.5f) * screenSize.y();

    // Max XY
    maxPosition     = std::min(maxPosition, math::Vector3Df(1.0f, 1.0f, 1.0f));
    maxPosition.x() = (maxPosition.x() * 0.5f + 0.5f) * screenSize.x();
    maxPosition.y() = (maxPosition.y() * 0.5f + 0.5f) * screenSize.y();
  }

  void getFrustumVertexInWorld(math::Vector3Df* vertexArray) const {
    math::Vector3Df far_lt;
    math::Vector3Df far_rt;
    math::Vector3Df far_lb;
    math::Vector3Df far_rb;

    math::Vector3Df near_lt;
    math::Vector3Df near_rt;
    math::Vector3Df near_lb;
    math::Vector3Df near_rb;

    const auto  origin = m_position_;
    const float n      = m_Near_;
    const float f      = m_far_;

    if (m_isPerspectiveProjection_) {
      const float     InvAspect = ((float)m_width_ / (float)m_height_);
      const float     length    = tanf(m_FOVRad_ * 0.5f);
      math::Vector3Df targetVec = getForwardVector().normalized();
      math::Vector3Df rightVec
          = getRightVector().normalized() * length * InvAspect;
      math::Vector3Df upVec = getUpVector().normalized() * length;

      math::Vector3Df rightUp   = (targetVec + rightVec + upVec);
      math::Vector3Df leftUp    = (targetVec - rightVec + upVec);
      math::Vector3Df rightDown = (targetVec + rightVec - upVec);
      math::Vector3Df leftDown  = (targetVec - rightVec - upVec);

      far_lt = origin + leftUp * f;
      far_rt = origin + rightUp * f;
      far_lb = origin + leftDown * f;
      far_rb = origin + rightDown * f;

      near_lt = origin + leftUp * n;
      near_rt = origin + rightUp * n;
      near_lb = origin + leftDown * n;
      near_rb = origin + rightDown * n;
    } else {
      const float w = (float)m_width_;
      const float h = (float)m_height_;

      math::Vector3Df targetVec = getForwardVector().normalized();
      math::Vector3Df rightVec  = getRightVector().normalized();
      math::Vector3Df upVec     = getUpVector().normalized();

      far_lt = origin + targetVec * f - rightVec * w * 0.5f + upVec * h * 0.5f;
      far_rt = origin + targetVec * f + rightVec * w * 0.5f + upVec * h * 0.5f;
      far_lb = origin + targetVec * f - rightVec * w * 0.5f - upVec * h * 0.5f;
      far_rb = origin + targetVec * f + rightVec * w * 0.5f - upVec * h * 0.5f;

      near_lt = origin + targetVec * n - rightVec * w * 0.5f + upVec * h * 0.5f;
      near_rt = origin + targetVec * n + rightVec * w * 0.5f + upVec * h * 0.5f;
      near_lb = origin + targetVec * n - rightVec * w * 0.5f - upVec * h * 0.5f;
      near_rb = origin + targetVec * n + rightVec * w * 0.5f - upVec * h * 0.5f;
    }

    vertexArray[0] = far_lt;
    vertexArray[1] = far_rt;
    vertexArray[2] = far_lb;
    vertexArray[3] = far_rb;

    vertexArray[4] = near_lt;
    vertexArray[5] = near_rt;
    vertexArray[6] = near_lb;
    vertexArray[7] = near_rb;
  }

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
  //bool  m_isEnableCullMode_       = false;
  //float m_pcfSizeDirectional_   = 2.0f;
  //float m_pcfSizeOmnidirectional_ = 8.0f;
};

class OrthographicCamera : public Camera {
  public:
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
    const auto toTarget    = (target - pos);
    camera->m_position_ = pos;
    camera->m_target_   = target;
    camera->m_up_       = up;
    camera->m_distance_ = distance;
    camera->setEulerAngle(math::g_getEulerAngleFrom(toTarget));

    camera->m_Near_                    = nearDist;
    camera->m_far_                     = farDist;
    camera->m_isPerspectiveProjection_ = false;

    camera->m_minX_ = minX;
    camera->m_minY_ = minY;
    camera->m_maxX_ = maxX;
    camera->m_maxY_ = maxY;
  }

  OrthographicCamera() { m_type_ = ECameraType::ORTHO; }

  virtual math::Matrix4f createProjection() const {
    return math::g_orthoLhZo(m_minX_, m_maxX_, m_maxY_, m_minY_, m_Near_, m_far_);
  }

  float getMinX() const { return m_minX_; }

  float getMinY() const { return m_minY_; }

  float getMaxX() const { return m_maxX_; }

  float getMaxY() const { return m_maxY_; }

  void setMinX(float minX) { m_minX_ = minX; }

  void setMinY(float minY) { m_minY_ = minY; }

  void setMaxX(float maxX) { m_maxX_ = maxX; }

  void setMaxY(float maxY) { m_maxY_ = maxY; }

  private:
  float m_minX_ = 0.0f;
  float m_minY_ = 0.0f;

  float m_maxX_ = 0.0f;
  float m_maxY_ = 0.0f;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_CAMERA_H