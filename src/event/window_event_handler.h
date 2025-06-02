#ifndef ARISE_EVENT_WINDOW_EVENT_HANDLER_H
#define ARISE_EVENT_WINDOW_EVENT_HANDLER_H

#include "event.h"

#include <functional>
#include <unordered_map>
#include <vector>

namespace arise {

class WindowEventHandler {
  public:
  using EventCallback = std::function<void(const WindowEvent&)>;

  void subscribe(WindowEventType eventType, const EventCallback& callback) {
    m_subscribers_[eventType].push_back(callback);
  }

  void dispatch(const WindowEvent& event) {
    auto it = m_subscribers_.find(event.event);
    if (it != m_subscribers_.end()) {
      for (auto& callback : it->second) {
        handleEvent_(event, callback);
      }
    }
  }

  private:
  void handleEvent_(const WindowEvent& event, const EventCallback& callback) { callback(event); }

  std::unordered_map<WindowEventType, std::vector<EventCallback>> m_subscribers_;
};

}  // namespace arise

#endif  // ARISE_EVENT_WINDOW_EVENT_HANDLER_H
