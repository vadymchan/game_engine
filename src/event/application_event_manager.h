#ifndef ARISE_EVENT_APPLICATION_EVENT_MANAGER_H
#define ARISE_EVENT_APPLICATION_EVENT_MANAGER_H

#include "application_event_handler.h"
#include "event.h"

#include <memory>

namespace arise {

class ApplicationEventManager {
  public:
  ApplicationEventManager() = default;

  ApplicationEventManager(std::unique_ptr<ApplicationEventHandler> applicationHandler)
      : m_applicationHandler_(std::move(applicationHandler)) {}

  void routeEvent(const Event& event) {
    if (event.type == SDL_QUIT && m_applicationHandler_) {
      m_applicationHandler_->dispatch(event.quit);
    }
  }

  ApplicationEventHandler* getApplicationHandler() const { return m_applicationHandler_.get(); }

  void setApplicationHandler(std::unique_ptr<ApplicationEventHandler> handler) {
    m_applicationHandler_ = std::move(handler);
  }

  private:
  std::unique_ptr<ApplicationEventHandler> m_applicationHandler_;
};

}  // namespace arise

#endif  // ARISE_EVENT_APPLICATION_EVENT_MANAGER_H