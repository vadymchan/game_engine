#include "scene/scene.h"

namespace game_engine {

Scene::Scene(Registry registry)
    : entityRegistry_(std::move(registry)) {
}

Registry& Scene::getEntityRegistry() {
  return entityRegistry_;
}

const Registry& Scene::getEntityRegistry() const {
  return entityRegistry_;
}

void Scene::setEntityRegistry(Registry registry) {
  entityRegistry_ = std::move(registry);
}

}  // namespace game_engine
