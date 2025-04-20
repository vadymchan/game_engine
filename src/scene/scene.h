#ifndef GAME_ENGINE_SCENE_H
#define GAME_ENGINE_SCENE_H

#include <entt/entt.hpp>

namespace game_engine {

using Registry = entt::registry;

/**
 * @class Scene
 * @brief Manages all entities and components within the current scene.
 */
class Scene {
  public:
  Scene() = default;

  Scene(Registry registry);

  Registry& getEntityRegistry();

  const Registry& getEntityRegistry() const;

  void setEntityRegistry(Registry registry);

  private:
  /// Holds all entities (actors) and their components in the current scene.
  Registry entityRegistry_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SCENE_H
