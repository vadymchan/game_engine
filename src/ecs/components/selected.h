#ifndef GAME_ENGINE_SELECTED_H
#define GAME_ENGINE_SELECTED_H

#include <math_library/vector.h>

namespace game_engine {

struct Selected {
  math::Vector4Df highlightColor   = math::Vector4Df(1.0f, 0.5f, 0.0f, 1.0f);  // Orange
  float           outlineThickness = 0.02f;
  bool            xRay             = false;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SELECTED_H