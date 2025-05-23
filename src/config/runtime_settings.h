#ifndef GAME_ENGINE_RUNTIME_SETTINGS_H
#define GAME_ENGINE_RUNTIME_SETTINGS_H

#include "config/config_manager.h"
#include "gfx/rhi/common/rhi_enums.h"
#include "utils/path_manager/path_manager.h"
#include "utils/service/service_locator.h"

#include <math_library/vector.h>

namespace game_engine {

// TODO: move this to a separate file

class RuntimeSettings {
  public:
  static RuntimeSettings& s_get();

  RuntimeSettings(const RuntimeSettings&)            = delete;
  RuntimeSettings& operator=(const RuntimeSettings&) = delete;

  const math::Vector3Df& getWorldUp() const;

  void updateFromConfig();

  private:
  RuntimeSettings();
  math::Vector3Df        m_worldUp_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RUNTIME_SETTINGS_H
