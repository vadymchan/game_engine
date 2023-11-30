#ifndef GAME_ENGINE_INPUT_MANAGER
#define GAME_ENGINE_INPUT_MANAGER

#include "event/event.h"
#include "event/keyboard_event_handler.h"
#include "event/mouse_event_handler.h"
#include "utils/logger/global_logger.h"

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
          m_keyboardHandler_->dispatch(event.key);
        }
        break;

      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        if (m_mouseHandler_) {
          m_mouseHandler_->dispatch(event.button);
        }
        break;
      case SDL_MOUSEMOTION:
        if (m_mouseHandler_) {
          m_mouseHandler_->dispatch(event.motion);
        }
        break;
      case SDL_MOUSEWHEEL:
        if (m_mouseHandler_) {
          m_mouseHandler_->dispatch(event.wheel);
        }
        break;

        // TODO: Add cases for joystick/gamepad events

      default:
        GlobalLogger::Log(LogLevel::Warning,
                          "Received an unhandled event type.");
        break;
    }
  }

  private:
  KeyboardEventHandler* m_keyboardHandler_;
  MouseEventHandler*    m_mouseHandler_;
};

}  // namespace game_engine

#endif
