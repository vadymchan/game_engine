#ifndef GAME_ENGINE_RENDER_SYSTEM_H
#define GAME_ENGINE_RENDER_SYSTEM_H

#include "ecs/systems/i_updatable_system.h"

namespace game_engine {

class RenderSystem : public IUpdatableSystem {
  public:
  void update(const std::shared_ptr<Scene>& scene, float deltaTime) override;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_SYSTEM_H
