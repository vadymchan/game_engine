#ifndef GAME_ENGINE_INPUT_MANAGER
#define GAME_ENGINE_INPUT_MANAGER

#include "event/event.h"
#include "event/keyboard_event_handler.h"
#include "event/mouse_event_handler.h"

namespace game_engine {

class InputManager {
  public:
  InputManager(KeyboardEventHandler* keyboardHandler = nullptr,
               MouseEventHandler*    mouseHandler    = nullptr)
      : m_keyboardHandler_(keyboardHandler)
      , m_mouseHandler_(mouseHandler) {}

  void routeEvent(const Event& event) {
    switch (event.type) {
      case SDL_KEYDOWN:
      case SDL_KEYUP:
        if (m_keyboardHandler_) {
          m_keyboardHandler_->dispatch(event);
        }
        break;

      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
      case SDL_MOUSEMOTION:
        if (m_mouseHandler_) {
          m_mouseHandler_->dispatch(event);
        }
        break;

        // TODO: Add cases for joystick/gamepad events
    }
  }

  private:
  KeyboardEventHandler* m_keyboardHandler_;
  MouseEventHandler*    m_mouseHandler_;
};

}  // namespace game_engine

#endif