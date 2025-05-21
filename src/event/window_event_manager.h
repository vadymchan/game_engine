#ifndef GAME_ENGINE_EVENT_WINDOW_EVENT_MANAGER_H
#define GAME_ENGINE_EVENT_WINDOW_EVENT_MANAGER_H

#include "event.h"
#include "window_event_handler.h"

#include <memory>

namespace game_engine {

class WindowEventManager {
  public:
  WindowEventManager() = default;

  WindowEventManager(std::unique_ptr<WindowEventHandler> windowHandler)
      : m_windowHandler_(std::move(windowHandler)) {}

  void routeEvent(const Event& event) {
    if (event.type == SDL_WINDOWEVENT && m_windowHandler_) {
      m_windowHandler_->dispatch(event.window);
    }
  }

  WindowEventHandler* getWindowHandler() const { return m_windowHandler_.get(); }

  void setWindowHandler(std::unique_ptr<WindowEventHandler> handler) { m_windowHandler_ = std::move(handler); }

  private:
  std::unique_ptr<WindowEventHandler> m_windowHandler_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_EVENT_WINDOW_EVENT_MANAGER_H