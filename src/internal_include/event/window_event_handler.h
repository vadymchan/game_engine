#ifndef GAME_ENGINE_EVENT_WINDOW_EVENT_HANDLER_H
#define GAME_ENGINE_EVENT_WINDOW_EVENT_HANDLER_H

#include "event.h"

#include <functional>
#include <unordered_map>
#include <vector>

namespace game_engine {

class WindowEventHandler {
  public:
  // ======= BEGIN: public aliases ============================================

  using EventCallback = std::function<void(const WindowEvent&)>;

  // ======= END: public aliases   ============================================

  // ======= BEGIN: public misc methods =======================================

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

  // ======= END: public misc methods   =======================================

  private:
  // ======= BEGIN: private misc methods ======================================

  void handleEvent_(const WindowEvent& event, const EventCallback& callback) {
    callback(event);
  }

  // ======= END: private misc methods   ======================================

  // ======= BEGIN: private misc fields =======================================

  std::unordered_map<WindowEventType, std::vector<EventCallback>>
      m_subscribers_;

  // ======= END: private misc fields   =======================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_EVENT_WINDOW_EVENT_HANDLER_H
