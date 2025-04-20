#ifndef GAME_ENGINE_I_UPDATABLE_SYSTEM_H
#define GAME_ENGINE_I_UPDATABLE_SYSTEM_H

#include "scene/scene.h"

namespace game_engine {

/**
 * TODO: Future system types can be added here:
 * - IReactiveSystem: For systems that respond to specific events
 * - IEventSystem: For systems that handle specific events or messages
 * - ILifecycleSystem: For systems that manage entity lifecycles
 **/

class IUpdatableSystem {
  public:
  virtual ~IUpdatableSystem() = default;

  // Scene is passed as a raw pointer since it's owned by the SceneManager
  virtual void update(Scene* scene, float deltaTime) = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_I_UPDATABLE_SYSTEM_H