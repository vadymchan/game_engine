#ifndef GAME_ENGINE_SEMAPHORE_MANAGER_H
#define GAME_ENGINE_SEMAPHORE_MANAGER_H

namespace game_engine {

class Semaphore {
  public:
  virtual ~Semaphore() {}

  virtual void* getHandle() const = 0;
};

class SemaphoreManager {
  public:
  virtual ~SemaphoreManager() {}

  virtual Semaphore* getOrCreateSemaphore()            = 0;
  virtual void       returnSemaphore(Semaphore* fence) = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SEMAPHORE_MANAGER_H