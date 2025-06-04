#ifndef ARISE_MOVEMENT_H
#define ARISE_MOVEMENT_H

#include <math_library/vector.h>

namespace arise {

struct Movement {
  math::Vector3f direction;
  float           strength;
};

}  // namespace arise

#endif  // ARISE_MOVEMENT_H
