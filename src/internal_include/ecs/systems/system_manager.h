#ifndef GAME_ENGINE_SYSTEM_MANAGER_H
#define GAME_ENGINE_SYSTEM_MANAGER_H

#include "ecs/systems/i_updatable_system.h"

namespace game_engine {

// TODO: In future, when other than UpdatableSystems will be introduced,
// consider to separate SystemManager to SubSystems like UpdatableSystemManager,
// ReactiveSystemManager, EventSystemManager, LifecycleSystemManager etc.

/**
 * @class SystemManager
 * @brief Manages all systems within the engine.
 *
 * @details
 * - Users should avoid adding duplicate systems.
 * - Systems should not overlap in functionality to prevent unintended behavior.
 */
class SystemManager {
  public:
  void addSystem(const std::shared_ptr<IUpdatableSystem>& system);

  // TODO: add removeSystem method

  void updateSystems(const std::shared_ptr<Scene>& scene, float deltaTime);

  private:
  std::vector<std::shared_ptr<IUpdatableSystem>> m_systems_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SYSTEM_MANAGER_H
