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
  Plane(float nx, float ny, float nz, float distance)
      : n(nx, ny, nz)
      , d(distance) {}

  Plane(math::Vector3Df normal, float distance)
      : n(std::move(normal))
      , d(distance) {}

  // TODO: use Point3Df instead of Vector3Df
  static Plane CreateFrustumFromThreePoints(const math::Vector3Df& p0,
                                            const math::Vector3Df& p1,
                                            const math::Vector3Df& p2) {
    const auto direction0 = p1 - p0;
    const auto direction1 = p2 - p0;
    auto       normal     = direction0.cross(direction1).normalized();
    const auto distance   = p2.dot(normal);
    return Plane(normal, distance);
  }

  float DotProductWithNormal(const math::Vector3Df& direction) {
    return n.dot(direction);
  }

  float DotProductWithPosition(const math::Vector3Df& position) {
    return n.x() * position.x() + n.y() * position.y() + n.z() * position.z()
         + d;
  }

  math::Vector3Df n;
  float           d;
};
}  // namespace math

#endif  // GAME_ENGINE_PLANE_H