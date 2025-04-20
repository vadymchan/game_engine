#ifndef GAME_ENGINE_CAMERA_H
#define GAME_ENGINE_CAMERA_H

#include <math_library/matrix.h>

namespace game_engine {

enum class CameraType {
  Perspective  = 0,
  Orthographic = 1
};

struct Camera {
  CameraType type;
  float      fov;
  float      nearClip;
  float      farClip;
  float      width;
  float      height;
};

// TODO: consider moving this to a separate file (don't expose it to the user -
// who will use engine for game)
struct CameraMatrices {
  math::Matrix4f<> view;
  math::Matrix4f<> projection;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_CAMERA_H
