
#include "game.h"

#include "config/config_manager.h"
#include "ecs/component_loaders.h"
#include "engine.h"
#include "gfx/rhi/rhi.h"
#include "input/input_manager.h"
#include "utils/hot_reload/hot_reload_manager.h"
#include "utils/path_manager/path_manager.h"
#include "utils/service/service_locator.h"

#include <ecs/components/movement.h>

#include <iostream>

namespace game_engine {

void Game::setup() {
  // TODO: consider change to std::mt19937
  srand(static_cast<uint32_t>(time(NULL)));

  // TODO: consider moving to another place (like init) if setup is called more
  {
    auto keyboardEventHandler
        = ServiceLocator::s_get<InputManager>()->getKeyboardHandler();

    // Subscribe to key press events to set action states
    keyboardEventHandler->subscribe(
        {SDL_KEYDOWN, SDL_SCANCODE_W}, [this](const KeyboardEvent& event) {
          m_actionStates_.set(static_cast<size_t>(Action::MoveForward), true);
        });

    keyboardEventHandler->subscribe(
        {SDL_KEYDOWN, SDL_SCANCODE_S}, [this](const KeyboardEvent& event) {
          m_actionStates_.set(static_cast<size_t>(Action::MoveBackward), true);
        });

    keyboardEventHandler->subscribe(
        {SDL_KEYDOWN, SDL_SCANCODE_A}, [this](const KeyboardEvent& event) {
          m_actionStates_.set(static_cast<size_t>(Action::MoveLeft), true);
        });

    keyboardEventHandler->subscribe(
        {SDL_KEYDOWN, SDL_SCANCODE_D}, [this](const KeyboardEvent& event) {
          m_actionStates_.set(static_cast<size_t>(Action::MoveRight), true);
        });

    keyboardEventHandler->subscribe(
        {SDL_KEYDOWN, SDL_SCANCODE_SPACE}, [this](const KeyboardEvent& event) {
          m_actionStates_.set(static_cast<size_t>(Action::MoveUp), true);
        });

    keyboardEventHandler->subscribe(
        {SDL_KEYDOWN, SDL_SCANCODE_LCTRL}, [this](const KeyboardEvent& event) {
          m_actionStates_.set(static_cast<size_t>(Action::MoveDown), true);
        });

    // Subscribe to key release events to reset action states
    keyboardEventHandler->subscribe(
        {SDL_KEYUP, SDL_SCANCODE_W}, [this](const KeyboardEvent& event) {
          m_actionStates_.set(static_cast<size_t>(Action::MoveForward), false);
        });

    keyboardEventHandler->subscribe(
        {SDL_KEYUP, SDL_SCANCODE_S}, [this](const KeyboardEvent& event) {
          m_actionStates_.set(static_cast<size_t>(Action::MoveBackward), false);
        });

    keyboardEventHandler->subscribe(
        {SDL_KEYUP, SDL_SCANCODE_A}, [this](const KeyboardEvent& event) {
          m_actionStates_.set(static_cast<size_t>(Action::MoveLeft), false);
        });

    keyboardEventHandler->subscribe(
        {SDL_KEYUP, SDL_SCANCODE_D}, [this](const KeyboardEvent& event) {
          m_actionStates_.set(static_cast<size_t>(Action::MoveRight), false);
        });

    keyboardEventHandler->subscribe(
        {SDL_KEYUP, SDL_SCANCODE_SPACE}, [this](const KeyboardEvent& event) {
          m_actionStates_.set(static_cast<size_t>(Action::MoveUp), false);
        });

    keyboardEventHandler->subscribe(
        {SDL_KEYUP, SDL_SCANCODE_LCTRL}, [this](const KeyboardEvent& event) {
          m_actionStates_.set(static_cast<size_t>(Action::MoveDown), false);
        });

    auto mouseEventHandler
        = ServiceLocator::s_get<InputManager>()->getMouseHandler();

    mouseEventHandler->subscribe(
        {SDL_MOUSEWHEEL}, [this](const MouseWheelEvent& event) {
          if (event.y > 0) {
            m_movingSpeed_ += s_speedChangeStep;
          } else if (event.y < 0) {
            m_movingSpeed_ -= s_speedChangeStep;
          }

          m_movingSpeed_ = std::clamp(m_movingSpeed_, s_minMovingSpeed, 100.0f);
        });

    mouseEventHandler->subscribe({SDL_MOUSEMOTION},
                                 [this](const MouseMotionEvent& event) {
                                   this->onMouseMove(event.xrel, event.yrel);
                                 });

    mouseEventHandler->subscribe({SDL_MOUSEBUTTONDOWN, SDL_BUTTON_RIGHT},
                                 [this](const MouseButtonEvent& event) {
                                   m_isRightMouseButtonPressed_ = true;
                                   SDL_SetRelativeMouseMode(SDL_TRUE);
                                 });

    mouseEventHandler->subscribe({SDL_MOUSEBUTTONUP, SDL_BUTTON_RIGHT},
                                 [this](const MouseButtonEvent& event) {
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
    //                   "config.json") .lock();

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
    //             .lock();
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
  // spawnObjects(ESpawnedType::TestPrimitive);

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
        auto config
            = configManager
                  ->getConfig(PathManager::s_getDebugPath() / "config.json")
                  .lock();
        if (config) {
          // TODO: consider moving to a more appropriate place (currently we
          // will call reload for each callback). But it's better to call it
          // only once the file is modified
          config->reloadAsync();
        }

        auto  sceneManager = ServiceLocator::s_get<SceneManager>();
        auto& registry = sceneManager->getCurrentScene()->getEntityRegistry();

        auto cameraEntity = registry.view<Camera>().front();

        if (cameraEntity == entt::null) {
          // TODO: log error
        }

        LoadCameraFromConfig(registry, cameraEntity);

        window->getSize().width();
        window->getSize().height();
      };

      auto hotReloadManager = ServiceLocator::s_get<HotReloadManager>();
      hotReloadManager->watchFileModifications(PathManager::s_getDebugPath(),
                                               fileModificationHandler);
    }
  }

  // cube
  {
    auto      cube = registry.create();
    Transform cubeTransform;
    cubeTransform.translation = math::Vector3Df(0.0f, 0.0f, 0.0f);
    cubeTransform.scale       = math::Vector3Df(1.0f, 1.0f, 1.0f);
    cubeTransform.rotation    = math::Vector3Df(0.0f, 0.0f, 0.0f);

    auto modelManager = ServiceLocator::s_get<RenderModelManager>();

    // TODO: use config file for file path
    auto cubeModel
        = modelManager->getRenderModel("assets/models/cube/cube.fbx");

    registry.emplace<Transform>(cube, cubeTransform);
    registry.emplace<std::shared_ptr<RenderModel>>(cube, cubeModel);
    registry.emplace<Movement>(cube);
  }

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
    registry.emplace<DirectionalLight>(directionalLightEntity,
                                       directionalLightData);

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
}

void Game::spawnObjects(ESpawnedType spawnType) {
  if (spawnType != m_spawnedType_) {
    m_spawnedType_ = spawnType;
    switch (m_spawnedType_) {
      case ESpawnedType::TestPrimitive:
        spawnTestPrimitives();
        break;
      case ESpawnedType::CubePrimitive:
        spawnCubePrimitives();
        break;
      case ESpawnedType::InstancingPrimitive:
        spawnInstancingPrimitives();
        break;
      case ESpawnedType::IndirectDrawPrimitive:
        spawnIndirectDrawPrimitives();
        break;
    }
  }
}

void Game::removeSpawnedObjects() {
  for (auto& iter : m_spawnedObjects_) {
    assert(iter);
    Object::s_removeObject(iter);
    delete iter;
  }
  m_spawnedObjects_.clear();
}

void Game::spawnTestPrimitives() {
  removeSpawnedObjects();

  auto triangle               = g_createTriangle(math::Vector3Df(0.2, 0.0, 0.0),
                                   math::g_oneVector<float, 3>(),
                                   math::Vector3Df(1.0, 1.0, 1.0),
                                   math::Vector4Df(1.0f, 1.0f, 1.0f, 1.0f));
  triangle->m_postUpdateFunc_ = [](Object* thisObject, float deltaTime) {
    // thisObject->m_renderObjects_[0]->SetRot(thisObject->m_renderObjects_[0]->GetRot()
    //                                      + math::Vector3Df(5.0f, 0.0f, 0.0f)
    //                                            * deltaTime);
  };
  Object::s_addObject(triangle);
  m_spawnedObjects_.push_back(triangle);

  // auto quad = createQuad(math::Vector3Df(1.0f, 1.0f, 1.0f),
  //                        math::Vector3Df(1.0f),
  //                        math::Vector3Df(1000.0f, 1000.0f, 1000.0f),
  //                        math::Vector4Df(1.0f, 1.0f, 1.0f, 1.0f));
  // quad->setPlane(math::Plane(math::Vector3Df(0.0, 1.0, 0.0), -0.1f));
  // quad->m_skipUpdateShadowVolume_ = true;
  // Object::s_addObject(quad);
  // m_spawnedObjects_.push_back(quad);

  // auto gizmo              = g_createGizmo(math::g_zeroVector<float, 3>(),
  //                          math::g_zeroVector<float, 3>(),
  //                          math::g_oneVector<float, 3>());
  // gizmo->m_skipShadowMapGen_ = true;
  // Object::s_addObject(gizmo);
  // m_spawnedObjects_.push_back(gizmo);

  // auto triangle            = g_createTriangle(math::Vector3Df(60.0,
  // 100.0, 20.0),
  //                                math::g_oneVector<float, 3>(),
  //                                math::Vector3Df(40.0, 40.0, 40.0),
  //                                math::Vector4Df(0.5f, 0.1f, 1.0f, 1.0f));
  // triangle->m_postUpdateFunc_ = [](Object* thisObject, float deltaTime) {
  //   thisObject->m_renderObjects_[0]->SetRot(thisObject->m_renderObjects_[0]->GetRot()
  //                                        + math::Vector3Df(5.0f, 0.0f, 0.0f)
  //                                              * deltaTime);
  // };
  // Object::s_addObject(triangle);
  // m_spawnedObjects_.push_back(triangle);

  // auto cube            = g_createCube(math::Vector3Df(-60.0f, 55.0f, -20.0f),
  //                        math::g_oneVector<float, 3>(),
  //                        math::Vector3Df(50.0f, 50.0f, 50.0f),
  //                        math::Vector4Df(0.7f, 0.7f, 0.7f, 1.0f));
  // cube->m_postUpdateFunc_ = [](Object* thisObject, float deltaTime) {
  //   thisObject->m_renderObjects_[0]->SetRot(thisObject->m_renderObjects_[0]->GetRot()
  //                                        + math::Vector3Df(0.0f, 0.0f, 0.5f)
  //                                              * deltaTime);
  // };
  // Object::s_addObject(cube);
  // m_spawnedObjects_.push_back(cube);

  // auto cube2 = g_createCube(math::Vector3Df(-65.0f, 35.0f, 10.0f),
  //                         math::g_oneVector<float, 3>(),
  //                         math::Vector3Df(50.0f, 50.0f, 50.0f),
  //                         math::Vector4Df(0.7f, 0.7f, 0.7f, 1.0f));
  // Object::s_addObject(cube2);
  // m_spawnedObjects_.push_back(cube2);

  // auto capsule            = g_createCapsule(math::Vector3Df(30.0f, 30.0f,
  // -80.0f),
  //                              40.0f,
  //                              10.0f,
  //                              20,
  //                              math::Vector3Df(1.0f),
  //                              math::Vector4Df(1.0f, 1.0f, 0.0f, 1.0f));
  // capsule->m_postUpdateFunc_ = [](Object* thisObject, float deltaTime) {
  //   thisObject->m_renderObjects_[0]->SetRot(thisObject->m_renderObjects_[0]->GetRot()
  //                                        + math::Vector3Df(-1.0f, 0.0f, 0.0f)
  //                                              * deltaTime);
  // };
  // Object::s_addObject(capsule);
  // m_spawnedObjects_.push_back(capsule);

  // auto cone            = g_createCone(math::Vector3Df(0.0f, 50.0f, 60.0f),
  //                        40.0f,
  //                        20.0f,
  //                        15,
  //                        math::g_oneVector<float, 3>(),
  //                        math::Vector4Df(1.0f, 1.0f, 0.0f, 1.0f));
  // cone->m_postUpdateFunc_ = [](Object* thisObject, float deltaTime) {
  //   thisObject->m_renderObjects_[0]->SetRot(thisObject->m_renderObjects_[0]->GetRot()
  //                                        + math::Vector3Df(0.0f, 3.0f, 0.0f)
  //                                              * deltaTime);
  // };
  // Object::s_addObject(cone);
  // m_spawnedObjects_.push_back(cone);

  // auto cylinder = g_createCylinder(math::Vector3Df(-30.0f, 60.0f, -60.0f),
  //                                20.0f,
  //                                10.0f,
  //                                20,
  //                                math::g_oneVector<float, 3>(),
  //                                math::Vector4Df(0.0f, 0.0f, 1.0f, 1.0f));
  // cylinder->m_postUpdateFunc_ = [](Object* thisObject, float deltaTime) {
  //   thisObject->m_renderObjects_[0]->SetRot(thisObject->m_renderObjects_[0]->GetRot()
  //                                        + math::Vector3Df(5.0f, 0.0f, 0.0f)
  //                                              * deltaTime);
  // };
  // Object::s_addObject(cylinder);
  // m_spawnedObjects_.push_back(cylinder);

  // auto quad2            = createQuad(math::Vector3Df(-20.0f, 80.0f, 40.0f),
  //                         math::g_oneVector<float, 3>(),
  //                         math::Vector3Df(20.0f, 20.0f, 20.0f),
  //                         math::Vector4Df(0.0f, 0.0f, 1.0f, 1.0f));
  // quad2->m_postUpdateFunc_ = [](Object* thisObject, float deltaTime) {
  //   thisObject->m_renderObjects_[0]->SetRot(thisObject->m_renderObjects_[0]->GetRot()
  //                                        + math::Vector3Df(0.0f, 0.0f, 8.0f)
  //                                              * deltaTime);
  // };
  // Object::s_addObject(quad2);
  // m_spawnedObjects_.push_back(quad2);

  // auto sphere            =
  // g_createSphere(math::Vector3Df(65.0f, 35.0f, 10.0f),
  //                            1.0,
  //                            150,
  //                            75,
  //                            math::Vector3Df(30.0f),
  //                            math::Vector4Df(0.8f, 0.0f, 0.0f, 1.0f));
  // sphere->m_postUpdateFunc_ = [](Object* thisObject, float deltaTime) {
  //   float RotationSpeed = 100.0f;
  //   thisObject->m_renderObjects_[0]->SetRot(
  //       thisObject->m_renderObjects_[0]->GetRot()
  //       + math::Vector3Df(0.0f, 0.0f, math::g_degreeToRadian(180.0f))
  //             * RotationSpeed * deltaTime);
  // };
  // Object::s_addObject(sphere);
  // m_spawnedObjects_.push_back(sphere);

  // auto sphere2            = g_createSphere(math::Vector3Df(150.0f, 5.0f,
  // 0.0f),
  //                             1.0,
  //                             150,
  //                             75,
  //                             math::Vector3Df(10.0f),
  //                             math::Vector4Df(0.8f, 0.4f, 0.6f, 1.0f));
  // sphere2->m_postUpdateFunc_ = [](Object* thisObject, float deltaTime) {
  //   const float startY  = 5.0f;
  //   const float endY    = 100;
  //   const float speed   = 150.0f * deltaTime;
  //   static bool dir     = true;
  //   auto        m_position_     = thisObject->m_renderObjects_[0]->GetPos();
  //   m_position_.y()            += dir ? speed : -speed;
  //   if (m_position_.y() < startY || m_position_.y() > endY) {
  //     dir      = !dir;
  //     m_position_.y() += dir ? speed : -speed;
  //   }
  //   thisObject->m_renderObjects_[0]->setPosition(m_position_);
  // };
  // Object::s_addObject(sphere2);
  // m_spawnedObjects_.push_back(sphere2);

  // auto billboard = g_createBillobardQuad(math::Vector3Df(0.0f, 60.0f, 80.0f),
  //                                      math::g_oneVector<float, 3>(),
  //                                      math::Vector3Df(20.0f, 20.0f, 20.0f),
  //                                      math::Vector4Df(1.0f,
  //                                      0.0f, 1.0f, 1.0f), m_mainCamera_);
  // Object::s_addObject(billboard);
  // m_spawnedObjects_.push_back(billboard);

  // const float Size = 20.0f;

  // for (int32_t i = 0; i < 10; ++i)
  //{
  //	for (int32_t j = 0; j < 10; ++j)
  //	{
  //		for (int32_t k = 0; k < 5; ++k)
  //		{
  //			auto cube = g_createCube(math::Vector3Df(i * 25.0f,
  // k * 25.0f, j * 25.0f), math::g_oneVector<float, 3>(),
  // math::Vector3Df(Size), math::Vector4Df(0.7f, 0.7f, 0.7f, 1.0f));
  // Object::s_addObject(cube);
  //			m_spawnedObjects_.push_back(cube);
  //		}
  //	}
  // }
}

// difference between perspective and orthographic projection
void Game::spawnGraphTestFunc() {
  // math::Vector3Df PerspectiveVector[90];
  // math::Vector3Df OrthographicVector[90];
  //{
  //   {
  //     static std::shared_ptr<CameraOld> pCamera
  //         = std::make_shared<CameraOld>(math::Vector3Df(0.0),
  //                                    math::Vector3Df(0.0, 0.0, 1.0),
  //                                    math::Vector3Df(0.0, 1.0, 0.0),
  //                                    math::g_degreeToRadian(90.0f),
  //                                    10.0,
  //                                    100.0,
  //                                    100.0,
  //                                    100.0,
  //                                    ECameraTypeDeprecated::Perspective);
  //     pCamera->updateCamera();
  //     int  cnt = 0;
  //     auto MV  = pCamera->m_projection_ * pCamera->m_view_;
  //     for (int i = 0; i < 90; ++i) {
  //       PerspectiveVector[cnt++] = math::g_transformPoint(
  //           math::Vector3Df({0.0f, 0.0f, 10.0f + static_cast<float>(i)}),
  //           MV);
  //     }

  //    for (int i = 0; i < std::size(PerspectiveVector); ++i) {
  //      PerspectiveVector[i].z() = (PerspectiveVector[i].z() + 1.0f) * 0.5f;
  //    }
  //  }
  //  {
  //    static std::shared_ptr<CameraOld> pCamera
  //        = std::make_shared<CameraOld>(math::Vector3Df(0.0),
  //                                   math::Vector3Df(0.0, 0.0, 1.0),
  //                                   math::Vector3Df(0.0, 1.0, 0.0),
  //                                   math::g_degreeToRadian(90.0f),
  //                                   10.0,
  //                                   100.0,
  //                                   100.0,
  //                                   100.0,
  //                                   ECameraTypeDeprecated::Orthographic);
  //    pCamera->updateCamera();
  //    int  cnt = 0;
  //    auto MV  = pCamera->m_projection_ * pCamera->m_view_;
  //    for (int i = 0; i < 90; ++i) {
  //      OrthographicVector[cnt++] = math::g_transformPoint(
  //          math::Vector3Df({0.0f, 0.0f, 10.0f + static_cast<float>(i)}), MV);
  //    }

  //    for (int i = 0; i < std::size(OrthographicVector); ++i) {
  //      OrthographicVector[i].z() = (OrthographicVector[i].z() + 1.0f) * 0.5f;
  //    }
  //  }
  //}
  // std::vector<math::Vector2Df> graph1;
  // std::vector<math::Vector2Df> graph2;

  // float scale = 100.0f;
  // for (int i = 0; i < std::size(PerspectiveVector); ++i) {
  //   graph1.push_back(math::Vector2Df(static_cast<float>(i * 2),
  //                                    PerspectiveVector[i].z() * scale));
  // }
  // for (int i = 0; i < std::size(OrthographicVector); ++i) {
  //   graph2.push_back(math::Vector2Df(static_cast<float>(i * 2),
  //                                    OrthographicVector[i].z() * scale));
  // }

  // auto graphObj1 = g_createGraph2D({360, 350}, {360, 300}, graph1);
  // Object::s_addUIDebugObject(graphObj1);

  // auto graphObj2 = g_createGraph2D({360, 700}, {360, 300}, graph2);
  // Object::s_addUIDebugObject(graphObj2);
}

void Game::spawnCubePrimitives() {
  removeSpawnedObjects();

  for (int i = 0; i < 20; ++i) {
    float height = 5.0f * i;
    auto  cube   = g_createCube(
        math::Vector3Df(-500.0f + i * 50.0f, height / 2.0f, 20.0f),
        math::g_oneVector<float, 3>(),
        math::Vector3Df(10.0f, height, 20.0f),
        math::Vector4Df(0.7f, 0.7f, 0.7f, 1.0f));
    Object::s_addObject(cube);
    m_spawnedObjects_.push_back(cube);
    cube = g_createCube(
        math::Vector3Df(-500.0f + i * 50.0f, height / 2.0f, 20.0f + i * 20.0f),
        math::g_oneVector<float, 3>(),
        math::Vector3Df(10.0f, height, 10.0f),
        math::Vector4Df(0.7f, 0.7f, 0.7f, 1.0f));
    Object::s_addObject(cube);
    m_spawnedObjects_.push_back(cube);
    cube = g_createCube(
        math::Vector3Df(-500.0f + i * 50.0f, height / 2.0f, 20.0f - i * 20.0f),
        math::g_oneVector<float, 3>(),
        math::Vector3Df(20.0f, height, 10.0f),
        math::Vector4Df(0.7f, 0.7f, 0.7f, 1.0f));
    Object::s_addObject(cube);
    m_spawnedObjects_.push_back(cube);
  }

  auto quad = g_createQuad(math::Vector3Df(1.0f, 1.0f, 1.0f),
                           math::Vector3Df(1.0f),
                           math::Vector3Df(1000.0f, 1000.0f, 1000.0f),
                           math::Vector4Df(1.0f, 1.0f, 1.0f, 1.0f));
  quad->setPlane(math::Plane(math::Vector3Df(0.0, 1.0, 0.0), -0.1f));
  Object::s_addObject(quad);
  m_spawnedObjects_.push_back(quad);
}

void Game::spawnInstancingPrimitives() {
  struct InstanceData {
    math::Vector4Df m_color;
    math::Vector3Df m_w;
  };

  const int    numInstances = 100;
  InstanceData instanceData[numInstances];

  const float     colorStep = 1.0f / (float)sqrt(std::size(instanceData));
  math::Vector4Df curStep
      = math::Vector4Df(colorStep, colorStep, colorStep, 1.0f);

  for (int32_t i = 0; i < std::size(instanceData); ++i) {
    float x                 = (float)(i / 10);
    float y                 = (float)(i % 10);
    instanceData[i].m_w     = math::Vector3Df(y * 10.0f, x * 10.0f, 0.0f);
    instanceData[i].m_color = curStep;
    if (i < std::size(instanceData) / 3) {
      curStep.x() += colorStep;
    } else if (i < std::size(instanceData) / 2) {
      curStep.y() += colorStep;
    } else if (i < std::size(instanceData)) {
      curStep.z() += colorStep;
    }
  }

  {
    auto obj         = g_createTriangle(math::Vector3Df(0.0f, 0.0f, 0.0f),
                                math::g_oneVector<float, 3>() * 8.0f,
                                math::g_oneVector<float, 3>(),
                                math::Vector4Df(1.0f, 0.0f, 0.0f, 1.0f));
    auto streamParam = std::make_shared<BufferAttributeStream<InstanceData>>(
        Name("InstanceData"),
        EBufferType::Static,
        sizeof(InstanceData),
        std::vector<IBufferAttribute::Attribute>{
          IBufferAttribute::Attribute(
              EBufferElementType::FLOAT, 0, math::Vector4Df::GetDataSize()),
          IBufferAttribute::Attribute(EBufferElementType::FLOAT,
                                      math::Vector4Df::GetDataSize(),
                                      math::Vector3Df::GetDataSize())},
        std::vector<InstanceData>(instanceData, instanceData + numInstances));

    // TODO: remove
    // auto streamParam =
    // std::make_shared<BufferAttributeStream<InstanceData>>();
    // streamParam->BufferType = EBufferType::Static;
    // streamParam->Attributes.push_back(IBufferAttribute::Attribute(
    //    EBufferElementType::FLOAT, 0, math::Vector4Df::GetDataSize()));
    // streamParam->Attributes.push_back(IBufferAttribute::Attribute(
    //    EBufferElementType::FLOAT, math::Vector3Df::GetDataSize()));
    // streamParam->Stride = sizeof(InstanceData);
    // streamParam->name   = Name("InstanceData");
    // streamParam->Data.resize(numInstances);
    // memcpy(&streamParam->Data[0], instanceData, sizeof(instanceData));

    auto& GeometryDataPtr = obj->m_renderObjects_[0]->m_geometryDataPtr_;

    GeometryDataPtr->m_vertexStreamInstanceDataPtr_
        = std::make_shared<VertexStreamData>();
    GeometryDataPtr->m_vertexStreamInstanceDataPtr_->m_elementCount_
        = std::size(instanceData);
    GeometryDataPtr->m_vertexStreamInstanceDataPtr_->m_startLocation_
        = (int32_t)GeometryDataPtr->m_vertexStreamPtr_->getEndLocation();
    GeometryDataPtr->m_vertexStreamInstanceDataPtr_->m_bindingIndex_
        = (int32_t)GeometryDataPtr->m_vertexStreamPtr_->m_streams_.size();
    GeometryDataPtr->m_vertexStreamInstanceDataPtr_->m_vertexInputRate_
        = EVertexInputRate::INSTANCE;
    GeometryDataPtr->m_vertexStreamInstanceDataPtr_->m_streams_.push_back(
        streamParam);
    GeometryDataPtr->m_vertexBufferInstanceDataPtr_ = g_rhi->createVertexBuffer(
        GeometryDataPtr->m_vertexStreamInstanceDataPtr_);

    Object::s_addObject(obj);
    m_spawnedObjects_.push_back(obj);
  }
}

void Game::spawnIndirectDrawPrimitives() {
  struct InstanceData {
    math::Vector4Df m_color;
    math::Vector3Df m_w;
  };

  const int    numInstances = 100;
  InstanceData instanceData[numInstances];

  const float     colorStep = 1.0f / (float)sqrt(std::size(instanceData));
  math::Vector4Df curStep
      = math::Vector4Df(colorStep, colorStep, colorStep, 1.0f);

  for (int32_t i = 0; i < std::size(instanceData); ++i) {
    float x                 = (float)(i / 10);
    float y                 = (float)(i % 10);
    instanceData[i].m_w     = math::Vector3Df(y * 10.0f, x * 10.0f, 0.0f);
    instanceData[i].m_color = curStep;
    if (i < std::size(instanceData) / 3) {
      curStep.x() += colorStep;
    } else if (i < std::size(instanceData) / 2) {
      curStep.y() += colorStep;
    } else if (i < std::size(instanceData)) {
      curStep.z() += colorStep;
    }
  }

  {
    auto obj = g_createTriangle(math::Vector3Df(0.0f, 0.0f, 0.0f),
                                math::g_oneVector<float, 3>() * 8.0f,
                                math::g_oneVector<float, 3>(),
                                math::Vector4Df(1.0f, 0.0f, 0.0f, 1.0f));

    auto streamParam = std::make_shared<BufferAttributeStream<InstanceData>>(
        Name("InstanceData"),
        EBufferType::Static,
        sizeof(InstanceData),
        std::vector<IBufferAttribute::Attribute>{
          IBufferAttribute::Attribute(
              EBufferElementType::FLOAT, 0, math::Vector4Df::GetDataSize()),
          IBufferAttribute::Attribute(EBufferElementType::FLOAT,
                                      math::Vector4Df::GetDataSize(),
                                      math::Vector3Df::GetDataSize())},
        std::vector<InstanceData>(instanceData, instanceData + numInstances));

    // TODO: remove
    // auto streamParam =
    // std::make_shared<BufferAttributeStream<InstanceData>>();
    // streamParam->BufferType = EBufferType::Static;
    // streamParam->Attributes.push_back(IBufferAttribute::Attribute(
    //    EBufferElementType::FLOAT, math::Vector4Df::GetDataSize()));
    // streamParam->Attributes.push_back(IBufferAttribute::Attribute(
    //    EBufferElementType::FLOAT, math::Vector3Df::GetDataSize()));
    // streamParam->Stride = sizeof(InstanceData);
    // streamParam->name   = Name("InstanceData");
    // streamParam->Data.resize(numInstances);
    // memcpy(&streamParam->Data[0], instanceData, sizeof(instanceData));

    auto& GeometryDataPtr = obj->m_renderObjects_[0]->m_geometryDataPtr_;
    GeometryDataPtr->m_vertexStreamInstanceDataPtr_
        = std::make_shared<VertexStreamData>();
    GeometryDataPtr->m_vertexStreamInstanceDataPtr_->m_elementCount_
        = std::size(instanceData);
    GeometryDataPtr->m_vertexStreamInstanceDataPtr_->m_startLocation_
        = (int32_t)GeometryDataPtr->m_vertexStreamPtr_->getEndLocation();
    GeometryDataPtr->m_vertexStreamInstanceDataPtr_->m_bindingIndex_
        = (int32_t)GeometryDataPtr->m_vertexStreamPtr_->m_streams_.size();
    GeometryDataPtr->m_vertexStreamInstanceDataPtr_->m_vertexInputRate_
        = EVertexInputRate::INSTANCE;
    GeometryDataPtr->m_vertexStreamInstanceDataPtr_->m_streams_.push_back(
        streamParam);
    GeometryDataPtr->m_vertexBufferInstanceDataPtr_ = g_rhi->createVertexBuffer(
        GeometryDataPtr->m_vertexStreamInstanceDataPtr_);

    // Create indirect draw buffer
    // TODO: Refactor this code to decouple it from Vulkan-specific classes and
    // functions (e.g., VkDrawIndirectCommand, g_rhi->createVertexBuffer).
    // Consider using an abstract rendering interface to handle draw calls and
    // resource management.
    {
      assert(GeometryDataPtr->m_vertexStreamInstanceDataPtr_);

      std::vector<VkDrawIndirectCommand> indrectCommands;

      const int32_t instanceCount
          = GeometryDataPtr->m_vertexStreamInstanceDataPtr_->m_elementCount_;
      const int32_t vertexCount
          = GeometryDataPtr->m_vertexStreamPtr_->m_elementCount_;
      for (int32_t i = 0; i < instanceCount; ++i) {
        VkDrawIndirectCommand command;
        command.vertexCount   = vertexCount;
        command.instanceCount = 1;
        command.firstVertex   = 0;
        command.firstInstance = i;
        indrectCommands.emplace_back(command);
      }

      const size_t bufferSize
          = indrectCommands.size() * sizeof(VkDrawIndirectCommand);

      assert(!GeometryDataPtr->m_indirectCommandBufferPtr_);
      GeometryDataPtr->m_indirectCommandBufferPtr_
          = g_rhi->createStructuredBuffer(bufferSize,
                                          0,
                                          sizeof(VkDrawIndirectCommand),
                                          EBufferCreateFlag::IndirectCommand,
                                          EResourceLayout::TRANSFER_DST,
                                          indrectCommands.data(),
                                          bufferSize);
    }

    Object::s_addObject(obj);
    m_spawnedObjects_.push_back(obj);
  }
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
    // TODO: log error
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
      // TODO: log error
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

  // m_mainCamera_->setPosition(cameraPos + moveSpeed * resultDirection);

  //// update main camera
  // if (m_mainCamera_) {
  //   m_mainCamera_->updateCamera();
  // }

  // gOptions.CameraPos = m_mainCamera_->m_position_;

  // for (auto iter : Object::s_getStaticObject())
  //	iter->update(deltaTime);

  // for (auto& iter : Object::s_getBoundBoxObject())
  //	iter->update(deltaTime);

  // for (auto& iter : Object::s_getBoundSphereObject())
  //	iter->update(deltaTime);

  // for (auto& iter : Object::s_getDebugObject())
  //	iter->update(deltaTime);

  // update object which have dirty flag
  Object::s_flushDirtyState();

  //// render all objects by using selected renderer
  // Renderer->render(m_mainCamera_);

  // for (auto& iter : Object::s_getStaticObject()) {
  //   iter->update(deltaTime);

  //  for (auto& RenderObject : iter->m_renderObjects_) {
  //    RenderObject->updateWorldMatrix();
  //  }
  //}

  // for (auto& iter : Object::s_getDebugObject()) {
  //   iter->update(deltaTime);

  //  for (auto& RenderObject : iter->m_renderObjects_) {
  //    RenderObject->updateWorldMatrix();
  //  }
  //}
}

void Game::linkWithEngine(const Engine& engine) {
  m_window_ = engine.getWindow();
}

void Game::release() {
  g_rhi->flush();

  for (Object* iter : m_spawnedObjects_) {
    delete iter;
  }
  m_spawnedObjects_.clear();

  // delete m_mainCamera_;
}

}  // namespace game_engine
