#ifndef GAME_ENGINE_LIGHT_H
#define GAME_ENGINE_LIGHT_H

#include <math_library/vector.h>

namespace game_engine {

// This is common data for all types of lights (directional, point, spot)
struct Light {
  math::Vector3Df color;
  float           intensity;
};

struct DirectionalLight {
  math::Vector3Df direction;
};

struct PointLight {
  float range;
};

struct SpotLight {
  // This can be retrieved from the transform component
  // math::Vector3Df direction;
  float range;
  float innerConeAngle;
  float outerConeAngle;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_LIGHT_H
