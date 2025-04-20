#ifndef GAME_ENGINE_MOVEMENT_SYSTEM_H
#define GAME_ENGINE_MOVEMENT_SYSTEM_H

#include "ecs/systems/i_updatable_system.h"

namespace game_engine {

// MovementSystem: Updates the Transform component based on the Movement
// component
class MovementSystem : public IUpdatableSystem {
  public:
  void update(Scene* scene, float deltaTime) override;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_MOVEMENT_SYSTEM_H