#ifndef ARISE_SERVICE_LOCATOR_H
#define ARISE_SERVICE_LOCATOR_H

#include <memory>

namespace arise {

class ServiceLocator {
  public:
  template <typename T>
  static void s_provide(std::unique_ptr<T> service) {
    s_getServiceContainer<T>() = std::move(service);
  }

  // constructing it in-place
  template <typename T, typename... Args>
  static void s_provide(Args&&... args) {
    s_getServiceContainer<T>() = std::make_unique<T>(std::forward<Args>(args)...);
  }

  // Get service as raw pointer (service locator keeps ownership)
  template <typename T>
  static T* s_get() {
    return s_getServiceContainer<T>().get();
  }

  template <typename T>
  static void s_remove() {
    s_getServiceContainer<T>().reset();
  }

  private:
  template <typename T>
  static std::unique_ptr<T>& s_getServiceContainer() {
    static std::unique_ptr<T> service = nullptr;
    return service;
  }
};

}  // namespace arise

#endif  // ARISE_SERVICE_LOCATOR_H