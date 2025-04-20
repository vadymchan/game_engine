#ifndef GAME_ENGINE_SCENE_MANAGER_H
#define GAME_ENGINE_SCENE_MANAGER_H

#include "scene/scene.h"

namespace game_engine {

class SceneManager {
  public:
  SceneManager() = default;

  void addScene(const std::string& name, Registry registry);

  // Retrieves a scene by name, useful for setting up a scene before switching
  Scene* getScene(const std::string& name);

  void removeScene(const std::string& name);

  bool switchToScene(const std::string& name);

  Scene* getCurrentScene() const;

  std::string getCurrentSceneName() const;

  bool hasScene(const std::string& name) const;

  void clearAllScenes();

  private:
  std::unordered_map<std::string, std::unique_ptr<Scene>> scenes_;
  Scene*                                                  currentScene_ = nullptr;
  std::string                                             currentSceneName_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SCENE_MANAGER_H
