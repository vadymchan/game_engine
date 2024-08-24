#ifndef GAME_ENGINE_SEMAPHORE_MANAGER_H
#define GAME_ENGINE_SEMAPHORE_MANAGER_H

namespace game_engine {

class ISemaphore {
  public:
  virtual ~ISemaphore() {}

  virtual void* getHandle() const = 0;
};

class ISemaphoreManager {
  public:
  virtual ~ISemaphoreManager() {}

  virtual ISemaphore* getOrCreateSemaphore()            = 0;
  virtual void       returnSemaphore(ISemaphore* fence) = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SEMAPHORE_MANAGER_H