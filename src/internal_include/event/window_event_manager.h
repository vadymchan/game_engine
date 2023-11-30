#ifndef GAME_ENGINE_EVENT_WINDOW_EVENT_MANAGER_H
#define GAME_ENGINE_EVENT_WINDOW_EVENT_MANAGER_H

#include "event.h"
#include "window_event_handler.h"

#include <memory>
#include <utility>

namespace game_engine {

class WindowEventManager {
  public:
  WindowEventManager(std::shared_ptr<WindowEventHandler> windowHandler)
      : m_windowHandler_(std::move(windowHandler)) {}

  void routeEvent(const Event& event) {
    if (event.type == SDL_WINDOWEVENT && m_windowHandler_) {
      m_windowHandler_->dispatch(event.window);
    }
    // TODO: other window event
  }

  private:
  std::shared_ptr<WindowEventHandler> m_windowHandler_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_EVENT_WINDOW_EVENT_MANAGER_H
