
#include "game.h"

#include "config/config_manager.h"
#include "ecs/component_loaders.h"
#include "engine.h"
#include "input/input_manager.h"
#include "utils/hot_reload/hot_reload_manager.h"
#include "utils/path_manager/path_manager.h"
#include "utils/service/service_locator.h"

#include <ecs/components/movement.h>

#include <random>

namespace game_engine {

void Game::setup() {
  GlobalLogger::Log(LogLevel::Info, "Game::setup() started");

  // TODO: consider change to std::mt19937
  srand(static_cast<uint32_t>(time(NULL)));

  // TODO: consider moving to another place (like initialize) if setup is called more
  {
    auto keyboardEventHandler = ServiceLocator::s_get<InputManager>()->getKeyboardHandler();

    // Subscribe to key press events to set action states
    keyboardEventHandler->subscribe({SDL_KEYDOWN, SDL_SCANCODE_W}, [this](const KeyboardEvent& event) {
      m_actionStates_.set(static_cast<size_t>(Action::MoveForward), true);
    });

    keyboardEventHandler->subscribe({SDL_KEYDOWN, SDL_SCANCODE_S}, [this](const KeyboardEvent& event) {
      m_actionStates_.set(static_cast<size_t>(Action::MoveBackward), true);
    });

    keyboardEventHandler->subscribe({SDL_KEYDOWN, SDL_SCANCODE_A}, [this](const KeyboardEvent& event) {
      m_actionStates_.set(static_cast<size_t>(Action::MoveLeft), true);
    });

    keyboardEventHandler->subscribe({SDL_KEYDOWN, SDL_SCANCODE_D}, [this](const KeyboardEvent& event) {
      m_actionStates_.set(static_cast<size_t>(Action::MoveRight), true);
    });

    keyboardEventHandler->subscribe({SDL_KEYDOWN, SDL_SCANCODE_SPACE}, [this](const KeyboardEvent& event) {
      m_actionStates_.set(static_cast<size_t>(Action::MoveUp), true);
    });

    keyboardEventHandler->subscribe({SDL_KEYDOWN, SDL_SCANCODE_LCTRL}, [this](const KeyboardEvent& event) {
      m_actionStates_.set(static_cast<size_t>(Action::MoveDown), true);
    });

    // Subscribe to key release events to reset action states
    keyboardEventHandler->subscribe({SDL_KEYUP, SDL_SCANCODE_W}, [this](const KeyboardEvent& event) {
      m_actionStates_.set(static_cast<size_t>(Action::MoveForward), false);
    });

    keyboardEventHandler->subscribe({SDL_KEYUP, SDL_SCANCODE_S}, [this](const KeyboardEvent& event) {
      m_actionStates_.set(static_cast<size_t>(Action::MoveBackward), false);
    });

    keyboardEventHandler->subscribe({SDL_KEYUP, SDL_SCANCODE_A}, [this](const KeyboardEvent& event) {
      m_actionStates_.set(static_cast<size_t>(Action::MoveLeft), false);
    });

    keyboardEventHandler->subscribe({SDL_KEYUP, SDL_SCANCODE_D}, [this](const KeyboardEvent& event) {
      m_actionStates_.set(static_cast<size_t>(Action::MoveRight), false);
    });

    keyboardEventHandler->subscribe({SDL_KEYUP, SDL_SCANCODE_SPACE}, [this](const KeyboardEvent& event) {
      m_actionStates_.set(static_cast<size_t>(Action::MoveUp), false);
    });

    keyboardEventHandler->subscribe({SDL_KEYUP, SDL_SCANCODE_LCTRL}, [this](const KeyboardEvent& event) {
      m_actionStates_.set(static_cast<size_t>(Action::MoveDown), false);
    });

    auto mouseEventHandler = ServiceLocator::s_get<InputManager>()->getMouseHandler();

    mouseEventHandler->subscribe({SDL_MOUSEWHEEL}, [this](const MouseWheelEvent& event) {
      if (event.y > 0) {
        m_movingSpeed_ += s_speedChangeStep;
      } else if (event.y < 0) {
        m_movingSpeed_ -= s_speedChangeStep;
      }

      m_movingSpeed_ = std::clamp(m_movingSpeed_, s_minMovingSpeed, 100.0f);
    });

    mouseEventHandler->subscribe({SDL_MOUSEMOTION},
                                 [this](const MouseMotionEvent& event) { this->onMouseMove(event.xrel, event.yrel); });

    mouseEventHandler->subscribe({SDL_MOUSEBUTTONDOWN, SDL_BUTTON_RIGHT}, [this](const MouseButtonEvent& event) {
      m_isRightMouseButtonPressed_ = true;
      SDL_SetRelativeMouseMode(SDL_TRUE);
    });

    mouseEventHandler->subscribe({SDL_MOUSEBUTTONUP, SDL_BUTTON_RIGHT}, [this](const MouseButtonEvent& event) {
      m_isRightMouseButtonPressed_ = false;
      SDL_SetRelativeMouseMode(SDL_FALSE);
    });
  }

  // TODO: consider better place for this
  {
    // auto hotReloadManager = ServiceLocator::s_get<HotReloadManager>();
    // auto configManager    = ServiceLocator::s_get<ConfigManager>();
    // auto config           = configManager
    //                   ->getConfig(PathManager::s_getDebugPath() /
    //                   "config.json") ;

    // config->registerConverter<math::Vector3Df>(&math::g_getVectorfromConfig);

    // auto cameraParameters =
    // config->get<CameraParametersDeprecated>("camera");
    // cameraParameters.m_width  = (float)m_window_->getSize().width();
    // cameraParameters.m_height = (float)m_window_->getSize().height();
    // auto worldUp              = config->get<math::Vector3Df>("worldUp");

    // m_mainCamera_ = std::make_shared<CameraOld>(cameraParameters.m_position,
    //                                             cameraParameters.m_direction,
    //                                             cameraParameters.m_orientation,
    //                                             worldUp,
    //                                             cameraParameters.m_fov,
    //                                             cameraParameters.m_near,
    //                                             cameraParameters.m_far,
    //                                             cameraParameters.m_width,
    //                                             cameraParameters.m_height,
    //                                             cameraParameters.m_type);

    // auto fileModificationHandler = [mainCamera = m_mainCamera_,
    //                                 window = m_window_](const wtr::event& e)
    //                                 {
    //   auto configManager = ServiceLocator::s_get<ConfigManager>();
    //   auto config
    //       = configManager
    //             ->getConfig(PathManager::s_getDebugPath() / "config.json")
    //             ;
    //   if (config) {
    //     // TODO: consider moving to a more appropriate place (currently we
    //     will
    //     // call reload for each callback). But it's better to call it only
    //     once
    //     // the file is modified
    //     config->reloadAsync();
    //   }

    //  if (mainCamera) {
    //    mainCamera->updateFromConfig();
    //    mainCamera->m_width_  = (float)window->getSize().width();
    //    mainCamera->m_height_ = (float)window->getSize().height();
    //  }
    //};

    // hotReloadManager->watchFileModifications(PathManager::s_getDebugPath(),
    //                                          fileModificationHandler);
  }

  // Select spawning object type
  // spawnObjects(SpawnedType::TestPrimitive);

  Registry registry;

  // camera
  {
    Entity cameraEntity = LoadCameraFromConfig(registry);
    registry.emplace<Movement>(cameraEntity);

    auto& camera  = registry.get<Camera>(cameraEntity);
    camera.width  = m_window_->getSize().width();
    camera.height = m_window_->getSize().height();

    // hot reload for camera
    {
      auto fileModificationHandler = [window = m_window_](const wtr::event& e) {
        auto configManager = ServiceLocator::s_get<ConfigManager>();
        auto config        = configManager->getConfig(PathManager::s_getDebugPath() / "config.json");
        if (config) {
          // TODO: consider moving to a more appropriate place (currently we
          // will call reload for each callback). But it's better to call it
          // only once the file is modified
          config->reloadAsync();
        }

        auto  sceneManager = ServiceLocator::s_get<SceneManager>();
        auto& registry     = sceneManager->getCurrentScene()->getEntityRegistry();

        auto cameraEntity = registry.view<Camera>().front();

        if (cameraEntity == entt::null) {
          GlobalLogger::Log(LogLevel::Error, "File modification handler: Could not find a camera entity.");
        }

        LoadCameraFromConfig(registry, cameraEntity);

        window->getSize().width();
        window->getSize().height();
      };

      auto hotReloadManager = ServiceLocator::s_get<HotReloadManager>();
      hotReloadManager->watchFileModifications(PathManager::s_getDebugPath(), fileModificationHandler);
    }
  }

  // cube
  {
    auto      cube = registry.create();
    Transform cubeTransform;
    cubeTransform.translation = math::Vector3Df(0.0f, 0.0f, -7.0f);
    cubeTransform.scale       = math::Vector3Df(1.0f, 1.0f, 1.0f);
    cubeTransform.rotation    = math::Vector3Df(0.0f, 0.0f, 0.0f);

    auto modelManager = ServiceLocator::s_get<RenderModelManager>();

    // TODO: use config file for file path
    auto cubeModel = modelManager->getRenderModel("assets/models/cube/cube.fbx");

    registry.emplace<Transform>(cube, cubeTransform);
    registry.emplace<RenderModel*>(cube, cubeModel);
    registry.emplace<Movement>(cube);
  }

  //// cubes
  //{
  //  std::random_device rd;
  //  std::mt19937       gen(rd());

  //  std::uniform_real_distribution<float> dist(-10.0f, 10.0f);

  //  const int NUM_CUBES = 100;

  //  auto modelManager = ServiceLocator::s_get<RenderModelManager>();

  //  auto cubeModel
  //      = modelManager->getRenderModel("assets/models/cube/cube.fbx");

  //  for (int i = 0; i < NUM_CUBES; ++i) {
  //    auto cube = registry.create();

  //    Transform cubeTransform;

  //    float randomX = dist(gen);
  //    float randomY = dist(gen);
  //    float randomZ = dist(gen);

  //    cubeTransform.translation = math::Vector3Df(randomX, randomY, randomZ);
  //    cubeTransform.scale       = math::Vector3Df(1.0f, 1.0f, 1.0f);
  //    cubeTransform.rotation    = math::Vector3Df(0.0f, 0.0f, 0.0f);

  //    registry.emplace<Transform>(cube, cubeTransform);
  //    registry.emplace<std::shared_ptr<RenderModel>>(cube, cubeModel);
  //    registry.emplace<Movement>(cube);
  //  }
  //}

  // lights
  {
    Light directionalLight;
    directionalLight.color     = math::Vector3Df(0.11f, 0.12f, 0.13f);
    directionalLight.intensity = 0.14f;
    DirectionalLight directionalLightData;
    directionalLightData.direction = math::Vector3Df(0.15f, -0.16f, 0.17f);

    Light pointLight;
    pointLight.color     = math::Vector3Df(0.21f, 0.22f, 0.23f);
    pointLight.intensity = 0.24f;
    PointLight pointLightData;
    pointLightData.range = 0.25f;
    Transform pointLightTransform;
    pointLightTransform.translation = math::Vector3Df(0.26f, 0.27f, 0.28f);

    Light spotLight;
    spotLight.color     = math::Vector3Df(0.31f, 0.32f, 0.33f);
    spotLight.intensity = 0.34f;
    SpotLight spotLightData;
    spotLightData.range = 0.35f;
    // spotLightData.innerConeAngle = math::g_degreeToRadian(30.0f);
    // spotLightData.outerConeAngle = math::g_degreeToRadian(45.0f);
    spotLightData.innerConeAngle = 0.36f;
    spotLightData.outerConeAngle = 0.37f;
    Transform spotLightTransform;
    spotLightTransform.translation = math::Vector3Df(0.38f, 0.39f, 0.310f);
    spotLightTransform.rotation    = math::Vector3Df(0.311f, 0.312f, 0.313f);

    auto directionalLightEntity = registry.create();
    registry.emplace<Light>(directionalLightEntity, directionalLight);
    registry.emplace<DirectionalLight>(directionalLightEntity, directionalLightData);

    auto pointLightEntity = registry.create();
    registry.emplace<Light>(pointLightEntity, pointLight);
    registry.emplace<PointLight>(pointLightEntity, pointLightData);
    registry.emplace<Transform>(pointLightEntity, pointLightTransform);

    auto spotLightEntity = registry.create();
    registry.emplace<Light>(spotLightEntity, spotLight);
    registry.emplace<SpotLight>(spotLightEntity, spotLightData);
    registry.emplace<Transform>(spotLightEntity, spotLightTransform);
  }

  const auto sceneName    = "test_scene";
  auto       sceneManager = ServiceLocator::s_get<SceneManager>();
  sceneManager->addScene(sceneName, std::move(registry));
  sceneManager->switchToScene(sceneName);
  m_scene_ = sceneManager->getCurrentScene();

  GlobalLogger::Log(LogLevel::Info, "Game::setup() completed");
}

void Game::onMouseMove(int32_t xOffset, int32_t yOffset) {
  if (!m_isRightMouseButtonPressed_) {
    return;
  }

  // TODO: promote to a member variable for configuration
  float xOffsetFloat = static_cast<float>(xOffset) * m_mouseSensitivity_;
  float yOffsetFloat = static_cast<float>(yOffset) * m_mouseSensitivity_;

  m_yaw_   += xOffsetFloat;
  m_pitch_ += yOffsetFloat;

  // TODO: remove magic numbers
  m_pitch_ = std::clamp(m_pitch_, -89.0f, 89.0f);

  auto& registry     = m_scene_->getEntityRegistry();
  auto  cameraEntity = registry.view<Camera, Transform>().front();

  if (cameraEntity == entt::null) {
    GlobalLogger::Log(LogLevel::Error, "Camera entity is null.");
    return;
  }

  auto& transform = registry.get<Transform>(cameraEntity);

  transform.rotation.x() = m_pitch_;
  transform.rotation.y() = m_yaw_;

  // float yawRadians   = math::g_degreeToRadian(m_yaw_);
  // float pitchRadians = math::g_degreeToRadian(m_pitch_);

  // auto rotationQuaternion = math::Quaternionf::fromEulerAngles(
  //     0, pitchRadians, yawRadians, math::EulerRotationOrder::ZXY);

  //// TODO: add getter / setter for camera orientation
  // m_mainCamera_->setOrientation(rotationQuaternion);
}

void Game::update(float deltaTime) {
  // TODO: move to a separate function
  {
    auto& registry     = m_scene_->getEntityRegistry();
    auto  cameraEntity = registry.view<Movement, CameraMatrices>().front();

    if (cameraEntity == entt::null) {
      GlobalLogger::Log(LogLevel::Error, "Camera entity is null");
      return;
    }

    auto& movement = registry.get<Movement>(cameraEntity);

    // TODO: this is more low level thing (engine side), should be moved
    auto cameraMatrices = registry.get<CameraMatrices>(cameraEntity);
    auto cameraRight    = cameraMatrices.view.getColumn<0>().resizedCopy<3>();
    auto cameraUp       = cameraMatrices.view.getColumn<1>().resizedCopy<3>();
    auto cameraForward  = cameraMatrices.view.getColumn<2>().resizedCopy<3>();

    math::Vector3Df resultDirection = math::g_zeroVector<float, 3>();

    if (m_actionStates_.test(static_cast<size_t>(Action::MoveForward))) {
      resultDirection += cameraForward;
    }
    if (m_actionStates_.test(static_cast<size_t>(Action::MoveBackward))) {
      resultDirection -= cameraForward;
    }
    if (m_actionStates_.test(static_cast<size_t>(Action::MoveLeft))) {
      resultDirection -= cameraRight;
    }
    if (m_actionStates_.test(static_cast<size_t>(Action::MoveRight))) {
      resultDirection += cameraRight;
    }
    if (m_actionStates_.test(static_cast<size_t>(Action::MoveUp))) {
      resultDirection += cameraUp;
    }
    if (m_actionStates_.test(static_cast<size_t>(Action::MoveDown))) {
      resultDirection -= cameraUp;
    }

    movement.direction = resultDirection;
    movement.strength  = m_movingSpeed_;
  }
}

void Game::linkWithEngine(const Engine& engine) {
  m_window_ = engine.getWindow();
}

void Game::release() {
}

}  // namespace game_engine
