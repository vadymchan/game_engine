#include "utils/math/math_util.h"

#include "utils/logger/global_logger.h"

namespace math {

Vector3Df g_getVectorfromConfig(const game_engine::ConfigValue& value) {
  Vector3Df result;

  result.x() = value["x"].GetFloat();
  result.y() = value["y"].GetFloat();
  result.z() = value["z"].GetFloat();

  return result;
}

}  // namespace math
