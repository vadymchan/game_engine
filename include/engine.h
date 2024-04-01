#ifndef GAME_ENGINE_ENGINE_H
#define GAME_ENGINE_ENGINE_H

#include "event/application_event_handler.h"
#include "event/application_event_manager.h"
#include "event/keyboard_event_handler.h"
#include "event/mouse_event_handler.h"
#include "event/window_event_handler.h"
#include "event/window_event_manager.h"
#include "game.h"
#include "gfx/rhi/vulkan/rhi_vk.h"
#include "input/input_manager.h"
#include "platform/common/window.h"
#include "utils/logger/console_logger.h"
#include "utils/logger/global_logger.h"
#include "utils/time/stopwatch.h"

#include <memory>

namespace game_engine {

class Engine {
  public:
  Engine() = default;

  Engine(const Engine&)                     = delete;
  auto operator=(const Engine&) -> Engine&  = delete;
  Engine(Engine&&)                          = delete;
  auto operator=(const Engine&&) -> Engine& = delete;

  ~Engine() = default;

  // TODO: add config file
  auto init() -> bool {
    bool successfullyInited{true};


    // logger
    // ------------------------------------------------------------------------
    m_consoleLogger_ = std::make_shared<ConsoleLogger>();
    GlobalLogger::AddLogger(m_consoleLogger_);

    // window
    // ------------------------------------------------------------------------
    m_window_ = std::make_unique<Window>(
        "First Triangle Vulkan",
        math::Dimension2Di{800, 600},
        math::Point2Di{100, 100},
        game_engine::Window::Flags::Resizable
            // TODO: Vulkan flag may not work for DX12 window
            | game_engine::Window::Flags::Vulkan);
    // game
    // ------------------------------------------------------------------------
    m_game_ = std::make_unique<Game>(m_window_);

    // input event
    // ------------------------------------------------------------------------
    m_keyboardEventHandler_ = std::make_shared<KeyboardEventHandler>();
    m_mouseEventHandler_    = std::make_shared<MouseEventHandler>();
    m_inputManager_ = std::make_unique<InputManager>(m_keyboardEventHandler_,
                                                     m_mouseEventHandler_);
    // window event
    // ------------------------------------------------------------------------
    m_windowEventHandler_ = std::make_shared<WindowEventHandler>();
    m_windowEventHandler_->subscribe(
        SDL_WINDOWEVENT_RESIZED, [this](const WindowEvent& event) {
          this->m_window_->onResize(event);
          this->m_game_->Resize(event.data1, event.data2);  // TODO: refactor
          g_rhi_vk->OnHandleResized(
              event.data1, event.data2, false);             // TODO: refactor
        });
    m_windowEventManager_
        = std::make_unique<WindowEventManager>(m_windowEventHandler_);

    // application event
    // ------------------------------------------------------------------------
    m_applicationEventHandler_ = std::make_shared<ApplicationEventHandler>();
    m_applicationEventHandler_->subscribe(
        SDL_QUIT, std::bind(&Engine::onClose, this, std::placeholders::_1));
    // [this](const ApplicationEvent& event) { this->onClose(event); }
    m_applicationEventManager_
        = std::make_unique<ApplicationEventManager>(m_applicationEventHandler_);

    g_rhi_vk = new RhiVk();
    g_rhi_vk->init(m_window_);
    g_rhi_vk->OnInitRHI();

    m_game_->Setup();

    return successfullyInited;
  }

  void run() {
    m_deltaTime_.start();
    m_isRunning_ = true;

    while (m_isRunning_) {
      float deltaTime = m_deltaTime_.elapsedTime<DeltaTime::DurationFloat<>>();
      m_deltaTime_.reset();

      processEvents_();
      m_game_->ProcessInput();  // TODO: consider remove
      update_(deltaTime);
      m_game_->Draw();
      // render();
    }
  }

  void release() {
    if (g_rhi_vk) {
      g_rhi_vk->Flush();
    }

    m_game_->Release();
  }

  void onClose(const ApplicationEvent& event) { m_isRunning_ = false; }

  private:
  void processEvents_() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      m_inputManager_->routeEvent(event);
      m_windowEventManager_->routeEvent(event);
      m_applicationEventManager_->routeEvent(event);
    }
  }

  void update_(float deltaTime) {
    m_game_->Update(deltaTime);

    Shader::StartAndRunCheckUpdateShaderThread();
  }

  bool                           m_isRunning_{false};
  DeltaTime                      m_deltaTime_;
  std::shared_ptr<ConsoleLogger> m_consoleLogger_;
  std::shared_ptr<Window>        m_window_;

  std::shared_ptr<KeyboardEventHandler> m_keyboardEventHandler_;
  std::shared_ptr<MouseEventHandler>    m_mouseEventHandler_;
  std::unique_ptr<InputManager>         m_inputManager_;

  std::shared_ptr<WindowEventHandler> m_windowEventHandler_;
  std::unique_ptr<WindowEventManager> m_windowEventManager_;

  std::shared_ptr<ApplicationEventHandler> m_applicationEventHandler_;
  std::unique_ptr<ApplicationEventManager> m_applicationEventManager_;

  std::unique_ptr<Game> m_game_;
  // TODO: memory manager
};

}  // namespace game_engine

#endif