#include "scene/scene_manager.h"

#include "utils/logger/global_logger.h"

namespace game_engine {

void SceneManager::addScene(const std::string& name, Registry registry) {
  if (scenes_.contains(name)) {
    GlobalLogger::Log(LogLevel::Warning, "Scene with name '" + name + "' already exists. Overwriting.");
  }

  scenes_[name] = std::make_unique<Scene>(std::move(registry));
}

Scene* SceneManager::getScene(const std::string& name) {
  auto it = scenes_.find(name);
  if (it != scenes_.end()) {
    return it->second.get();
  }
  return nullptr;
}

void SceneManager::removeScene(const std::string& name) {
  if (currentSceneName_ == name) {
    currentScene_     = nullptr;
    currentSceneName_ = "";
  }

  scenes_.erase(name);
}

bool SceneManager::switchToScene(const std::string& name) {
  auto it = scenes_.find(name);
  if (it != scenes_.end()) {
    currentScene_     = it->second.get();
    currentSceneName_ = name;
    return true;
  }

  GlobalLogger::Log(LogLevel::Error, "Failed to switch to scene '" + name + "'. Scene not found.");
  return false;
}

Scene* SceneManager::getCurrentScene() const {
  return currentScene_;
}

std::string SceneManager::getCurrentSceneName() const {
  return currentSceneName_;
}

bool SceneManager::hasScene(const std::string& name) const {
  return scenes_.contains(name);
}

void SceneManager::clearAllScenes() {
  currentScene_     = nullptr;
  currentSceneName_ = "";
  scenes_.clear();
}

}  // namespace game_engine
