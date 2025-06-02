#ifndef ARISE_VERTEX_H
#define ARISE_VERTEX_H

#include <math_library/vector.h>

namespace arise {

struct Vertex {
  math::Vector3Df position;
  math::Vector2Df texCoords;
  math::Vector3Df normal;
  math::Vector3Df tangent;
  math::Vector3Df bitangent;
  math::Vector4Df color;
};

}  // namespace arise

#endif  // ARISE_VERTEX_H
