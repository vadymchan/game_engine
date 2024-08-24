#ifndef GAME_ENGINE_PLANE_H
#define GAME_ENGINE_PLANE_H

#include "math_library/vector.h"

// TODO: consider move this code to math_library third party
namespace math {
// TODO: consider change from struct to class and make the fields private
struct Plane {
  Plane() = default;

  // TODO: write documentation that n is the normal of the plane (or consider
  // renaming it)
  Plane(float nX, float nY, float nZ, float distance)
      : m_n_(nX, nY, nZ)
      , m_d_(distance) {}

  Plane(math::Vector3Df normal, float distance)
      : m_n_(std::move(normal))
      , m_d_(distance) {}

  // TODO: use Point3Df instead of Vector3Df
  static Plane s_createFrustumFromThreePoints(const math::Vector3Df& p0,
                                              const math::Vector3Df& p1,
                                              const math::Vector3Df& p2) {
    const auto kDirection0 = p1 - p0;
    const auto kDirection1 = p2 - p0;
    auto       normal     = kDirection0.cross(kDirection1).normalized();
    const auto kDistance   = p2.dot(normal);
    return Plane(normal, kDistance);
  }

  // TODO: consider rename
  float dotProductWithNormal(const math::Vector3Df& direction) {
    return m_n_.dot(direction);
  }

  // TODO: consider rename + investigate math_library/graphics.h for similar
  // functionality
  float dotProductWithPosition(const math::Vector3Df& position) {
    return m_n_.x() * position.x() + m_n_.y() * position.y()
         + m_n_.z() * position.z() + m_d_;
  }

  // TODO: consider whether this is the acceptable name or not
  math::Vector3Df m_n_;
  float           m_d_;
};
}  // namespace math

#endif  // GAME_ENGINE_PLANE_H