#include "scene/scene_manager.h"

namespace game_engine {

void SceneManager::addScene(const std::string& name, Registry registry) {
  if (scenes_.find(name) == scenes_.end()) {
    auto scene = std::make_shared<Scene>();
    scene->setEntityRegistry(std::move(registry));
    scenes_[name] = scene;
  } else {
    // Log: Scene with this name already exists
  }
}

std::shared_ptr<Scene> SceneManager::getScene(const std::string& name) {
  auto it = scenes_.find(name);
  return it != scenes_.end() ? it->second : nullptr;
}

void SceneManager::removeScene(const std::string& name) {
  if (scenes_.erase(name) > 0) {
    if (currentSceneName_ == name) {
      currentSceneName_.clear();
      currentScene_.reset();
    }
  } else {
    // Log: Scene not found
  }
}

bool SceneManager::switchToScene(const std::string& name) {
  auto it = scenes_.find(name);
  if (it != scenes_.end()) {
    currentSceneName_ = name;
    currentScene_     = it->second;
    return true;
  } else {
    // Log: Scene not found
    return false;
  }
}

std::shared_ptr<Scene> SceneManager::getCurrentScene() const {
  return currentScene_.lock();
}

std::string SceneManager::getCurrentSceneName() const {
  return currentSceneName_;
}

bool SceneManager::hasScene(const std::string& name) const {
  return scenes_.find(name) != scenes_.end();
}

void SceneManager::clearAllScenes() {
  scenes_.clear();
  currentScene_.reset();
  currentSceneName_.clear();
}

}  // namespace game_engine
