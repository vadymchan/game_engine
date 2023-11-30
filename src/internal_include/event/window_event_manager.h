#ifndef GAME_ENGINE_EVENT_WINDOW_EVENT_MANAGER_H
#define GAME_ENGINE_EVENT_WINDOW_EVENT_MANAGER_H

#include "event.h"
#include "window_event_handler.h"

namespace game_engine {

class WindowEventManager {
  public:
  WindowEventManager(WindowEventHandler* windowHandler)
      : m_windowHandler_(windowHandler) {}

  void routeEvent(const Event& event) {
    if (event.type == SDL_WINDOWEVENT) {
      m_windowHandler_->dispatch(event.window);
    }
    // TODO: other window event
  }

  private:
  // TODO : consider using unique_ptr
  WindowEventHandler* m_windowHandler_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_EVENT_WINDOW_EVENT_MANAGER_H
