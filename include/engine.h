#ifndef GAME_ENGINE_ENGINE_H
#define GAME_ENGINE_ENGINE_H

#include "config/config_manager.h"
#include "config/runtime_settings.h"
#include "ecs/component_loaders.h"
#include "ecs/components/camera.h"
#include "ecs/systems/camera_system.h"
#include "ecs/systems/movement_system.h"
#include "ecs/systems/render_system.h"
#include "ecs/systems/system_manager.h"
#include "editor/editor.h"
#include "event/application_event_manager.h"
#include "event/window_event_manager.h"
#include "game.h"
#include "gfx/rhi/dx12/rhi_dx12.h"
#include "gfx/rhi/rhi.h"
#include "gfx/rhi/shader_manager.h"
#include "gfx/rhi/vulkan/rhi_vk.h"
#include "input/input_manager.h"
#include "platform/common/window.h"
#include "resources/assimp_material_loader.h"
#include "resources/assimp_render_model_loader.h"
#include "scene/scene_manager.h"
#include "utils/hot_reload/hot_reload_manager.h"
#include "utils/image/image_loader_manager.h"
#include "utils/image/image_manager.h"
#include "utils/logger/console_logger.h"
#include "utils/logger/file_logger.h"
#include "utils/logger/global_logger.h"
#include "utils/logger/memory_logger.h"
#include "utils/model/render_model_loader_manager.h"
#include "utils/model/render_model_manager.h"
#include "utils/path_manager/path_manager.h"
#include "utils/service/service_locator.h"
#include "utils/third_party/directx_tex_util.h"
#include "utils/third_party/stb_util.h"
#include "utils/time/stopwatch.h"
#include "utils/time/timing_manager.h"

#include <memory>

namespace game_engine {

class Engine {
  public:
  // ======= BEGIN: public constructors =======================================

  Engine()              = default;
  Engine(const Engine&) = delete;
  Engine(Engine&&)      = delete;

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  ~Engine() {
    g_rhi->flush();
    m_game_->release();
    m_renderer_.reset();
    ServiceLocator::s_remove<ConfigManager>();
    ServiceLocator::s_remove<FileWatcherManager>();
    ServiceLocator::s_remove<HotReloadManager>();
    ServiceLocator::s_remove<InputManager>();
    ServiceLocator::s_remove<WindowEventManager>();
    ServiceLocator::s_remove<ApplicationEventManager>();
    ServiceLocator::s_remove<SceneManager>();
    ServiceLocator::s_remove<SystemManager>();
    ServiceLocator::s_remove<TimingManager>();
    ServiceLocator::s_remove<RenderModelManager>();
    ServiceLocator::s_remove<RenderModelLoaderManager>();
    ServiceLocator::s_remove<ImageManager>();
    ServiceLocator::s_remove<ImageLoaderManager>();
    ServiceLocator::s_remove<ShaderManager>();
    g_rhi->release();
  }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public getters ============================================

  auto getWindow() const -> const std::shared_ptr<Window>& { return m_window_; }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public setters ============================================

  void setGame(std::shared_ptr<Game> game) {
    m_game_ = std::move(game);
    m_game_->linkWithEngine(*this);
  }

  // ======= END: public setters   ============================================

  // ======= BEGIN: public misc methods =======================================

  // TODO: add config file
  auto init() -> bool {
    bool successfullyInitialized{true};

    // logger
    // ------------------------------------------------------------------------
    m_consoleLogger_ = std::make_shared<ConsoleLogger>("console_logger");
    GlobalLogger::AddLogger(m_consoleLogger_);

    // auto memoryLogger = std::make_shared<MemoryLogger>("memory_logger");
    // GlobalLogger::AddLogger(memoryLogger);

    // auto fileLogger = std::make_shared<FileLogger>("file_logger");
    // GlobalLogger::AddLogger(fileLogger);

    // input event
    // ------------------------------------------------------------------------
    auto keyboardEventHandler = std::make_shared<KeyboardEventHandler>();
    auto mouseEventHandler    = std::make_shared<MouseEventHandler>();

    // window event
    // ------------------------------------------------------------------------
    auto windowEventHandler = std::make_shared<WindowEventHandler>();
    windowEventHandler->subscribe(
        SDL_WINDOWEVENT_RESIZED, [this](const WindowEvent& event) {
          this->m_window_->onResize(event);
          // TODO: add here camera width and height change
          auto scene = ServiceLocator::s_get<SceneManager>()->getCurrentScene();
          auto& registry = scene->getEntityRegistry();
          auto  view     = registry.view<Camera>();
          auto  entity   = view.front();
          if (entity != entt::null) {
            auto& camera  = view.get<Camera>(entity);
            camera.width  = event.data1;
            camera.height = event.data2;
          }

          // TODO: refactor
          g_rhi->onHandleResized(event.data1, event.data2, false);
        });

    // application event
    // ------------------------------------------------------------------------
    auto applicationEventHandler = std::make_shared<ApplicationEventHandler>();
    applicationEventHandler->subscribe(
        SDL_QUIT, std::bind(&Engine::onClose, this, std::placeholders::_1));
    // [this](const ApplicationEvent& event) { this->onClose(event); }

    // service locator
    // ------------------------------------------------------------------------
    ServiceLocator::s_provide<ConfigManager>();
    ServiceLocator::s_provide<FileWatcherManager>();
    ServiceLocator::s_provide<HotReloadManager>();
    ServiceLocator::s_provide<InputManager>(keyboardEventHandler,
                                            mouseEventHandler);
    ServiceLocator::s_provide<WindowEventManager>(windowEventHandler);
    ServiceLocator::s_provide<ApplicationEventManager>(applicationEventHandler);
    ServiceLocator::s_provide<SceneManager>();
    ServiceLocator::s_provide<SystemManager>();
    ServiceLocator::s_provide<TimingManager>();
    ServiceLocator::s_provide<ShaderManager>(false);

    // config
    // ------------------------------------------------------------------------
    auto configManager = ServiceLocator::s_get<ConfigManager>();
    auto debugPath     = PathManager::s_getDebugPath() / "config.json";
    configManager->addConfig(debugPath);
    auto debugConfig = configManager->getConfig(debugPath).lock();

    auto renderingApi = debugConfig->get<std::string>("renderingApi");

    // register config
    // ------------------------------------------------------------------------
    debugConfig->registerConverter<math::Vector3Df>(
        &math::g_getVectorfromConfig);
    debugConfig->registerConverter<math::Quaternionf>(
        &math::g_getQuaternionfromConfig);
    debugConfig->registerConverter<Transform>(&LoadTransform);
    debugConfig->registerConverter<Camera>(&LoadCamera);

    // hot reload
    // ------------------------------------------------------------------------
    // auto hotReloadManager = ServiceLocator::s_get<HotReloadManager>();

    // window
    // ------------------------------------------------------------------------
    m_window_ = std::make_unique<Window>(
        renderingApi,
        // Desired size (for maximized window will be 0)
        math::Dimension2Di{0, 0},
        math::Point2Di{SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED},
        game_engine::Window::Flags::Resizable
            // TODO: Vulkan flag may not work for DX12 window
            | game_engine::Window::Flags::Vulkan
            | game_engine::Window::Flags::Maximized);

    if (renderingApi == "dx12") {
      g_rhi = new RhiDx12();
    } else if (renderingApi == "vulkan") {
      g_rhi = new RhiVk();
    } else {
      g_rhi = new RhiVk();
    }
    g_rhi->init(m_window_);
    g_rhi->onInitRHI();

    // image loader
    // ------------------------------------------------------------------------
    auto imageLoaderManager = std::make_shared<ImageLoaderManager>();
    auto stbLoader          = std::make_shared<STBImageLoader>();
    imageLoaderManager->registerLoader(EImageType::JPEG, stbLoader);
    imageLoaderManager->registerLoader(EImageType::JPG, stbLoader);
    imageLoaderManager->registerLoader(EImageType::PNG, stbLoader);
    imageLoaderManager->registerLoader(EImageType::BMP, stbLoader);
    imageLoaderManager->registerLoader(EImageType::TGA, stbLoader);
    imageLoaderManager->registerLoader(EImageType::GIF, stbLoader);
    imageLoaderManager->registerLoader(EImageType::HDR, stbLoader);
    imageLoaderManager->registerLoader(EImageType::PIC, stbLoader);
    imageLoaderManager->registerLoader(EImageType::PPM, stbLoader);
    imageLoaderManager->registerLoader(EImageType::PGM, stbLoader);
    auto directxTexLoader = std::make_shared<DirectXTexImageLoader>();
    imageLoaderManager->registerLoader(EImageType::DDS, directxTexLoader);
    ServiceLocator::s_provide<ImageLoaderManager>(imageLoaderManager);

    auto imageManager = std::make_shared<ImageManager>();
    ServiceLocator::s_provide<ImageManager>(imageManager);

    // model / material loader
    // ------------------------------------------------------------------------
    auto renderModelLoaderManager
        = std::make_shared<RenderModelLoaderManager>();
    auto assimpModelLoader = std::make_shared<AssimpRenderModelLoader>();
    renderModelLoaderManager->registerLoader(EModelType::OBJ,
                                             assimpModelLoader);
    renderModelLoaderManager->registerLoader(EModelType::FBX,
                                             assimpModelLoader);
    ServiceLocator::s_provide<RenderModelLoaderManager>(
        renderModelLoaderManager);

    auto renderModelManager = std::make_shared<RenderModelManager>();
    ServiceLocator::s_provide<RenderModelManager>(renderModelManager);

    // ecs
    // ------------------------------------------------------------------------
    auto systemManager = ServiceLocator::s_get<SystemManager>();
    systemManager->addSystem(std::make_shared<CameraSystem>());
    systemManager->addSystem(std::make_shared<MovementSystem>());
    systemManager->addSystem(std::make_shared<RenderSystem>());

    // renderer
    // ------------------------------------------------------------------------
    m_renderer_ = std::make_shared<Renderer>();

    // editor
    // ------------------------------------------------------------------------
    m_editor_ = std::make_shared<Editor>();
    m_editor_->init(m_window_);

    return successfullyInitialized;
  }

  void render() {
    // TODO: temp solution to create render context here and each frame
    // (consider other approach)
    // auto viewportDimension = m_window_->getSize();
    auto viewportDimension
        = m_editor_->getRenderParams().editorViewportDimension;

    auto renderContext = std::make_shared<RenderContext>();
    renderContext->scene
        = ServiceLocator::s_get<SceneManager>()->getCurrentScene();
    renderContext->viewportDimension = m_window_->getSize();
    renderContext->renderFrameContext
        = g_rhi->beginRenderFrame(viewportDimension);

    m_renderer_->renderFrame(renderContext, m_editor_->getRenderParams());

    m_editor_->render(renderContext);

    g_rhi->endRenderFrame(renderContext->renderFrameContext);

    g_rhi->incrementFrameNumber();
  }

  void run() {
    m_isRunning_ = true;

    while (m_isRunning_) {
      auto timingManager = ServiceLocator::s_get<TimingManager>();
      timingManager->update();
      processEvents_();
      m_game_->processInput();  // TODO: consider remove
      update_(timingManager->getDeltaTime());
      render();
    }
  }

  void onClose(const ApplicationEvent& event) { m_isRunning_ = false; }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public overloaded operators ===============================

  auto operator=(const Engine&) -> Engine&  = delete;
  auto operator=(const Engine&&) -> Engine& = delete;

  // ======= END: public overloaded operators   ===============================

  private:
  // ======= BEGIN: private misc methods ======================================

  void processEvents_() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      ServiceLocator::s_get<InputManager>()->routeEvent(event);
      ServiceLocator::s_get<WindowEventManager>()->routeEvent(event);
      ServiceLocator::s_get<ApplicationEventManager>()->routeEvent(event);
    }
  }

  void update_(float deltaTime) {
    auto systemManager = ServiceLocator::s_get<SystemManager>();
    auto scene = ServiceLocator::s_get<SceneManager>()->getCurrentScene();
    systemManager->updateSystems(scene, deltaTime);

    m_game_->update(deltaTime);

    // Shader::s_startAndRunCheckUpdateShaderThread();
  }

  // ======= END: private misc methods   ======================================

  // ======= BEGIN: private misc fields =======================================

  bool                           m_isRunning_{false};
  // DeltaTime                      m_deltaTime_;
  std::shared_ptr<ConsoleLogger> m_consoleLogger_;
  std::shared_ptr<Window>        m_window_;
  std::shared_ptr<Renderer>      m_renderer_;

  std::shared_ptr<Editor> m_editor_;

  // TODO: consider adding interface for Game class (for abstraction). add objec
  // in init method parameter
  std::shared_ptr<Game> m_game_;
  // TODO: memory manager

  // ======= END: private misc fields   =======================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_ENGINE_H
