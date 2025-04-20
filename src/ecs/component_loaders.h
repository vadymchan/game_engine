#ifndef GAME_ENGINE_COMPONENT_LOADERS_H
#define GAME_ENGINE_COMPONENT_LOADERS_H

#include "config/config.h"
#include "ecs/entity.h"
#include "scene/scene.h"

namespace game_engine {

struct Transform;
struct Camera;

Transform LoadTransform(const ConfigValue& value);

Camera LoadCamera(const ConfigValue& value);

void ApplyCameraComponents(Registry& registry, Entity cameraEntity);

Entity LoadCameraFromConfig(Registry& registry,
                            Entity    cameraEntity = entt::null);

}  // namespace game_engine

#endif  // GAME_ENGINE_COMPONENT_LOADERS_H
