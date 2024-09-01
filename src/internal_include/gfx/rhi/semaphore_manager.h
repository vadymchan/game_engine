#ifndef GAME_ENGINE_SEMAPHORE_MANAGER_H
#define GAME_ENGINE_SEMAPHORE_MANAGER_H

namespace game_engine {

class ISemaphore {
  public:
  // ======= BEGIN: public destructor =========================================

  virtual ~ISemaphore() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void* getHandle() const = 0;

  // ======= END: public overridden methods   =================================
};

class ISemaphoreManager {
  public:
  // ======= BEGIN: public destructor =========================================

  virtual ~ISemaphoreManager() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void        returnSemaphore(ISemaphore* fence) = 0;
  virtual ISemaphore* getOrCreateSemaphore()             = 0;

  // ======= END: public overridden methods   =================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_SEMAPHORE_MANAGER_H