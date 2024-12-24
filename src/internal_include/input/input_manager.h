#ifndef GAME_ENGINE_INPUT_MANAGER
#define GAME_ENGINE_INPUT_MANAGER

#include "event/event.h"
#include "event/keyboard_event_handler.h"
#include "event/mouse_event_handler.h"
#include "utils/logger/global_logger.h"

#include <memory>

namespace game_engine {

class InputManager {
  public:
  InputManager(const std::shared_ptr<KeyboardEventHandler>& keyboardHandler
               = nullptr,
               const std::shared_ptr<MouseEventHandler>& mouseHandler = nullptr)
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

  [[nodiscard]] auto getKeyboardHandler() const
      -> std::shared_ptr<KeyboardEventHandler> {
    return m_keyboardHandler_;
  }

  [[nodiscard]] auto getMouseHandler() const
      -> std::shared_ptr<MouseEventHandler> {
    return m_mouseHandler_;
  }

  private:
  std::shared_ptr<KeyboardEventHandler> m_keyboardHandler_;
  std::shared_ptr<MouseEventHandler>    m_mouseHandler_;
};

}  // namespace game_engine

#endif
