// TODO:
// - move to more appropriate folder (e.g. scene)
// - consider moving scene folder to a more appropriate location (gfx is not the
// very best place for it)

#ifndef GAME_ENGINE_CAMERA_H
#define GAME_ENGINE_CAMERA_H

#include "utils/math/math_util.h"
#include "utils/math/plane.h"
#include "gfx/rhi/vulkan/feature_switch_vk.h"


#include <math_library/graphics.h>
#include <math_library/matrix.h>
#include <math_library/vector.h>

#include <array>
#include <map>

namespace game_engine {

struct FrustumPlane {
  bool IsInFrustum(const math::Vector3Df& pos, float radius) const {
    for (const auto& plane : Planes) {
      const float r = pos.dot(plane.n) - plane.d + radius;
      if (r < 0.0f) {
        return false;
      }
    }
    return true;
  }

  bool IsInFrustumWithDirection(const math::Vector3Df& pos,
                                const math::Vector3Df& direction,
                                float                  radius) const {
    for (const auto& plane : Planes) {
      const float r = pos.dot(plane.n) - plane.d + radius;
      if (r < 0.0f) {
        if (direction.dot(plane.n) <= 0.0) {
          return false;
        }
      }
    }
    return true;
  }

  std::array<math::Plane, 6> Planes;
};

//namespace CameraUtil {
// math::g_lookAtLh(pos, target, up); - default but support
// math::g_lookAtRh(pos, target, up);
//math::Matrix4f CreateViewMatrix(const math::Vector3Df& pos,
//                                const math::Vector3Df& target,
//                                const math::Vector3Df& up) {
//  const auto zAxis = (target - pos).normalize();
//  auto       yAxis = (up - pos).normalize();
//  const auto xAxis = yAxis.cross(zAxis).normalize();
//  yAxis            = zAxis.cross(xAxis).normalize();
//
//  math::Matrix4f InvRot{IdentityType};
//  InvRot.m[0][0] = xAxis.x();
//  InvRot.m[1][0] = xAxis.y();
//  InvRot.m[2][0] = xAxis.z();
//  InvRot.m[0][1] = yAxis.x();
//  InvRot.m[1][1] = yAxis.y();
//  InvRot.m[2][1] = yAxis.z();
//#if RIGHT_HANDED
//  InvRot.m[0][2] = -zAxis.x();
//  InvRot.m[1][2] = -zAxis.y();
//  InvRot.m[2][2] = -zAxis.z();
//#else
//  InvRot.m[0][2] = zAxis.x();
//  InvRot.m[1][2] = zAxis.y();
//  InvRot.m[2][2] = zAxis.z();
//#endif
//
//  // auto InvPos = Matrix::MakeTranslate(-pos.x(), -pos.y(), -pos.z());
//  // return InvRot * InvPos;
//
//  auto InvPos    = math::Vector4Df(-pos.x(), -pos.y(), -pos.z(), 1.0f);
//  InvRot.m[3][0] = InvRot.getColumn<0>().dot(InvPos);
//  InvRot.m[3][1] = InvRot.getColumn<1>().dot(InvPos);
//  InvRot.m[3][2] = InvRot.getColumn<2>().dot(InvPos);
//
//  return InvRot;
//}

// math::g_perspectiveLhZo(fov, width, height, nearDist, farDist) but support
// math::g_perspectiveRhZo(fov, width, height, nearDist, farDist)
//math::Matrix4f CreatePerspectiveMatrix(
//    float width, float height, float fov, float nearDist, float farDist) {
//  const float F          = 1.0f / tanf(fov / 2.0f);
//  const float farSubNear = (farDist - nearDist);
//
//  math::Matrix4f projMat;
//#if RIGHT_HANDED
//  projMat.m[0][0] = F * (height / width);
//  projMat.m[1][0] = 0.0f;
//  projMat.m[2][0] = 0.0f;
//  projMat.m[3][0] = 0.0f;
//  projMat.m[0][1] = 0.0f;
//  projMat.m[1][1] = F;
//  projMat.m[2][1] = 0.0f;
//  projMat.m[3][1] = 0.0f;
//  projMat.m[0][2] = 0.0f;
//  projMat.m[1][2] = 0.0f;
//  projMat.m[2][2] = -(farDist + nearDist) / farSubNear;
//  projMat.m[3][2] = -(2.0f * nearDist * farDist) / farSubNear;
//  projMat.m[0][3] = 0.0f;
//  projMat.m[1][3] = 0.0f;
//  projMat.m[2][3] = -1.0f;
//  projMat.m[3][3] = 0.0f;
//#else
//  projMat.m[0][0] = F * (height / width);
//  projMat.m[1][0] = 0.0f;
//  projMat.m[2][0] = 0.0f;
//  projMat.m[3][0] = 0.0f;
//  projMat.m[0][1] = 0.0f;
//  projMat.m[1][1] = F;
//  projMat.m[2][1] = 0.0f;
//  projMat.m[3][1] = 0.0f;
//  projMat.m[0][2] = 0.0f;
//  projMat.m[1][2] = 0.0f;
//  projMat.m[2][2] = farDist / farSubNear;
//  projMat.m[3][2] = -nearDist * farDist / farSubNear;
//  projMat.m[0][3] = 0.0f;
//  projMat.m[1][3] = 0.0f;
//  projMat.m[2][3] = 1.0f;
//  projMat.m[3][3] = 0.0f;
//#endif
//  return projMat;
//}

//math::Matrix4f CreatePerspectiveMatrixFarAtInfinity(float width,
//                                                    float height,
//                                                    float fov,
//                                                    float nearDist) {
//  const float     F        = 1.0f / tanf(fov / 2.0f);
//  constexpr float epsilone = FLT_EPSILON * 2.0f;
//
//  math::Matrix4f projMat;
//#if RIGHT_HANDED
//  projMat.m[0][0] = F * (height / width);
//  projMat.m[1][0] = 0.0f;
//  projMat.m[2][0] = 0.0f;
//  projMat.m[3][0] = 0.0f;
//  projMat.m[0][1] = 0.0f;
//  projMat.m[1][1] = F;
//  projMat.m[2][1] = 0.0f;
//  projMat.m[3][1] = 0.0f;
//  projMat.m[0][2] = 0.0f;
//  projMat.m[1][2] = 0.0f;
//  projMat.m[2][2] = epsilone - 1.0f;
//  projMat.m[3][2] = ((epsilone - 2.0f) * nearDist);
//  projMat.m[0][3] = 0.0f;
//  projMat.m[1][3] = 0.0f;
//  projMat.m[2][3] = -1.0f;
//  projMat.m[3][3] = 0.0f;
//#else
//  projMat.m[0][0] = F * (height / width);
//  projMat.m[1][0] = 0.0f;
//  projMat.m[2][0] = 0.0f;
//  projMat.m[3][0] = 0.0f;
//  projMat.m[0][1] = 0.0f;
//  projMat.m[1][1] = F;
//  projMat.m[2][1] = 0.0f;
//  projMat.m[3][1] = 0.0f;
//  projMat.m[0][2] = 0.0f;
//  projMat.m[1][2] = 0.0f;
//  projMat.m[2][2] = 1.0f - epsilone;
//  projMat.m[3][2] = ((-1.0f - epsilone) * nearDist);
//  projMat.m[0][3] = 0.0f;
//  projMat.m[1][3] = 0.0f;
//  projMat.m[2][3] = 1.0f;
//  projMat.m[3][3] = 0.0f;
//#endif
//  return projMat;
//}

//math::Matrix4f CreateOrthogonalMatrix(float width,
//                                      float height,
//                                      float nearDist,
//                                      float farDist) {
//  const float    fsn        = (farDist - nearDist);
//  const float    halfWidth  = width * 0.5f;
//  const float    halfHeight = height * 0.5f;
//  math::Matrix4f projMat;
//#if RIGHT_HANDED
//  projMat.m[0][0] = 1.0f / halfWidth;
//  projMat.m[1][0] = 0.0f;
//  projMat.m[2][0] = 0.0f;
//  projMat.m[3][0] = 0.0f;
//  projMat.m[0][1] = 0.0f;
//  projMat.m[1][1] = 1.0f / halfHeight;
//  projMat.m[2][1] = 0.0f;
//  projMat.m[3][1] = 0.0f;
//  projMat.m[0][2] = 0.0f;
//  projMat.m[1][2] = 0.0f;
//  projMat.m[2][2] = -2.0f / fsn;
//  projMat.m[3][2] = -(farDist + nearDist) / fsn;
//  projMat.m[0][3] = 0.0f;
//  projMat.m[1][3] = 0.0f;
//  projMat.m[2][3] = 0.0f;
//  projMat.m[3][3] = 1.0f;
//#else
//  projMat.m[0][0] = 1.0f / halfWidth;
//  projMat.m[1][0] = 0.0f;
//  projMat.m[2][0] = 0.0f;
//  projMat.m[3][0] = 0.0f;
//  projMat.m[0][1] = 0.0f;
//  projMat.m[1][1] = 1.0f / halfHeight;
//  projMat.m[2][1] = 0.0f;
//  projMat.m[3][1] = 0.0f;
//  projMat.m[0][2] = 0.0f;
//  projMat.m[1][2] = 0.0f;
//  projMat.m[2][2] = 1.0f / fsn;
//  projMat.m[3][2] = -(nearDist) / fsn;
//  projMat.m[0][3] = 0.0f;
//  projMat.m[1][3] = 0.0f;
//  projMat.m[2][3] = 0.0f;
//  projMat.m[3][3] = 1.0f;
//#endif
//  return projMat;
//}

// math::g_orthoLhZo(fov, width, height, nearDist, farDist) but support
// math::g_orthoRhZo(fov, width, height, nearDist, farDist)
//math::Matrix4f CreateOrthogonalMatrix(float left,
//                                      float right,
//                                      float top,
//                                      float bottom,
//                                      float nearDist,
//                                      float farDist) {
//  const float fsn = (farDist - nearDist);
//  const float rsl = (right - left);
//  const float tsb = (top - bottom);
//
//  math::Matrix4f projMat;
//#if RIGHT_HANDED
//  projMat.m[0][0] = 2.0f / rsl;
//  projMat.m[1][0] = 0.0f;
//  projMat.m[2][0] = 0.0f;
//  projMat.m[3][0] = -(right + left) / rsl;
//  projMat.m[0][1] = 0.0f;
//  projMat.m[1][1] = 2.0f / tsb;
//  projMat.m[2][1] = 0.0f;
//  projMat.m[3][1] = -(top + bottom) / tsb;
//  projMat.m[0][2] = 0.0f;
//  projMat.m[1][2] = 0.0f;
//  projMat.m[2][2] = -2.0f / fsn;
//  projMat.m[3][2] = -(farDist + nearDist) / fsn;
//  projMat.m[0][3] = 0.0f;
//  projMat.m[1][3] = 0.0f;
//  projMat.m[2][3] = 0.0f;
//  projMat.m[3][3] = 1.0f;
//#else
//  projMat.m[0][0] = 2.0f / rsl;
//  projMat.m[1][0] = 0.0f;
//  projMat.m[2][0] = 0.0f;
//  projMat.m[3][0] = -(right + left) / rsl;
//  projMat.m[0][1] = 0.0f;
//  projMat.m[1][1] = 2.0f / tsb;
//  projMat.m[2][1] = 0.0f;
//  projMat.m[3][1] = -(top + bottom) / tsb;
//  projMat.m[0][2] = 0.0f;
//  projMat.m[1][2] = 0.0f;
//  projMat.m[2][2] = 1.0f / fsn;
//  projMat.m[3][2] = -(nearDist) / fsn;
//  projMat.m[0][3] = 0.0f;
//  projMat.m[1][3] = 0.0f;
//  projMat.m[2][3] = 0.0f;
//  projMat.m[3][3] = 1.0f;
//#endif
//  return projMat;
//}
//}  // namespace CameraUtil

class Camera {
  public:
  enum ECameraType {
    NORMAL = 0,
    ORTHO,
    MAX
  };

  Camera()
      : Type(ECameraType::NORMAL) {}

  virtual ~Camera() {}

  static std::map<int32_t, Camera*> CameraMap;

  static void AddCamera(int32_t id, Camera* camera) {
    CameraMap.insert(std::make_pair(id, camera));
  }

  static Camera* GetCamera(int32_t id) {
    auto it_find = CameraMap.find(id);
    return (CameraMap.end() != it_find) ? it_find->second : nullptr;
  }

  static void RemoveCamera(int32_t id) { CameraMap.erase(id); }

  static Camera* GetMainCamera() { return GetCamera(0); }

  static Camera* CreateCamera(const math::Vector3Df& pos,
                               const math::Vector3Df& target,
                               const math::Vector3Df& up,
                               float                  fovRad,
                               float                  nearDist,
                               float                  farDist,
                               float                  width,
                               float                  height,
                               bool                   isPerspectiveProjection) {
    Camera* camera = new Camera();
    SetCamera(camera,
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

  static void GetForwardRightUpFromEulerAngle(
      math::Vector3Df&       OutForward,
      math::Vector3Df&       OutRight,
      math::Vector3Df&       OutUp,
      const math::Vector3Df& InEulerAngle) {
    OutForward = math::GetDirectionFromEulerAngle(InEulerAngle).normalize();

    const bool IsInvert
        = (InEulerAngle.x() < 0 || math::g_kPi < InEulerAngle.x());

    const math::Vector3Df UpVector = (IsInvert ? math::g_downVector<float, 3>()
                                               : math::g_upVector<float, 3>());
    OutRight                       = (IsInvert ? math::g_downVector<float, 3>()
                                               : math::g_upVector<float, 3>())
                   .cross(OutForward)
                   .normalize();
    if (math::g_isNearlyZero(OutRight.magnitudeSquared())) {
      OutRight = (IsInvert ? math::g_backwardVector<float, 3>()
                           : math::g_forwardVector<float, 3>())
                     .cross(OutForward)
                     .normalize();
    }
    OutUp = OutForward.cross(OutRight).normalize();
  }

  static void SetCamera(Camera*               OutCamera,
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
    const auto toTarget = (target - pos);
    OutCamera->Pos      = pos;
    OutCamera->Target   = target;
    OutCamera->Up       = up;
    OutCamera->Distance = distance;
    OutCamera->SetEulerAngle(math::GetEulerAngleFrom(toTarget));

    OutCamera->FOVRad                  = fovRad;
    OutCamera->Near                    = nearDist;
    OutCamera->Far                     = farDist;
    OutCamera->Width                   = static_cast<int32_t>(width);
    OutCamera->Height                  = static_cast<int32_t>(height);
    OutCamera->IsPerspectiveProjection = isPerspectiveProjection;
  }

  virtual math::Matrix4f CreateView() const {
    // TODO: consider adding RH / LH selection logic
    return math::g_lookAtLh(Pos, Target, Up);
  }

  virtual math::Matrix4f CreateProjection() const {
    const auto ratio = static_cast<float>(Width) / static_cast<float>(Height);
    if (IsPerspectiveProjection) {
      if (IsInfinityFar) {
        return math::g_perspectiveLhZoInf(FOVRad, ratio, Near);
      }
      return math::g_perspectiveLhZo(FOVRad, ratio, Near, Far);
    }
    return math::g_orthoLhZo((float)Width, (float)Height, Near, Far);
  }

  // TODO: remove (currently not used)
  //virtual void BindCamera(const Shader* shader) const {
  //  auto VP = Projection * view;
  //  auto P  = Projection;
  //  SET_UNIFORM_BUFFER_STATIC("P", P, shader);
  //  SET_UNIFORM_BUFFER_STATIC("VP", VP, shader);
  //  SET_UNIFORM_BUFFER_STATIC("Eye", Pos, shader);

  // 
  //}

  void UpdateCameraFrustum() {
    auto       toTarget = (Target - Pos).normalize();
    const auto length   = tanf(FOVRad) * Far;
    auto       toRight  = GetRightVector() * length;
    auto       toUp     = GetUpVector() * length;

    auto rightUp   = (toTarget * Far + toRight + toUp).normalize();
    auto leftUp    = (toTarget * Far - toRight + toUp).normalize();
    auto rightDown = (toTarget * Far + toRight - toUp).normalize();
    auto leftDown  = (toTarget * Far - toRight - toUp).normalize();

    auto far_lt = Pos + leftUp * Far;
    auto far_rt = Pos + rightUp * Far;
    auto far_lb = Pos + leftDown * Far;
    auto far_rb = Pos + rightDown * Far;

    auto near_lt = Pos + leftUp * Near;
    auto near_rt = Pos + rightUp * Near;
    auto near_lb = Pos + leftDown * Near;
    auto near_rb = Pos + rightDown * Near;

    Frustum.Planes[0] = math::Plane::CreateFrustumFromThreePoints(
        near_lb, far_lb, near_lt);   // left
    Frustum.Planes[1] = math::Plane::CreateFrustumFromThreePoints(
        near_rt, far_rt, near_rb);   // right
    Frustum.Planes[2] = math::Plane::CreateFrustumFromThreePoints(
        near_lt, far_lt, near_rt);   // top
    Frustum.Planes[3] = math::Plane::CreateFrustumFromThreePoints(
        near_rb, far_rb, near_lb);   // bottom
    Frustum.Planes[4] = math::Plane::CreateFrustumFromThreePoints(
        near_lb, near_lt, near_rb);  // near
    Frustum.Planes[5] = math::Plane::CreateFrustumFromThreePoints(
        far_rb, far_rt, far_lb);     // far

    // debug object update
  }

  void UpdateCamera() {
    PrevViewProjection = Projection * view;

    view              = CreateView();
    Projection        = CreateProjection();
    ViewProjection    = Projection * view;
    InvViewProjection = ViewProjection.inverse();
    if (IsPerspectiveProjection) {
#if USE_REVERSEZ_PERSPECTIVE_SHADOW
        // TODO: add implementation
#else
      ReverseZProjection = Projection;
#endif
    }
  }

  void UpdateCameraParameters() {
    math::Vector3Df ForwardDir;
    math::Vector3Df RightDir;
    math::Vector3Df UpDir;
    GetForwardRightUpFromEulerAngle(ForwardDir, RightDir, UpDir, EulerAngle);
    Target = Pos + ForwardDir * Distance;
    Up     = Pos + UpDir;
  }

  void SetEulerAngle(const math::Vector3Df& InEulerAngle) {
    if (EulerAngle != InEulerAngle) {
      EulerAngle = InEulerAngle;
      UpdateCameraParameters();
    }
  }

  math::Vector3Df GetForwardVector() const {
    return view.getColumn<2>().resizedCopy<3>();
  }

  math::Vector3Df GetUpVector() const {
    return view.getColumn<1>().resizedCopy<3>();
  }

  math::Vector3Df GetRightVector() const {
    return view.getColumn<0>().resizedCopy<3>();
  }

  void MoveShift(float dist) {
    auto toRight  = GetRightVector() * dist;
    Pos          += toRight;
    Target       += toRight;
    Up           += toRight;
  }

  void MoveForward(float dist) {
    auto toForward  = GetForwardVector() * dist;
    Pos            += toForward;
    Target         += toForward;
    Up             += toForward;
  }

  void RotateCameraAxis(const math::Vector3Df& axis, float radian) {
    const auto transformMatrix = math::g_translate(Pos)
                               * math::g_rotateLh(axis, radian)
                               * math::g_translate(-Pos);
    Pos    = math::g_transformPoint(Pos, transformMatrix);
    Target = math::g_transformPoint(Target, transformMatrix);
    Up     = math::g_transformPoint(Up, transformMatrix);
  }

  void RotateForwardAxis(float radian) {
    RotateCameraAxis(GetForwardVector(), radian);
  }

  void RotateUpAxis(float radian) { RotateCameraAxis(GetUpVector(), radian); }

  void RotateRightAxis(float radian) {
    RotateCameraAxis(GetRightVector(), radian);
  }

  void RotateXAxis(float radian) {
    RotateCameraAxis(math::Vector3Df(1.0f, 0.0f, 0.0f), radian);
  }

  void RotateYAxis(float radian) {
    RotateCameraAxis(math::Vector3Df(0.0f, 1.0f, 0.0f), radian);
  }

  void RotateZAxis(float radian) {
    RotateCameraAxis(math::Vector3Df(0.0f, 0.0f, 1.0f), radian);
  }

  math::Matrix4f GetViewProjectionMatrix() const { return ViewProjection; }

  math::Matrix4f GetInverseViewProjectionMatrix() const {
    return InvViewProjection;
  }

  bool IsInFrustum(const math::Vector3Df& pos, float radius) {
    for (auto& iter : Frustum.Planes) {
      const float d = pos.dot(iter.n) - iter.d + radius;
      if (d < 0.0f) {
        return false;
      }
    }
    return true;
  }

  bool IsInFrustumWithDirection(const math::Vector3Df& pos,
                                const math::Vector3Df& dir,
                                float                  radius) const {
    for (auto& iter : Frustum.Planes) {
      const float d = pos.dot(iter.n) - iter.d + radius;
      if (d < 0.0f) {
        if (dir.dot(iter.n) <= 0) {
          return false;
        }
      }
    }
    return true;
  }

  math::Vector3Df GetEulerAngle() const { return EulerAngle; }

  void GetRectInNDCSpace(math::Vector3Df&      OutPosMin,
                         math::Vector3Df&      OutPosMax,
                         const math::Matrix4f& InVP) const {
    math::Vector3Df far_lt;
    math::Vector3Df far_rt;
    math::Vector3Df far_lb;
    math::Vector3Df far_rb;

    math::Vector3Df near_lt;
    math::Vector3Df near_rt;
    math::Vector3Df near_lb;
    math::Vector3Df near_rb;

    const auto  origin = Pos;
    const float n      = Near;
    const float f      = Far;

    if (IsPerspectiveProjection) {
      const float     InvAspect = ((float)Width / (float)Height);
      const float     length    = tanf(FOVRad * 0.5f);
      math::Vector3Df targetVec = GetForwardVector().normalize();
      math::Vector3Df rightVec
          = GetRightVector().normalize() * length * InvAspect;
      math::Vector3Df upVec = GetUpVector().normalize() * length;

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
      const float w = (float)Width;
      const float h = (float)Height;

      math::Vector3Df targetVec = GetForwardVector().normalize();
      math::Vector3Df rightVec  = GetRightVector().normalize();
      math::Vector3Df upVec     = GetUpVector().normalize();

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
      far_lt = math::g_transformPoint(far_lt, InVP);
      far_rt = math::g_transformPoint(far_rt, InVP);
      far_lb = math::g_transformPoint(far_lb, InVP);
      far_rb = math::g_transformPoint(far_rb, InVP);

      near_lt = math::g_transformPoint(near_lt, InVP);
      near_rt = math::g_transformPoint(near_rt, InVP);
      near_lb = math::g_transformPoint(near_lb, InVP);
      near_rb = math::g_transformPoint(near_rb, InVP);
    }

    OutPosMin = math::Vector3Df(FLT_MAX);
    OutPosMin = std::min(OutPosMin, far_lt);
    OutPosMin = std::min(OutPosMin, far_rt);
    OutPosMin = std::min(OutPosMin, far_lb);
    OutPosMin = std::min(OutPosMin, far_rb);
    OutPosMin = std::min(OutPosMin, near_lt);
    OutPosMin = std::min(OutPosMin, near_rt);
    OutPosMin = std::min(OutPosMin, near_lb);
    OutPosMin = std::min(OutPosMin, near_rb);

    OutPosMax = math::Vector3Df(FLT_MIN);
    OutPosMax = std::max(OutPosMax, far_lt);
    OutPosMax = std::max(OutPosMax, far_rt);
    OutPosMax = std::max(OutPosMax, far_lb);
    OutPosMax = std::max(OutPosMax, far_rb);
    OutPosMax = std::max(OutPosMax, near_lt);
    OutPosMax = std::max(OutPosMax, near_rt);
    OutPosMax = std::max(OutPosMax, near_lb);
    OutPosMax = std::max(OutPosMax, near_rb);
  }

  void GetRectInScreenSpace(math::Vector3Df&       OutPosMin,
                            math::Vector3Df&       OutPosMax,
                            const math::Matrix4f&  InVP,
                            const math::Vector2Df& InScreenSize
                            = math::Vector2Df(1.0f, 1.0f)) const {
    GetRectInNDCSpace(OutPosMin, OutPosMax, InVP);

    // Min XY
    OutPosMin     = std::max(OutPosMin, math::Vector3Df(-1.0f, -1.0f, -1.0f));
    OutPosMin.x() = (OutPosMin.x() * 0.5f + 0.5f) * InScreenSize.x();
    OutPosMin.y() = (OutPosMin.y() * 0.5f + 0.5f) * InScreenSize.y();

    // Max XY
    OutPosMax     = std::min(OutPosMax, math::Vector3Df(1.0f, 1.0f, 1.0f));
    OutPosMax.x() = (OutPosMax.x() * 0.5f + 0.5f) * InScreenSize.x();
    OutPosMax.y() = (OutPosMax.y() * 0.5f + 0.5f) * InScreenSize.y();
  }

  void GetFrustumVertexInWorld(math::Vector3Df* OutVertexArray) const {
    math::Vector3Df far_lt;
    math::Vector3Df far_rt;
    math::Vector3Df far_lb;
    math::Vector3Df far_rb;

    math::Vector3Df near_lt;
    math::Vector3Df near_rt;
    math::Vector3Df near_lb;
    math::Vector3Df near_rb;

    const auto  origin = Pos;
    const float n      = Near;
    const float f      = Far;

    if (IsPerspectiveProjection) {
      const float     InvAspect = ((float)Width / (float)Height);
      const float     length    = tanf(FOVRad * 0.5f);
      math::Vector3Df targetVec = GetForwardVector().normalize();
      math::Vector3Df rightVec
          = GetRightVector().normalize() * length * InvAspect;
      math::Vector3Df upVec = GetUpVector().normalize() * length;

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
      const float w = (float)Width;
      const float h = (float)Height;

      math::Vector3Df targetVec = GetForwardVector().normalize();
      math::Vector3Df rightVec  = GetRightVector().normalize();
      math::Vector3Df upVec     = GetUpVector().normalize();

      far_lt = origin + targetVec * f - rightVec * w * 0.5f + upVec * h * 0.5f;
      far_rt = origin + targetVec * f + rightVec * w * 0.5f + upVec * h * 0.5f;
      far_lb = origin + targetVec * f - rightVec * w * 0.5f - upVec * h * 0.5f;
      far_rb = origin + targetVec * f + rightVec * w * 0.5f - upVec * h * 0.5f;

      near_lt = origin + targetVec * n - rightVec * w * 0.5f + upVec * h * 0.5f;
      near_rt = origin + targetVec * n + rightVec * w * 0.5f + upVec * h * 0.5f;
      near_lb = origin + targetVec * n - rightVec * w * 0.5f - upVec * h * 0.5f;
      near_rb = origin + targetVec * n + rightVec * w * 0.5f - upVec * h * 0.5f;
    }

    OutVertexArray[0] = far_lt;
    OutVertexArray[1] = far_rt;
    OutVertexArray[2] = far_lb;
    OutVertexArray[3] = far_rb;

    OutVertexArray[4] = near_lt;
    OutVertexArray[5] = near_rt;
    OutVertexArray[6] = near_lb;
    OutVertexArray[7] = near_rb;
  }

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
  //		UniformBufferBlock = g_rhi->CreateUniformBufferBlock("Camera",
  // sizeof(UniformBufferCamera));
  //	}

  //	const auto& V = view;
  //	const auto& VP = Projection * view;

  //	UniformBufferCamera ubo = {};
  //	ubo.V = V;
  //	ubo.VP = VP;
  //	ubo.Far = Far;
  //	ubo.Near = Near;

  //	UniformBufferBlock->UpdateBufferData(&ubo, sizeof(ubo));
  //}
  //////////////////////////////////////////////////////////////////////////

  ECameraType Type;

  math::Vector3Df Pos;
  math::Vector3Df Target;
  math::Vector3Df Up;

  math::Vector3Df EulerAngle = math::g_zeroVector<float, 3>();
  float           Distance   = 300.0f;
  math::Matrix4f  view;
  math::Matrix4f  Projection;
  math::Matrix4f  ViewProjection;
  math::Matrix4f  InvViewProjection;
  math::Matrix4f  PrevViewProjection;
  math::Matrix4f  ReverseZProjection;
  bool            IsPerspectiveProjection = true;
  bool            IsInfinityFar           = false;

  // debug object

  float FOVRad = 0.0f;  // Radian
  float Near   = 0.0f;
  float Far    = 0.0f;

  // std::vector<Light*> LightList;
  // AmbientLight* Ambient = nullptr;
  bool          UseAmbient = false;
  FrustumPlane Frustum;
  int32_t       Width  = 0;
  int32_t       Height = 0;

  // todo
  bool  IsEnableCullMode         = false;
  float PCF_SIZE_DIRECTIONAL     = 2.0f;
  float PCF_SIZE_OMNIDIRECTIONAL = 8.0f;
};

class OrthographicCamera : public Camera {
  public:
  static OrthographicCamera* CreateCamera(const math::Vector3Df& pos,
                                           const math::Vector3Df& target,
                                           const math::Vector3Df& up,
                                           float                  minX,
                                           float                  minY,
                                           float                  maxX,
                                           float                  maxY,
                                           float                  nearDist,
                                           float                  farDist) {
    OrthographicCamera* camera = new OrthographicCamera();
    SetCamera(
        camera, pos, target, up, minX, minY, maxX, maxY, nearDist, farDist);
    return camera;
  }

  static void SetCamera(OrthographicCamera*   OutCamera,
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
    const auto toTarget = (target - pos);
    OutCamera->Pos      = pos;
    OutCamera->Target   = target;
    OutCamera->Up       = up;
    OutCamera->Distance = distance;
    OutCamera->SetEulerAngle(math::GetEulerAngleFrom(toTarget));

    OutCamera->Near                    = nearDist;
    OutCamera->Far                     = farDist;
    OutCamera->IsPerspectiveProjection = false;

    OutCamera->MinX = minX;
    OutCamera->MinY = minY;
    OutCamera->MaxX = maxX;
    OutCamera->MaxY = maxY;
  }

  OrthographicCamera() { Type = ECameraType::ORTHO; }

  virtual math::Matrix4f CreateProjection() const {
    return math::g_orthoLhZo(MinX, MaxX, MaxY, MinY, Near, Far);
  }

  float GetMinX() const { return MinX; }

  float GetMinY() const { return MinY; }

  float GetMaxX() const { return MaxX; }

  float GetMaxY() const { return MaxY; }

  void SetMinX(float minX) { MinX = minX; }

  void SetMinY(float minY) { MinY = minY; }

  void SetMaxX(float maxX) { MaxX = maxX; }

  void SetMaxY(float maxY) { MaxY = maxY; }

  private:
  float MinX = 0.0f;
  float MinY = 0.0f;

  float MaxX = 0.0f;
  float MaxY = 0.0f;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_CAMERA_H