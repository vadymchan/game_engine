#ifndef GAME_ENGINE_MATH_UTIL_H
#define GAME_ENGINE_MATH_UTIL_H

#include <math_library/graphics.h>
#include <math_library/matrix.h>
#include <math_library/vector.h>

// TODO: consider move this code to math_library third party
namespace math {

// TODO:
// - add functions like get matrix w/o scale/translate/rotate component
// - add functions like make scale + translate (and similar)

// ----------------------------------------------------------------------------

// TODO:
// - acosf(y) suggests measuring the angle from the 'up' direction. (this is
//   counterintuitive since pitch usually refers to rotation above or below the
//   horizontal plane). asinf(y) is better to use.
// - no roll rotation (around the forward axis)
// - yaw assumes only right-handed coordinate system
inline math::Vector3Df g_getEulerAngleFrom(math::Vector3Df direction) {
  // TODO: consider to document that direction should be normalized
  // and remove the code
  direction = direction.normalized();

  // Assumes up vector (0, 1, 0) as the base rotation vector direction.
  return math::Vector3Df(
      acosf(direction.y()), atan2f(direction.x(), direction.z()), 0.0f);
}

inline math::Vector3Df g_getDirectionFromEulerAngle(
    const math::Vector3Df& eulerAngle) {
  // TODO: consider whether needed
  return math::g_transformVector(math::g_upVector<float, 3>(),
                                 math::g_rotateLh(eulerAngle));
}

// TODO: consider adding to math_library
const math::Vector4Df g_colorGreen = math::Vector4Df(0.0f, 1.0f, 0.0f, 1.0f);
const math::Vector4Df g_colorBlue  = math::Vector4Df(0.0f, 0.0f, 1.0f, 1.0f);
const math::Vector4Df g_colorWhite = math::Vector4Df(1.0f, 1.0f, 1.0f, 1.0f);
const math::Vector4Df g_colorBlack = math::Vector4Df(0.0f, 0.0f, 0.0f, 1.0f);

}  // namespace math

#endif  // GAME_ENGINE_MATH_UTIL_H