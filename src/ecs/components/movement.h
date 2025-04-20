#ifndef GAME_ENGINE_MOVEMENT_H
#define GAME_ENGINE_MOVEMENT_H

#include <math_library/vector.h>

namespace game_engine {

struct Movement {
  math::Vector3Df direction;
  float           strength;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_MOVEMENT_H
