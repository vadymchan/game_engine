#ifndef GAME_ENGINE_EVENT_APPLICATION_EVENT_MANAGER_H
#define GAME_ENGINE_EVENT_APPLICATION_EVENT_MANAGER_H

#include "application_event_handler.h"
#include "event.h"

#include <memory>

namespace game_engine {

class ApplicationEventManager {
  public:
  ApplicationEventManager() = default;

  ApplicationEventManager(std::unique_ptr<ApplicationEventHandler> applicationHandler)
      : m_applicationHandler_(std::move(applicationHandler)) {}

  void routeEvent(const Event& event) {
    if (event.type == SDL_QUIT && m_applicationHandler_) {
      m_applicationHandler_->dispatch(event.quit);
    }
    // TODO: other application events
  }

  ApplicationEventHandler* getApplicationHandler() const { return m_applicationHandler_.get(); }

  void setApplicationHandler(std::unique_ptr<ApplicationEventHandler> handler) {
    m_applicationHandler_ = std::move(handler);
  }

  private:
  std::unique_ptr<ApplicationEventHandler> m_applicationHandler_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_EVENT_APPLICATION_EVENT_MANAGER_H