#include "config/runtime_settings.h"

#include "utils/logger/global_logger.h"
#include "utils/math/math_util.h"

namespace arise {

using gfx::rhi::RenderingApi;

RuntimeSettings& RuntimeSettings::s_get() {
  static RuntimeSettings instance;
  return instance;
}

RuntimeSettings::RuntimeSettings() {
  auto configManager = ServiceLocator::s_get<ConfigManager>();
  auto configPath    = PathManager::s_getEngineSettingsPath() / "settings.json";
  auto config        = configManager->getConfig(configPath);
  if (!config) {
    configManager->addConfig(configPath);
    config = configManager->getConfig(configPath);
  }
  config->registerConverter<math::Vector3Df>(&math::g_getVectorfromConfig);
  updateFromConfig();
}

void RuntimeSettings::updateFromConfig() {
  auto configManager = ServiceLocator::s_get<ConfigManager>();
  auto config        = configManager->getConfig(PathManager::s_getEngineSettingsPath() / "settings.json");

  if (config) {
    m_worldUp_ = config->get<math::Vector3Df>("worldUp");
  } else {
    GlobalLogger::Log(LogLevel::Error, std::string("Failed to initialize RuntimeSettings at - config is nullptr."));
  }
}

const math::Vector3Df& RuntimeSettings::getWorldUp() const {
  return m_worldUp_;
}

}  // namespace arise
