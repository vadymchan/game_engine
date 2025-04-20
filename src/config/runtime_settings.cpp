#include "config/runtime_settings.h"

#include "utils/logger/global_logger.h"

namespace game_engine {

using gfx::rhi::RenderingApi;

RuntimeSettings& RuntimeSettings::s_get() {
  static RuntimeSettings instance;
  return instance;
}

RuntimeSettings::RuntimeSettings() {
  updateFromConfig();
}

void RuntimeSettings::updateFromConfig() {
  auto configManager = ServiceLocator::s_get<ConfigManager>();
  auto config        = configManager->getConfig(PathManager::s_getDebugPath() / "config.json");

  if (config) {
    m_worldUp_            = config->get<math::Vector3Df>("worldUp");
    auto renderingApiText = config->get<std::string>("renderingApi");
    // TODO: make this more robust
    m_renderingApi_ = renderingApiText == "dx12"   ? RenderingApi::Dx12
                    : renderingApiText == "vulkan" ? RenderingApi::Vulkan
                                                   : RenderingApi::Dx12;
  } else {
    GlobalLogger::Log(LogLevel::Error,
                      std::string("Failed to initialize RuntimeSettings at ") + __FILE__ + ":"
                          + std::to_string(__LINE__) + " - config is nullptr.");
  }
}

const math::Vector3Df& RuntimeSettings::getWorldUp() const {
  return m_worldUp_;
}

RenderingApi RuntimeSettings::getRenderingApi() const {
  return m_renderingApi_;
}
}  // namespace game_engine
