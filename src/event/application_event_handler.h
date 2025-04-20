#ifndef GAME_ENGINE_EVENT_APPLICATION_EVENT_HANDLER_H
#define GAME_ENGINE_EVENT_APPLICATION_EVENT_HANDLER_H

#include "event.h"

#include <functional>
#include <unordered_map>
#include <vector>

namespace game_engine {

class ApplicationEventHandler {
  public:
  using EventCallback = std::function<void(const ApplicationEvent&)>;

  void subscribe(ApplicationEventType eventType, const EventCallback& callback) {
    m_subscribers_[eventType].push_back(callback);
  }

  void dispatch(const ApplicationEvent& event) {
    auto it = m_subscribers_.find(event.type);
    if (it != m_subscribers_.end()) {
      for (auto& callback : it->second) {
        handleEvent_(event, callback);
      }
    }
  }

  private:
  void handleEvent_(const ApplicationEvent& event, const EventCallback& callback) { callback(event); }

  std::unordered_map<ApplicationEventType, std::vector<EventCallback>> m_subscribers_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_EVENT_APPLICATION_EVENT_HANDLER_H
