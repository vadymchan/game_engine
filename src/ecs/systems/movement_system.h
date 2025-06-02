#ifndef ARISE_MOVEMENT_SYSTEM_H
#define ARISE_MOVEMENT_SYSTEM_H

#include "ecs/systems/i_updatable_system.h"

namespace arise {

// MovementSystem: Updates the Transform component based on the Movement
// component
class MovementSystem : public IUpdatableSystem {
  public:
  void update(Scene* scene, float deltaTime) override;
};

}  // namespace arise

#endif  // ARISE_MOVEMENT_SYSTEM_H