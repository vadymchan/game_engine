#ifndef GAME_ENGINE_VERTEX_H
#define GAME_ENGINE_VERTEX_H

#include <math_library/vector.h>

namespace game_engine {

struct Vertex {
  math::Vector3Df position;
  math::Vector2Df texCoords;
  math::Vector3Df normal;
  math::Vector3Df tangent;
  math::Vector3Df bitangent;
  math::Vector4Df color;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_VERTEX_H
