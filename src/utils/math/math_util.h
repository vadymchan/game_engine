#ifndef GAME_ENGINE_MATH_UTIL_H
#define GAME_ENGINE_MATH_UTIL_H

#include "config/config_manager.h"

#include <math_library/graphics.h>
#include <math_library/matrix.h>
#include <math_library/quaternion.h>
#include <math_library/vector.h>

// TODO: consider move this code to math_library third party
namespace math {

// TODO:
// - add functions like get matrix w/o scale/translate/rotate component
// - add functions like make scale + translate (and similar)

// ----------------------------------------------------------------------------

Vector3Df g_getVectorfromConfig(const game_engine::ConfigValue& value);

}  // namespace math

#endif  // GAME_ENGINE_MATH_UTIL_H
