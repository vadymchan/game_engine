#ifndef GAME_ENGINE_MATH_UTIL_H
#define GAME_ENGINE_MATH_UTIL_H

#include "config/config_manager.h"

#include <math_library/graphics.h>
#include <math_library/matrix.h>
#include <math_library/quaternion.h>
#include <math_library/vector.h>

// TODO: consider move this code to math_library third party
namespace math {

// TODO:
// - add functions like get matrix w/o scale/translate/rotate component
// - add functions like make scale + translate (and similar)

// ----------------------------------------------------------------------------

Vector3Df g_getVectorfromConfig(const game_engine::ConfigValue& value);

inline Quaternionf g_getQuaternionfromConfig(
    const game_engine::ConfigValue& value) {
  Quaternionf result;
  result.x() = value["x"].GetFloat();
  result.y() = value["y"].GetFloat();
  result.z() = value["z"].GetFloat();
  result.w() = value["w"].GetFloat();
  return result;
}

// TODO:
// - acosf(y) suggests measuring the angle from the 'up' direction. (this is
//   counterintuitive since pitch usually refers to rotation above or below
//   the horizontal plane). asinf(y) is better to use.
// - no roll rotation (around the forward axis)
// - yaw assumes only right-handed coordinate system
inline Vector3Df g_getEulerAngleFrom(Vector3Df direction) {
  // TODO: consider to document that direction should be normalized
  // and remove the code
  direction = direction.normalized();

  // Assumes up vector (0, 1, 0) as the base rotation vector direction.
  return Vector3Df(
      acosf(direction.y()), atan2f(direction.x(), direction.z()), 0.0f);
}

inline Vector3Df g_getDirectionFromEulerAngle(const Vector3Df& eulerAngle) {
  // TODO: consider whether needed
  return g_transformVector(g_upVector<float, 3>(), g_rotateLh(eulerAngle));
}

// TODO: consider adding to math_library
const Vector4Df g_kColorBlack = Vector4Df(0.0f, 0.0f, 0.0f, 1.0f);
const Vector4Df g_kColorRed   = Vector4Df(1.0f, 0.0f, 0.0f, 1.0f);
const Vector4Df g_kColorGreen = Vector4Df(0.0f, 1.0f, 0.0f, 1.0f);
const Vector4Df g_kColorBlue  = Vector4Df(0.0f, 0.0f, 1.0f, 1.0f);
const Vector4Df g_kColorWhite = Vector4Df(1.0f, 1.0f, 1.0f, 1.0f);

}  // namespace math

#endif  // GAME_ENGINE_MATH_UTIL_H
