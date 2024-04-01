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
  using EventCallback = std::function<void(const KeyboardEvent&)>;

  struct EventInfo {
    KeyType    m_type;  // SDL_KEYDOWN or SDL_KEYUP
    VirtualKey m_key;
  };

  void subscribe(EventInfo eventType, const EventCallback& callback) {
    auto& subscribers = (eventType.m_type == SDL_KEYDOWN)
                          ? m_keyDownSubscribers_
                          : m_keyUpSubscribers_;
    subscribers[eventType.m_key].emplace_back(callback);
  }

  void dispatch(const KeyboardEvent& event) {
    auto& subscribers = (event.type == SDL_KEYDOWN) ? m_keyDownSubscribers_
                                                    : m_keyUpSubscribers_;
    auto  it          = subscribers.find(event.keysym.sym);
    if (it != subscribers.end()) {
      for (auto& callback : it->second) {
        handleEvent_(event, callback);
      }
    }
  }

  private:
  void handleEvent_(const KeyboardEvent& event, const EventCallback& callback) {
    // Directly execute the event handling logic
    callback(event);
  }

  std::unordered_map<VirtualKey, std::vector<EventCallback>>
      m_keyUpSubscribers_;
  std::unordered_map<VirtualKey, std::vector<EventCallback>>
      m_keyDownSubscribers_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_EVENT_KEYBOARD_EVENT_HANDLER_H
