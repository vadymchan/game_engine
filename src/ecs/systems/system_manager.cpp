#include "ecs/systems/system_manager.h"

namespace game_engine {

void SystemManager::addSystem(const std::shared_ptr<IUpdatableSystem>& system) {
  m_systems_.push_back(system);
}

void SystemManager::updateSystems(const std::shared_ptr<Scene>& scene,
                                  float                         deltaTime) {
  for (const auto& system : m_systems_) {
    system->update(scene, deltaTime);
  }
}

}  // namespace game_engine
