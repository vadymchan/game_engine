#include "utils/path_manager/path_manager.h"

#include "config/config_manager.h"
#include "utils/service/service_locator.h"

#include <iostream>

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
      // TODO: use logger
      std::cout << "ConfigManager is not provided in ServiceLocator when "
                   "using PathManager. Adding it...\n";
      ServiceLocator::s_provide<ConfigManager>();
      configManager = ServiceLocator::s_get<ConfigManager>();
    }
    configManager->addConfig(std::string(s_configFile));

    s_config_ = configManager->getConfig(std::string(s_configFile));

    if (s_config_.expired()) {
      // TODO: use logger
      std::cerr << "Failed to load config!" << std::endl;
      return false;
    }
  }

  return true;
}

std::filesystem::path PathManager::s_getPath(std::string_view pathKey) {
  if (!s_isConfigAvailable()) {
    // TODO: use logger
    std::cerr << "Config is not available." << std::endl;
    return {};
  }

  if (auto config = s_config_.lock()) {
    return std::filesystem::path(
        config->get<std::string>(std::string(pathKey)));
  }

  return {};
}

}  // namespace game_engine
