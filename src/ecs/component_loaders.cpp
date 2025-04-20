#include "ecs/component_loaders.h"

#include "config/config_manager.h"
#include "ecs/components/camera.h"
#include "ecs/components/transform.h"
#include "utils/logger/global_logger.h"
#include "utils/path_manager/path_manager.h"
#include "utils/service/service_locator.h"

namespace game_engine {

Transform LoadTransform(const ConfigValue& value) {
  Transform transform;

  const auto& transformValue = value["transform"];

  auto position = transformValue["position"].GetArray();
  auto rotation = transformValue["rotation"].GetArray();
  auto scale    = transformValue["scale"].GetArray();

  for (size_t i = 0; i < position.Size(); i++) {
    transform.translation.coeffRef(i) = position[i].GetFloat();
  }

  for (size_t i = 0; i < rotation.Size(); i++) {
    transform.rotation.coeffRef(i) = rotation[i].GetFloat();
  }

  for (size_t i = 0; i < scale.Size(); i++) {
    transform.scale.coeffRef(i) = scale[i].GetFloat();
  }

  return transform;
}

Camera LoadCamera(const ConfigValue& value) {
  Camera camera;

  auto& cameraParameters = value["parameters"];

  camera.fov      = cameraParameters["fov"].GetFloat();
  camera.nearClip = cameraParameters["near"].GetFloat();
  camera.farClip  = cameraParameters["far"].GetFloat();
  camera.type     = static_cast<CameraType>(cameraParameters["type"].GetInt());
  camera.width    = cameraParameters["width"].GetFloat();
  camera.height   = cameraParameters["height"].GetFloat();

  return camera;
}

void ApplyCameraComponents(Registry& registry, Entity cameraEntity) {
  auto configManager = ServiceLocator::s_get<ConfigManager>();
  auto config
      = configManager->getConfig(PathManager::s_getDebugPath() / "config.json");

  auto transform = config->get<Transform>("camera");
  registry.emplace_or_replace<Transform>(cameraEntity, transform);

  auto camera = config->get<Camera>("camera");
  registry.emplace_or_replace<Camera>(cameraEntity, camera);
}

Entity LoadCameraFromConfig(Registry& registry,
                            Entity    cameraEntity /*= entt::null*/) {
  if (!registry.valid(cameraEntity)) {
    if (cameraEntity != entt::null) {
      GlobalLogger::Log(
          LogLevel::Warning,
          "Provided camera entity is invalid. Creating a new camera entity.");
    }
    cameraEntity = registry.create();
  }

  ApplyCameraComponents(registry, cameraEntity);

  return cameraEntity;
}

}  // namespace game_engine
