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

  class ButtonEventHandler {
    public:
    struct EventInfo {
      EventType   m_type;  // SDL_MOUSEBUTTONDOWN or SDL_MOUSEBUTTONUP
      MouseButton m_button;
    };

    using EventCallback = std::function<void(const MouseButtonEvent&)>;

    void subscribe(const EventInfo& eventInfo, const EventCallback& callback) {
      auto& subscribers = (eventInfo.m_type == SDL_MOUSEBUTTONDOWN)
                            ? m_mouseButtonDownSubscribers
                            : m_mouseButtonUpSubscribers;
      subscribers[eventInfo.m_button].emplace_back(callback);
    }

    void dispatch(const MouseButtonEvent& event) {
      auto& subscribers = (event.type == SDL_MOUSEBUTTONDOWN)
                            ? m_mouseButtonDownSubscribers
                            : m_mouseButtonUpSubscribers;
      auto  it          = subscribers.find(event.button);
      if (it != subscribers.end()) {
        for (auto& callback : it->second) {
          callback(event);
        }
      }
    }

    private:
    std::unordered_map<MouseButton, std::vector<EventCallback>>
        m_mouseButtonDownSubscribers;
    std::unordered_map<MouseButton, std::vector<EventCallback>>
        m_mouseButtonUpSubscribers;
  };

  class MotionEventHandler {
    public:
    struct EventInfo {
      EventType        m_type;  // SDL_MOUSEMOTION
      MouseMotionState m_state;
    };

    using EventCallback = std::function<void(const MouseMotionEvent&)>;

    void subscribe(const EventInfo& eventInfo, const EventCallback& callback) {
      if (eventInfo.m_type == SDL_MOUSEMOTION) {
        m_mouseMotionSubscribers[eventInfo.m_state].emplace_back(callback);
      }
    }

    void dispatch(const MouseMotionEvent& event) {
      auto it = m_mouseMotionSubscribers.find(event.state);
      if (it != m_mouseMotionSubscribers.end()) {
        for (auto& callback : it->second) {
          callback(event);
        }
      }
    }

    private:
    std::unordered_map<MouseMotionState, std::vector<EventCallback>>
        m_mouseMotionSubscribers;
  };

  class WheelEventHandler {
    public:
    struct EventInfo {
      EventType m_type;  // SDL_MOUSEWHEEL
    };

    using EventCallback = std::function<void(const MouseWheelEvent&)>;

    void subscribe(const EventInfo& eventInfo, const EventCallback& callback) {
      if (eventInfo.m_type == SDL_MOUSEWHEEL) {
        m_mouseWheelSubscribers.emplace_back(callback);
      }
    }

    void dispatch(const MouseWheelEvent& event) {
      for (auto& callback : m_mouseWheelSubscribers) {
        callback(event);
      }
    }

    private:
    std::vector<EventCallback> m_mouseWheelSubscribers;
  };

  void subscribe(const ButtonEventHandler::EventInfo&     eventInfo,
                 const ButtonEventHandler::EventCallback& callback) {
    m_buttonEventHandler_.subscribe(eventInfo, callback);
  }

  void subscribe(const MotionEventHandler::EventInfo&     eventInfo,
                 const MotionEventHandler::EventCallback& callback) {
    m_motionEventHandler_.subscribe(eventInfo, callback);
  }

  void subscribe(const WheelEventHandler::EventInfo&     eventInfo,
                 const WheelEventHandler::EventCallback& callback) {
    m_wheelEventHandler_.subscribe(eventInfo, callback);
  }

  void dispatch(const MouseButtonEvent& event) {
    m_buttonEventHandler_.dispatch(event);
  }

  void dispatch(const MouseMotionEvent& event) {
    m_motionEventHandler_.dispatch(event);
  }

  void dispatch(const MouseWheelEvent& event) {
    m_wheelEventHandler_.dispatch(event);
  }

  private:
  ButtonEventHandler m_buttonEventHandler_;
  MotionEventHandler m_motionEventHandler_;
  WheelEventHandler  m_wheelEventHandler_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_EVENT_MOUSE_EVENT_HANDLER_H
