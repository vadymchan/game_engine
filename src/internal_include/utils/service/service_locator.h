#ifndef GAME_ENGINE_SERVICE_LOCATOR_H
#define GAME_ENGINE_SERVICE_LOCATOR_H

#include <memory>

namespace game_engine {

// For managing managers (note: for only static managers it makes no sence to
// add it here)
class ServiceLocator {
  public:
  template <typename T>
  static void s_provide(std::shared_ptr<T> service) {
    s_getServiceContainer<T>() = std::move(service);
  }

  template <typename T, typename... Args>
  static void s_provide(Args&&... args) {
    s_getServiceContainer<T>()
        = std::make_shared<T>(std::forward<Args>(args)...);
  }

  // TODO: use std::weak_ptr (we will manage destructors here)
  template <typename T>
  static std::shared_ptr<T> s_get() {
    return s_getServiceContainer<T>();
  }

  // TODO: test
  template <typename T>
  static void s_remove() {
    s_getServiceContainer<T>().reset();
  }

  private:
  template <typename T>
  static std::shared_ptr<T>& s_getServiceContainer() {
    static std::shared_ptr<T> service = nullptr;
    return service;
  }
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SERVICE_LOCATOR_H
