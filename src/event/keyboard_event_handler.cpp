#include "event/keyboard_event_handler.h"

#include "input/input_manager.h"
#include "utils/service/service_locator.h"

namespace game_engine {

void KeyboardEventHandler::subscribe(EventInfo eventType, const EventCallback& callback) {
  auto& subscribers = (eventType.m_type == SDL_KEYDOWN) ? m_keyDownSubscribers_ : m_keyUpSubscribers_;
  subscribers[eventType.m_key].emplace_back(callback);
}

void KeyboardEventHandler::dispatch(const KeyboardEvent& event) {
  auto& subscribers = (event.type == SDL_KEYDOWN) ? m_keyDownSubscribers_ : m_keyUpSubscribers_;
  auto  it          = subscribers.find(event.keysym.scancode);
  if (it != subscribers.end()) {
    for (auto& callback : it->second) {
      handleEvent_(event, callback);
    }
  }
}

void KeyboardEventHandler::handleEvent_(const KeyboardEvent& event, const EventCallback& callback) {
  auto inputManager = ServiceLocator::s_get<InputManager>();
  if (!inputManager->shouldProcessGameInput()) {
    return;
  }
  callback(event);
}

}  // namespace game_engine