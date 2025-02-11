#include "utils/path_manager/path_manager.h"

#include "config/config_manager.h"
#include "utils/logger/global_logger.h"
#include "utils/service/service_locator.h"

namespace game_engine {

std::filesystem::path PathManager::s_getAssetPath() {
  return s_getPath(s_assetPath);
}

std::filesystem::path PathManager::s_getShaderPath() {
  return s_getPath(s_shaderPath);
}

std::filesystem::path PathManager::s_getDebugPath() {
  return s_getPath(s_debugPath);
}

bool PathManager::s_isConfigAvailable() {
  // if weak_ptr is expired, we need to load the config (either initialization
  // or reload)
  if (s_config_.expired()) {
    auto configManager = ServiceLocator::s_get<ConfigManager>();
    if (!configManager) {
      GlobalLogger::Log(LogLevel::Warning,
                        "ConfigManager is not provided in ServiceLocator when "
                        "using PathManager. Adding it...");
      ServiceLocator::s_provide<ConfigManager>();
      configManager = ServiceLocator::s_get<ConfigManager>();
    }
    configManager->addConfig(std::string(s_configFile));

    s_config_ = configManager->getConfig(std::string(s_configFile));

    if (s_config_.expired()) {
      GlobalLogger::Log(LogLevel::Error, "Failed to load config!");
      return false;
    }
  }

  return true;
}

std::filesystem::path PathManager::s_getPath(std::string_view pathKey) {
  if (!s_isConfigAvailable()) {
    GlobalLogger::Log(LogLevel::Error, "Config is not available.");
    return {};
  }

  if (auto config = s_config_.lock()) {
    return std::filesystem::path(
        config->get<std::string>(std::string(pathKey)));
  }

  return {};
}

}  // namespace game_engine
