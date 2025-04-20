#include "ecs/systems/system_manager.h"

namespace game_engine {

void SystemManager::addSystem(std::unique_ptr<IUpdatableSystem> system) {
  m_systems_.push_back(std::move(system));
}

void SystemManager::updateSystems(Scene* scene, float deltaTime) {
  for (const auto& system : m_systems_) {
    system->update(scene, deltaTime);
  }
}

}  // namespace game_engine
