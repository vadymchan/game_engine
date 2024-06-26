#ifndef GAME_ENGINE_BOUND_PRIMITIVE_H
#define GAME_ENGINE_BOUND_PRIMITIVE_H

// TODO: 
// - consider move file to other directory
// - consider rename a file

#include <math_library/vector.h>

#include <vector>

namespace game_engine {

struct BoundBox {
  static BoundBox GenerateBoundBox(const std::vector<float>& vertices) {
    auto min = math::Vector3Df(FLT_MAX);
    auto max = math::Vector3Df(FLT_MIN);
    for (size_t i = 0; i < vertices.size() / 3; ++i) {
      auto curIndex = i * 3;
      auto x        = vertices[curIndex];
      auto y        = vertices[curIndex + 1];
      auto z        = vertices[curIndex + 2];
      if (max.x() < x) {
        max.x() = x;
      }
      if (max.y() < y) {
        max.y() = y;
      }
      if (max.z() < z) {
        max.z() = z;
      }

      if (min.x() > x) {
        min.x() = x;
      }
      if (min.y() > y) {
        min.y() = y;
      }
      if (min.z() > z) {
        min.z() = z;
      }
    }

    return {min, max};
  }

  void CreateBoundBox(const std::vector<float>& vertices) {
    *this = BoundBox::GenerateBoundBox(vertices);
  }

  math::Vector3Df GetExtent() const { return Max - Min; }

  math::Vector3Df GetHalfExtent() const { return GetExtent() / 0.5f; }

  math::Vector3Df Min;
  math::Vector3Df Max;
};

struct BoundSphere {
  float Radius = 0.0f;

  static BoundSphere GenerateBoundSphere(const std::vector<float>& vertices) {
    auto maxDist = FLT_MIN;
    for (size_t i = 0; i < vertices.size() / 3; ++i) {
      auto curIndex = i * 3;
      auto x        = vertices[curIndex];
      auto y        = vertices[curIndex + 1];
      auto z        = vertices[curIndex + 2];

      auto       currentPos = math::Vector3Df(x, y, z);
      const auto dist       = currentPos.magnitude();
      if (maxDist < dist) {
        maxDist = dist;
      }
    }
    return {maxDist};
  }

  void CreateBoundSphere(const std::vector<float>& vertices) {
    *this = BoundSphere::GenerateBoundSphere(vertices);
  }
};

}  // namespace game_engine

#endif  // GAME_ENGINE_BOUND_PRIMITIVE_H