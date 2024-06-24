#ifndef GAME_ENGINE_MATH_UTIL_H
#define GAME_ENGINE_MATH_UTIL_H

#include <math_library/graphics.h>
#include <math_library/matrix.h>
#include <math_library/vector.h>

// TODO: consider move this code to math_library third party
namespace math {
// TODO: remove (already implemented in math_library)

// static constexpr float PI = 3.141592653f;
//
// constexpr float RadianToDegree(float radian) {
//   constexpr float ToDegree = 180.0f / PI;
//   return radian * ToDegree;
// }
//
// constexpr float DegreeToRadian(float degree) {
//   constexpr float ToRadian = PI / 180.0f;
//   return degree * ToRadian;
// }

// clang-format off

// Create matrices
// ----------------------------------------------------------------------------

// TODO: remove (already implemented in math_library)

// N.B. : the functions is for matrices in row-major! 
//Matrix4f MakeTranslate(float x, float y, float z) {
//  return Matrix4f(
//    1.0f, 0.0f,	0.0f, 0.0f,
//    0.0f, 1.0f,	0.0f, 0.0f,
//    0.0f, 0.0f,	1.0f, 0.0f,
//    x,    y,	z,    1.0f);
//}
//
//Matrix4f MakeTranslate(Vector3Df const& vector) {
//  return MakeTranslate(vector.x(), vector.y(), vector.z());
//}
//
//// N.B. Rotation matrices is for right handed coordinate system!
//
//Matrix4f MakeRotateX(float fRadian) {
//  return Matrix4f(
//  	1.0f, 0.0f,				0.0f,			0.0f,
//  	0.0f, cosf(fRadian),	sinf(fRadian),	0.0f,
//  	0.0f, -sinf(fRadian),	cosf(fRadian),	0.0f,
//  	0.0f, 0.0f,				0.0f,			1.0f);
//}
//	
//Matrix4f MakeRotateY(float fRadian)
//{
//  return Matrix4f(
//  	cosf(fRadian),	0.0f,	-sinf(fRadian),	0.0f,
//  	0.0f,			1.0f,	0.0f,			0.0f,
//  	sinf(fRadian),	0.0f,	cosf(fRadian),	0.0f,
//  	0.0f,			0.0f,	0.0f,			1.0f);
//}
//
//Matrix4f MakeRotateZ(float fRadian) 
//{
//	return Matrix4f(
//		cosf(fRadian),	sinf(fRadian),	0.0f, 0.0f,
//		-sinf(fRadian),	cosf(fRadian),	0.0f, 0.0f,
//		0.0f,			0.0f,			1.0f, 0.0f,
//		0.0f,			0.0f,			0.0f, 1.0f);
//}
//
// Matrix4f MakeRotate(Vector3Df const& vector)
//{
//	return MakeRotate(vector.x(), vector.y(), vector.z());
//}
//
// Matrix4f MakeRotate(float fRadianX, float fRadianY, float fRadianZ) {
//  // Rotation Order : Z (Roll) -> X (Pitch) -> Y (Yaw)
//  float const SX = sinf(fRadianX);
//  float const CX = cosf(fRadianX);
//  float const SY = sinf(fRadianY);
//  float const CY = cosf(fRadianY);
//  float const SZ = sinf(fRadianZ);
//  float const CZ = cosf(fRadianZ);
//
//  return Matrix4f(
//     CY * CZ + SY * SX * SZ,   CX * SZ,  -SY * CZ + CY * SX * SZ,  0.0f,
//    -CY * SZ + SY * SX * CZ,   CX * CZ,   SY * SZ + CY * SX * CZ,  0.0f,
//     SY * CX,                 -SX,        CY * CX,                 0.0f,
//     0.0f,                     0.0f,      0.0f,                    1.0f);
//}
//
//
//
//Matrix4f MakeScale(float fX, float fY, float fZ) {
//  return Matrix4f(
//    fX,		0.0f,	0.0f,	0.0f,
//    0.0f,	fY,		0.0f,	0.0f,
//    0.0f,	0.0f,	fZ,		0.0f,
//    0.0f,	0.0f,	0.0f,	1.0f);
//}
//
//Matrix4f MakeScale(Vector3Df const& vector) {
//  MakeScale(vector.x(), vector.y(), vector.z());
//}

// TODO: 
// - add functions like get matrix w/o scale/translate/rotate component
// - add functions like make scale + translate (and similar)

// clang-format on

// ----------------------------------------------------------------------------

// TODO:
// - acosf(y) suggests measuring the angle from the 'up' direction. (this is
//   counterintuitive since pitch usually refers to rotation above or below the
//   horizontal plane). asinf(y) is better to use.
// - no roll rotation (around the forward axis)
// - yaw assumes only right-handed coordinate system
inline math::Vector3Df GetEulerAngleFrom(math::Vector3Df direction) {
  // TODO: consider to document that direction should be normalized
  // and remove the code
  direction = direction.normalized();

  // Assumes up vector (0, 1, 0) as the base rotation vector direction.
  return math::Vector3Df(
      acosf(direction.y()), atan2f(direction.x(), direction.z()), 0.0f);
}

inline math::Vector3Df GetDirectionFromEulerAngle(
    const math::Vector3Df& eulerAngle) {
  // TODO: consider whether needed
  return math::g_transformVector(math::g_upVector<float, 3>(),
                                 math::g_rotateLh(eulerAngle));
}

// TODO: consider adding to math_library
const math::Vector4Df ColorGreen = math::Vector4Df(0.0f, 1.0f, 0.0f, 1.0f);
const math::Vector4Df ColorBlue  = math::Vector4Df(0.0f, 0.0f, 1.0f, 1.0f);
const math::Vector4Df ColorWhite = math::Vector4Df(1.0f, 1.0f, 1.0f, 1.0f);
const math::Vector4Df ColorBlack = math::Vector4Df(0.0f, 0.0f, 0.0f, 1.0f);

}  // namespace math

#endif  // GAME_ENGINE_MATH_UTIL_H