#ifndef GAME_ENGINE_EVENT_EVENT_HANDLER_H
#define GAME_ENGINE_EVENT_EVENT_HANDLER_H

#include "event.h"

#include <functional>
#include <unordered_map>
#include <vector>

namespace game_engine {

class EventHandler {
  public:
         // ======= BEGIN: public aliases ============================================

  using EventCallback = std::function<void(const Event&)>;

  // ======= END: public aliases   ============================================
  
  // ======= BEGIN: public misc methods =======================================

  void subscribe(EventType eventType, const EventCallback& callback) {
    m_subscribers_[eventType].push_back(callback);
  }

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

  // ======= END: public misc methods   =======================================




  private:
          // ======= BEGIN: private misc methods ======================================

  void handleEvent_(const Event& event, const EventCallback& callback) {
    // Directly execute the event handling logic
    callback(event);
  }

  // ======= END: private misc methods   ======================================
  
  // ======= BEGIN: private misc fields =======================================

  std::unordered_map<EventType, std::vector<EventCallback>> m_subscribers_;

  // ======= END: private misc fields   =======================================



};

}  // namespace game_engine

#endif  // GAME_ENGINE_EVENT_EVENT_HANDLER_H
