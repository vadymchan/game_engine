#ifndef GAME_ENGINE_EVENT_APPLICATION_EVENT_HANDLER_H
#define GAME_ENGINE_EVENT_APPLICATION_EVENT_HANDLER_H

#include "event.h"

#include <functional>
#include <unordered_map>
#include <vector>

namespace game_engine {

class ApplicationEventHandler {
  public:
   // ======= BEGIN: public aliases ============================================

using EventCallback = std::function<void(const ApplicationEvent&)>;


  // ======= END: public aliases   ============================================
  
  // ======= BEGIN: public misc methods =======================================

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

  // ======= END: public misc methods   =======================================



  private:
    // ======= BEGIN: private misc methods ======================================

void handleEvent_(const ApplicationEvent& event, const EventCallback& callback) {
    callback(event);
  }

  // ======= END: private misc methods   ======================================
  
  // ======= BEGIN: private misc fields =======================================

  std::unordered_map<ApplicationEventType, std::vector<EventCallback>>
      m_subscribers_;

  // ======= END: private misc fields   =======================================



};

}  // namespace game_engine

#endif  // GAME_ENGINE_EVENT_APPLICATION_EVENT_HANDLER_H
