#ifndef GAME_ENGINE_EVENT_KEYBOARD_EVENT_HANDLER_H
#define GAME_ENGINE_EVENT_KEYBOARD_EVENT_HANDLER_H

#include "event.h"
#include "input/key.h"

#include <functional>
#include <unordered_map>
#include <vector>

namespace game_engine {

class KeyboardEventHandler {
  public:
  // TODO: implement both virtual (for writing text) and physical (for control)
  // keys
  struct EventInfo {
    KeyType     m_type;  // SDL_KEYDOWN or SDL_KEYUP
    PhysicalKey m_key;
  };

  using EventCallback = std::function<void(const KeyboardEvent&)>;

  void subscribe(EventInfo eventType, const EventCallback& callback);

  void dispatch(const KeyboardEvent& event);

  private:
  void handleEvent_(const KeyboardEvent& event, const EventCallback& callback);

  std::unordered_map<PhysicalKey, std::vector<EventCallback>> m_keyUpSubscribers_;
  std::unordered_map<PhysicalKey, std::vector<EventCallback>> m_keyDownSubscribers_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_EVENT_KEYBOARD_EVENT_HANDLER_H
