#ifndef ARISE_EVENT_EVENT_HANDLER_H
#define ARISE_EVENT_EVENT_HANDLER_H

#include "event.h"

#include <functional>
#include <unordered_map>
#include <vector>

namespace arise {

class EventHandler {
  public:
  using EventCallback = std::function<void(const Event&)>;

  void subscribe(EventType eventType, const EventCallback& callback) { m_subscribers_[eventType].push_back(callback); }

  void dispatch(const Event& event) {
    // This method could potentially route to different handlers,
    // but for now, it's directly executing the callbacks.
    auto it = m_subscribers_.find(static_cast<EventType>(event.type));
    if (it != m_subscribers_.end()) {
      for (auto& callback : it->second) {
        handleEvent_(event, callback);
      }
    }
  }

  private:
  void handleEvent_(const Event& event, const EventCallback& callback) {
    // Directly execute the event handling logic
    callback(event);
  }

  std::unordered_map<EventType, std::vector<EventCallback>> m_subscribers_;
};

}  // namespace arise

#endif  // ARISE_EVENT_EVENT_HANDLER_H
