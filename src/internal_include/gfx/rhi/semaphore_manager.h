#ifndef GAME_ENGINE_SEMAPHORE_MANAGER_H
#define GAME_ENGINE_SEMAPHORE_MANAGER_H

namespace game_engine {

class jSemaphore {
  public:
  virtual ~jSemaphore() {}

  virtual void* GetHandle() const = 0;
};

class jSemaphoreManager {
  public:
  virtual ~jSemaphoreManager() {}

  virtual jSemaphore* GetOrCreateSemaphore()             = 0;
  virtual void        ReturnSemaphore(jSemaphore* fence) = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SEMAPHORE_MANAGER_H