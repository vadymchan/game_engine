#ifndef GAME_ENGINE_SCENE_LOADER_H
#define GAME_ENGINE_SCENE_LOADER_H

#include "config/config.h"
#include "ecs/entity.h"
#include "scene/scene.h"
#include "scene/scene_manager.h"

#include <filesystem>

namespace game_engine {

class SceneLoader {
  public:
  static Scene* loadScene(const std::string& sceneName, SceneManager* sceneManager);

  static Scene* loadSceneFromFile(const std::filesystem::path& configPath,
                                  SceneManager*                sceneManager,
                                  const std::string&           customSceneName = "");
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SCENE_LOADER_H