#ifndef GAME_ENGINE_LIGHT_H
#define GAME_ENGINE_LIGHT_H

#include <math_library/vector.h>

namespace game_engine {

// TODO: consider adding a base class inside all light types(Directional, Point, Spot) for better cache locality
struct Light {
  math::Vector3Df color;
  float           intensity;
  bool            isDirty = true;
  bool            enabled = true;
};

struct DirectionalLight {
  math::Vector3Df direction;
  bool            isDirty = true;
};

struct PointLight {
  float range;
  bool  isDirty = true;
};

struct SpotLight {
  // This can be retrieved from the transform component
  // math::Vector3Df direction;
  float range;
  float innerConeAngle;
  float outerConeAngle;
  bool  isDirty = true;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_LIGHT_H
