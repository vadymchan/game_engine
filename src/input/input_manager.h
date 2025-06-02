#ifndef GAME_ENGINE_INPUT_MANAGER
#define GAME_ENGINE_INPUT_MANAGER

#include "event/event.h"
#include "event/keyboard_event_handler.h"
#include "event/mouse_event_handler.h"
#include "input/input_context.h"
#include "utils/logger/global_logger.h"

#include <memory>

namespace game_engine {

class InputManager {
  public:
  InputManager() = default;

  InputManager(std::unique_ptr<KeyboardEventHandler> keyboardHandler, std::unique_ptr<MouseEventHandler> mouseHandler)
      : m_keyboardHandler_(std::move(keyboardHandler))
      , m_mouseHandler_(std::move(mouseHandler)) {}

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
        GlobalLogger::Log(LogLevel::Warning, "Received an unhandled event type.");
        break;
    }
  }

  KeyboardEventHandler* getKeyboardHandler() const { return m_keyboardHandler_.get(); }

  MouseEventHandler* getMouseHandler() const { return m_mouseHandler_.get(); }

  void setContextManager(InputContextManager* contextManager) { m_contextManager = contextManager; }

  bool shouldProcessGameInput() const {
    return !m_contextManager || m_contextManager->isContextActive(InputContext::Game);
  }

  private:
  InputContextManager*                  m_contextManager = nullptr;
  std::unique_ptr<KeyboardEventHandler> m_keyboardHandler_;
  std::unique_ptr<MouseEventHandler>    m_mouseHandler_;
};

}  // namespace game_engine

#endif