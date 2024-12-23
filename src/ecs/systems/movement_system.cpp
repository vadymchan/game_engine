#include "ecs/systems/movement_system.h"

#include "ecs/components/movement.h"
#include "ecs/components/transform.h"

#include <iostream>  // TODO: Remove this

namespace game_engine {

void MovementSystem::update(const std::shared_ptr<Scene>& scene,
                            float                         deltaTime) {
  Registry& registry = scene->getEntityRegistry();
  auto      view     = registry.view<Transform, Movement>();

  for (auto entity : view) {
    auto& transform = view.get<Transform>(entity);
    auto& movement  = view.get<Movement>(entity);

    // Update translation based on direction, strength, and deltaTime
    transform.translation += movement.direction * movement.strength * deltaTime;

    movement = Movement{};  // Reset movement

    // std::cout << "Moved Entity " << int(entity) << " to Position: ("
    //           << transform.translation << ")\n";
  }
}

}  // namespace game_engine
