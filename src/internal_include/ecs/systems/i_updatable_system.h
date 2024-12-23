#ifndef GAME_ENGINE_I_UPDATABLE_SYSTEM_H
#define GAME_ENGINE_I_UPDATABLE_SYSTEM_H

#include "scene/scene.h"

#include <memory>

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

  virtual void update(const std::shared_ptr<Scene>& scene, float deltaTime) = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_I_UPDATABLE_SYSTEM_H
