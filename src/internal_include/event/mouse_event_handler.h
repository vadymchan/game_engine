#ifndef GAME_ENGINE_EVENT_MOUSE_EVENT_HANDLER_H
#define GAME_ENGINE_EVENT_MOUSE_EVENT_HANDLER_H

#include "event.h"
#include "input/mouse.h"

#include <functional>
#include <unordered_map>
#include <vector>

namespace game_engine {
class MouseEventHandler {
  public:
  using ButtonEventCallback = std::function<void(const MouseButtonEvent&)>;
  using MotionEventCallback = std::function<void(const MouseMotionEvent&)>;
  using WheelEventCallback  = std::function<void(const MouseWheelEvent&)>;

  struct ButtonEventInfo {
    EventType   m_type;  // SDL_MOUSEBUTTONDOWN or SDL_MOUSEBUTTONUP
    MouseButton m_button;
  };

  struct MotionEventInfo {
    EventType        m_type;  // SDL_MOUSEMOTION
    MouseMotionState m_state;
  };

  struct WheelEventInfo {
    EventType m_type;  // SDL_MOUSEWHEEL
  };

  void subscribe(ButtonEventInfo            eventType,
                 const ButtonEventCallback& callback) {
    auto& subscribers = (eventType.m_type == SDL_MOUSEBUTTONDOWN)
                          ? m_mouseButtonDownSubscribers_
                          : m_mouseButtonUpSubscribers_;
    subscribers[eventType.m_button].emplace_back(callback);
  }

  void subscribe(MotionEventInfo            eventType,
                 const MotionEventCallback& callback) {
    if (eventType.m_type == SDL_MOUSEMOTION) {
      m_mouseMotionSubscribers_[eventType.m_state].emplace_back(callback);
    }
  }

  void subscribe(WheelEventInfo            eventType,
                 const WheelEventCallback& callback) {
    if (eventType.m_type == SDL_MOUSEWHEEL) {
      m_mouseWheelSubscribers_.emplace_back(callback);
    }
  }

  void dispatch(const MouseButtonEvent& event) {
    auto& subscribers = (event.type == SDL_MOUSEBUTTONDOWN)
                          ? m_mouseButtonDownSubscribers_
                          : m_mouseButtonUpSubscribers_;
    auto  it          = subscribers.find(event.button);
    if (it != subscribers.end()) {
      for (auto& callback : it->second) {
        handleEvent_(event, callback);
      }
    }
  }

  void dispatch(const MouseMotionEvent& event) {
    auto& subscribers = m_mouseMotionSubscribers_;
    auto  it          = subscribers.find(event.state);
    if (it != subscribers.end()) {
      for (auto& callback : it->second) {
        handleEvent_(event, callback);
      }
    }
  }

  void dispatch(const MouseWheelEvent& event) {
    auto& subscribers = m_mouseWheelSubscribers_;
    for (auto& callback : subscribers) {
      handleEvent_(event, callback);
    }
  }

  private:

  void handleEvent_(const MouseButtonEvent&         event,
                    const ButtonEventCallback& callback) {
    callback(event);
  }

  void handleEvent_(const MouseMotionEvent&         event,
                    const MotionEventCallback& callback) {
    callback(event);
  }

  void handleEvent_(const MouseWheelEvent&         event,
                    const WheelEventCallback& callback) {
    callback(event);
  }

  std::unordered_map<MouseButton, std::vector<ButtonEventCallback>>
      m_mouseButtonUpSubscribers_;
  std::unordered_map<MouseButton, std::vector<ButtonEventCallback>>
      m_mouseButtonDownSubscribers_;
  std::unordered_map<MouseMotionState, std::vector<MotionEventCallback>>
      m_mouseMotionSubscribers_;

  std::vector<WheelEventCallback&> m_mouseWheelSubscribers_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_EVENT_MOUSE_EVENT_HANDLER_H
