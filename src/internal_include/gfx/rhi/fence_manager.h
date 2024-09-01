#ifndef GAME_ENGINE_FENCE_MANAGER_H
#define GAME_ENGINE_FENCE_MANAGER_H

#include <cstdint>

namespace game_engine {

class IFence {
  public:
  // ======= BEGIN: public destructor =========================================

  virtual ~IFence() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  // virtual void  s_release()                                     = 0;
  virtual void waitForFence(uint64_t timeoutNanoSec = UINT64_MAX) = 0;

  // TODO: consider making them pure virtual
  virtual bool isValid() const { return false; }

  virtual bool isComplete(uint64_t fenceValue) const { return false; }

  virtual bool isComplete() const { return false; }

  virtual void* getHandle() const = 0;

  // ======= END: public overridden methods   =================================
};

class IFenceManager {
  public:
  // ======= BEGIN: public destructor =========================================

  virtual ~IFenceManager() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void    returnFence(IFence* fence) = 0;
  virtual void    release()                  = 0;
  virtual IFence* getOrCreateFence()         = 0;

  // ======= END: public overridden methods   =================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_FENCE_MANAGER_H