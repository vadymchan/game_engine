#ifndef GAME_ENGINE_BOUND_PRIMITIVE_H
#define GAME_ENGINE_BOUND_PRIMITIVE_H

// TODO:
// - consider move file to other directory
// - consider rename a file

#include <math_library/vector.h>

#include <vector>

namespace game_engine {

struct BoundBox {
  // ======= BEGIN: public static methods =====================================

  static BoundBox s_generateBoundBox(const std::vector<float>& vertices) {
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

  // ======= END: public static methods   =====================================

  // ======= BEGIN: public getters ============================================

  math::Vector3Df getExtent() const { return m_max_ - m_min_; }

  math::Vector3Df getHalfExtent() const { return getExtent() / 0.5f; }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  void createBoundBox(const std::vector<float>& vertices) {
    *this = BoundBox::s_generateBoundBox(vertices);
  }

  // ======= END: public misc methods   =======================================



  // ======= BEGIN: public misc fields ========================================

  math::Vector3Df m_min_;
  math::Vector3Df m_max_;

  // ======= END: public misc fields   ========================================
};

struct BoundSphere {
  // ======= BEGIN: public static methods =====================================

  static BoundSphere s_generateBoundSphere(const std::vector<float>& vertices) {
    auto maxDist = FLT_MIN;
    for (size_t i = 0; i < vertices.size() / 3; ++i) {
      auto curIndex = i * 3;
      auto x        = vertices[curIndex];
      auto y        = vertices[curIndex + 1];
      auto z        = vertices[curIndex + 2];

      auto       currentPos = math::Vector3Df(x, y, z);
      // TODO: consider better naming
      const auto kDist = currentPos.magnitude();
      if (maxDist < kDist) {
        maxDist = kDist;
      }
    }
    return {maxDist};
  }

  // ======= END: public static methods   =====================================

  // ======= BEGIN: public misc methods =======================================

  void createBoundSphere(const std::vector<float>& vertices) {
    *this = BoundSphere::s_generateBoundSphere(vertices);
  }

  // ======= END: public misc methods   =======================================


  // ======= BEGIN: public misc fields ========================================

  float m_radius_ = 0.0f;

  // ======= END: public misc fields   ========================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_BOUND_PRIMITIVE_H