#ifndef GAME_ENGINE_PATH_MANAGER_H
#define GAME_ENGINE_PATH_MANAGER_H

#include "config/config.h"

#include <filesystem>

namespace game_engine {

class PathManager {
  public:
  // TODO: consider whether to use s_ here (and for other managers)
  static std::filesystem::path s_getAssetPath();

  static std::filesystem::path s_getShaderPath();

  static std::filesystem::path s_getDebugPath();

  private:
  static constexpr std::string_view s_assetPath  = "assetPath";
  static constexpr std::string_view s_shaderPath = "shaderPath";
  static constexpr std::string_view s_debugPath  = "debugPath";
  // TODO: currently hardcoded, consider changing to dynamic (but this may
  // introduce coupling through shared state)
  static constexpr std::string_view s_configFile
      = "config/directories/config.json";

  static inline Config* s_config_;

  static bool s_isConfigAvailable();

  static std::filesystem::path s_getPath(std::string_view pathKey);
};

}  // namespace game_engine

#endif  // GAME_ENGINE_PATH_MANAGER_H
