#ifndef GAME_ENGINE_EVENT_APPLICATION_EVENT_MANAGER_H
#define GAME_ENGINE_EVENT_APPLICATION_EVENT_MANAGER_H

#include "application_event_handler.h"
#include "event.h"

#include <memory>
#include <utility>

namespace game_engine {

class ApplicationEventManager {
  public:
  ApplicationEventManager(std::shared_ptr<ApplicationEventHandler> applicationHandler)
      : m_applicationHandler_(std::move(applicationHandler)) {}

  void routeEvent(const Event& event) {
    if (event.type == SDL_QUIT) {
      m_applicationHandler_->dispatch(event.quit);
    }
    // TODO: other application event
  }

  private:
  std::shared_ptr<ApplicationEventHandler> m_applicationHandler_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_EVENT_APPLICATION_EVENT_MANAGER_H
