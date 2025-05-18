#ifndef GAME_ENGINE_ENGINE_H
#define GAME_ENGINE_ENGINE_H

#include "config/config_manager.h"
#include "config/runtime_settings.h"
#include "core/game.h"
#include "ecs/component_loaders.h"
#include "ecs/components/camera.h"
#include "ecs/systems/camera_system.h"
#include "ecs/systems/light_system.h"
#include "ecs/systems/movement_system.h"
#include "ecs/systems/render_system.h"
#include "ecs/systems/system_manager.h"
#include "editor/editor.h"
#include "event/application_event_manager.h"
#include "event/window_event_manager.h"
#include "gfx/renderer/render_resource_manager.h"
#include "gfx/renderer/renderer.h"
#include "gfx/rhi/shader_manager.h"
#include "input/input_manager.h"
#include "platform/common/window.h"
#include "resources/assimp/assimp_material_loader.h"
#include "resources/assimp/assimp_render_model_loader.h"
#include "resources/cgltf/cgltf_material_loader.h"
#include "resources/cgltf/cgltf_render_model_loader.h"
#include "scene/scene_loader.h"
#include "scene/scene_manager.h"
#include "utils/buffer/buffer_manager.h"
#include "utils/hot_reload/hot_reload_manager.h"
#include "utils/image/image_loader_manager.h"
#include "utils/image/image_manager.h"
#include "utils/logger/console_logger.h"
#include "utils/logger/file_logger.h"
#include "utils/logger/global_logger.h"
#include "utils/logger/memory_logger.h"
#include "utils/material/material_manager.h"
#include "utils/math/math_util.h"
#include "utils/model/mesh_manager.h"
#include "utils/model/render_geometry_mesh_manager.h"
#include "utils/model/render_mesh_manager.h"
#include "utils/model/render_model_loader_manager.h"
#include "utils/model/render_model_manager.h"
#include "utils/path_manager/path_manager.h"
#include "utils/service/service_locator.h"
#include "utils/texture/texture_manager.h"
#include "utils/third_party/directx_tex_util.h"
#include "utils/third_party/ktx_image_loader.h"
#include "utils/third_party/stb_util.h"
#include "utils/time/stopwatch.h"
#include "utils/time/timing_manager.h"

// TODO: consider moving this to another file

#include <imgui_impl_sdl2.h>

#include <memory>

namespace game_engine {

class Engine {
  public:
  Engine()              = default;
  Engine(const Engine&) = delete;
  Engine(Engine&&)      = delete;

  ~Engine() {
    m_game_->release();

    if (m_renderer_) {
      m_renderer_->getDevice()->waitIdle();
    }

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
    // ServiceLocator::s_remove<gfx::rhi::ShaderManager>();
    ServiceLocator::s_remove<gfx::renderer::RenderResourceManager>();
    ServiceLocator::s_remove<TextureManager>();
    ServiceLocator::s_remove<BufferManager>();

    GlobalLogger::Shutdown();
  }

  Window* getWindow() const { return m_window_.get(); }

  void setGame(Game* game) {
    m_game_ = game;
    if (m_game_) {
      m_game_->linkWithEngine(*this);
    }
  }

  // TODO: add config file
  auto initialize() -> bool {
    bool successfullyInitialized{true};

    // logger
    // ------------------------------------------------------------------------
    auto consoleLogger = std::make_unique<ConsoleLogger>("console_logger");
    GlobalLogger::AddLogger(std::move(consoleLogger));

    // auto memoryLogger = std::make_unique<MemoryLogger>("memory_logger");
    // GlobalLogger::AddLogger(std::move(memoryLogger));

    // auto fileLogger = std::make_unique<FileLogger>("file_logger");
    // GlobalLogger::AddLogger(std::move(fileLogger));

    GlobalLogger::Log(LogLevel::Info, "Engine::initialize() started");

    // input event
    // ------------------------------------------------------------------------
    auto keyboardEventHandler = std::make_unique<KeyboardEventHandler>();
    auto mouseEventHandler    = std::make_unique<MouseEventHandler>();

    // window event
    // ------------------------------------------------------------------------
    auto windowEventHandler = std::make_unique<WindowEventHandler>();
    windowEventHandler->subscribe(SDL_WINDOWEVENT_RESIZED, [this](const WindowEvent& event) {
      auto renderMode = m_editor_->getRenderParams().appMode;

      auto device = m_renderer_->getDevice();
      if (device) {
        device->waitIdle();
      }

      if (renderMode == gfx::renderer::ApplicationRenderMode::Game) {
        this->m_window_->onResize(event);

        auto  scene    = ServiceLocator::s_get<SceneManager>()->getCurrentScene();
        auto& registry = scene->getEntityRegistry();
        auto  view     = registry.view<Camera>();
        auto  entity   = view.front();
        if (entity != entt::null) {
          auto& camera  = view.get<Camera>(entity);
          camera.width  = event.data1;
          camera.height = event.data2;
        }

        m_renderer_->onWindowResize(event.data1, event.data2);
      } else if (renderMode == gfx::renderer::ApplicationRenderMode::Editor) {
        this->m_window_->onResize(event);
        m_renderer_->onWindowResize(event.data1, event.data2);
        m_editor_->onWindowResize(event.data1, event.data2);

        GlobalLogger::Log(LogLevel::Info, "Window resize in editor mode - ignoring renderer resize");
      }
    });

    // application event
    // ------------------------------------------------------------------------
    auto applicationEventHandler = std::make_unique<ApplicationEventHandler>();
    applicationEventHandler->subscribe(SDL_QUIT, std::bind(&Engine::onClose, this, std::placeholders::_1));

    // service locator
    // ------------------------------------------------------------------------
    ServiceLocator::s_provide<ConfigManager>();
    ServiceLocator::s_provide<FileWatcherManager>();
    ServiceLocator::s_provide<HotReloadManager>();
    ServiceLocator::s_provide<InputManager>(std::move(keyboardEventHandler), std::move(mouseEventHandler));
    ServiceLocator::s_provide<WindowEventManager>(std::move(windowEventHandler));
    ServiceLocator::s_provide<ApplicationEventManager>(std::move(applicationEventHandler));
    ServiceLocator::s_provide<SceneManager>();
    ServiceLocator::s_provide<SystemManager>();
    ServiceLocator::s_provide<TimingManager>();
    ServiceLocator::s_provide<gfx::renderer::RenderResourceManager>();

    // config
    // ------------------------------------------------------------------------
    auto configManager = ServiceLocator::s_get<ConfigManager>();
    auto configPath    = PathManager::s_getEngineSettingsPath() / "settings.json";
    configManager->addConfig(configPath);
    auto config = configManager->getConfig(configPath);

    auto renderingApiString = config->get<std::string>("renderingApi");

    gfx::rhi::RenderingApi renderingApi = gfx::rhi::RenderingApi::Vulkan;
    if (renderingApiString == "dx12") {
      renderingApi = gfx::rhi::RenderingApi::Dx12;
    } else if (renderingApiString == "vulkan") {
      renderingApi = gfx::rhi::RenderingApi::Vulkan;
    }

    auto applicationModeStr = config->get<std::string>("applicationMode");

    // window
    // ------------------------------------------------------------------------
    m_window_ = std::make_unique<Window>(renderingApiString,
                                         // Desired size (for maximized window will be 0)
                                         math::Dimension2Di{0, 0},
                                         math::Point2Di{SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED},
                                         game_engine::Window::Flags::Resizable | game_engine::Window::Flags::Vulkan
                                             | game_engine::Window::Flags::Maximized);

    // ecs
    // ------------------------------------------------------------------------
    auto systemManager = ServiceLocator::s_get<SystemManager>();
    systemManager->addSystem(std::make_unique<CameraSystem>());
    systemManager->addSystem(std::make_unique<MovementSystem>());
    systemManager->addSystem(std::make_unique<RenderSystem>());

    // renderer
    // ------------------------------------------------------------------------
    m_renderer_ = std::make_unique<gfx::renderer::Renderer>();
    m_renderer_->initialize(m_window_.get(), renderingApi);

    // These managers are depending on the renderer device
    auto device = m_renderer_->getDevice();
    // ServiceLocator::s_provide<gfx::rhi::ShaderManager>(device, false);
    ServiceLocator::s_provide<TextureManager>(device);
    ServiceLocator::s_provide<BufferManager>(device);

    systemManager->addSystem(std::make_unique<LightSystem>(device, m_renderer_->getResourceManager()));

    // image loader
    // ------------------------------------------------------------------------
    auto imageLoaderManager = std::make_unique<ImageLoaderManager>();
    auto stbLoader          = std::make_shared<STBImageLoader>();
    imageLoaderManager->registerLoader(ImageType::JPEG, stbLoader);
    imageLoaderManager->registerLoader(ImageType::JPG, stbLoader);
    imageLoaderManager->registerLoader(ImageType::PNG, stbLoader);
    imageLoaderManager->registerLoader(ImageType::BMP, stbLoader);
    imageLoaderManager->registerLoader(ImageType::TGA, stbLoader);
    imageLoaderManager->registerLoader(ImageType::GIF, stbLoader);
    imageLoaderManager->registerLoader(ImageType::HDR, stbLoader);
    imageLoaderManager->registerLoader(ImageType::PIC, stbLoader);
    imageLoaderManager->registerLoader(ImageType::PPM, stbLoader);
    imageLoaderManager->registerLoader(ImageType::PGM, stbLoader);
    auto directxTexLoader = std::make_shared<DirectXTexImageLoader>();
    imageLoaderManager->registerLoader(ImageType::DDS, directxTexLoader);
    auto khronosTexLoader = std::make_shared<KtxImageLoader>();
    imageLoaderManager->registerLoader(ImageType::KTX, khronosTexLoader);
    imageLoaderManager->registerLoader(ImageType::KTX2, khronosTexLoader);
    ServiceLocator::s_provide<ImageLoaderManager>(std::move(imageLoaderManager));

    auto imageManager = std::make_unique<ImageManager>();
    ServiceLocator::s_provide<ImageManager>(std::move(imageManager));

    // mesh / model / material loader
    // ------------------------------------------------------------------------
    ServiceLocator::s_provide<MeshManager>();
    ServiceLocator::s_provide<RenderMeshManager>();
    ServiceLocator::s_provide<RenderGeometryMeshManager>();

    auto renderModelLoaderManager = std::make_unique<RenderModelLoaderManager>();
#ifdef GAME_ENGINE_USE_ASSIMP
    auto assimpModelLoader = std::make_shared<AssimpRenderModelLoader>();
    renderModelLoaderManager->registerLoader(ModelType::OBJ, assimpModelLoader);
    renderModelLoaderManager->registerLoader(ModelType::FBX, assimpModelLoader);
#endif  // GAME_ENGINE_USE_ASSIMP
    auto cgltfModelLoader = std::make_shared<CgltfRenderModelLoader>();
    renderModelLoaderManager->registerLoader(ModelType::GLTF, cgltfModelLoader);
    renderModelLoaderManager->registerLoader(ModelType::GLB, cgltfModelLoader);
    ServiceLocator::s_provide<RenderModelLoaderManager>(std::move(renderModelLoaderManager));

    auto renderModelManager = std::make_unique<RenderModelManager>();
    ServiceLocator::s_provide<RenderModelManager>(std::move(renderModelManager));

    ServiceLocator::s_provide<MaterialManager>();
    auto materialLoaderManager = std::make_unique<MaterialLoaderManager>();
#ifdef GAME_ENGINE_USE_ASSIMP
    auto assimpMaterialLoader = std::make_shared<AssimpMaterialLoader>();
    materialLoaderManager->registerLoader(MaterialType::MTL, assimpMaterialLoader);
    materialLoaderManager->registerLoader(MaterialType::FBX, assimpMaterialLoader);
#endif  // GAME_ENGINE_USE_ASSIMP
    auto cgltfMaterialLoader = std::make_shared<CgltfMaterialLoader>();
    materialLoaderManager->registerLoader(MaterialType::GLTF, cgltfMaterialLoader);
    ServiceLocator::s_provide<MaterialLoaderManager>(std::move(materialLoaderManager));

    // editor
    // ------------------------------------------------------------------------

    gfx::renderer::ApplicationRenderMode applicationMode = gfx::renderer::ApplicationRenderMode::Game;
    if (applicationModeStr == "editor") {
      m_applicationMode = gfx::renderer::ApplicationRenderMode::Editor;
    } else if (applicationModeStr == "game") {
      m_applicationMode = gfx::renderer::ApplicationRenderMode::Game;
    }

    m_editor_ = std::make_unique<Editor>();
    switch (m_applicationMode) {
      case gfx::renderer::ApplicationRenderMode::Editor:
        if (!m_editor_->initialize(
                m_window_.get(), renderingApi, m_renderer_->getDevice(), m_renderer_->getFrameResources())) {
          GlobalLogger::Log(LogLevel::Error, "Failed to initialize Editor");
          successfullyInitialized = false;
        }
        GlobalLogger::Log(LogLevel::Info, "Editor initialized successfully");
        break;
      case gfx::renderer::ApplicationRenderMode::Game:
        GlobalLogger::Log(LogLevel::Info, "Running in Game mode, Editor not initialized");
        break;
      default:
        GlobalLogger::Log(LogLevel::Error, "Unknown application mode");
        successfullyInitialized = false;
        break;
    }

    if (successfullyInitialized) {
      GlobalLogger::Log(LogLevel::Info, "Engine::initialize() completed");
    }

    return successfullyInitialized;
  }

  void render() {
    auto windowSize = m_window_->getSize();
    if (windowSize.width() == 0 || windowSize.height() == 0 || m_window_->isMinimized()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      return;
    }

    auto& renderSettings = m_editor_->getRenderParams();

    auto context = m_renderer_->beginFrame(ServiceLocator::s_get<SceneManager>()->getCurrentScene(), renderSettings);

    m_renderer_->renderFrame(context);

    if (renderSettings.appMode == gfx::renderer::ApplicationRenderMode::Editor) {
      m_editor_->render(context);
    }

    m_renderer_->endFrame(context);
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

  auto operator=(const Engine&) -> Engine&  = delete;
  auto operator=(const Engine&&) -> Engine& = delete;

  private:
  void processEvents_() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (m_applicationMode == gfx::renderer::ApplicationRenderMode::Editor) {
        ImGui_ImplSDL2_ProcessEvent(&event);
      }
      ServiceLocator::s_get<InputManager>()->routeEvent(event);
      ServiceLocator::s_get<WindowEventManager>()->routeEvent(event);
      ServiceLocator::s_get<ApplicationEventManager>()->routeEvent(event);
    }
  }

  void update_(float deltaTime) {
    auto systemManager = ServiceLocator::s_get<SystemManager>();
    auto scene         = ServiceLocator::s_get<SceneManager>()->getCurrentScene();
    systemManager->updateSystems(scene, deltaTime);

    m_game_->update(deltaTime);
  }

  bool                                     m_isRunning_{false};
  gfx::renderer::ApplicationRenderMode     m_applicationMode = gfx::renderer::ApplicationRenderMode::Game;
  std::unique_ptr<Window>                  m_window_;
  std::unique_ptr<gfx::renderer::Renderer> m_renderer_;
  std::unique_ptr<Editor>                  m_editor_;

  // TODO: consider adding interface for Game class (for abstraction). add objec
  // in initialize method parameter
  Game* m_game_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_ENGINE_H
