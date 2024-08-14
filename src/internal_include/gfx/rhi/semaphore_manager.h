#ifndef GAME_ENGINE_SEMAPHORE_MANAGER_H
#define GAME_ENGINE_SEMAPHORE_MANAGER_H

namespace game_engine {

class Semaphore {
  public:
  virtual ~Semaphore() {}

  virtual void* GetHandle() const = 0;
};

class SemaphoreManager {
  public:
  virtual ~SemaphoreManager() {}

  virtual Semaphore* GetOrCreateSemaphore()             = 0;
  virtual void        ReturnSemaphore(Semaphore* fence) = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SEMAPHORE_MANAGER_H