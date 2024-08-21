#ifndef GAME_ENGINE_FENCE_MANAGER_H
#define GAME_ENGINE_FENCE_MANAGER_H

#include <cstdint>

namespace game_engine {

class Fence {
  public:
  virtual ~Fence() {}

  virtual void* getHandle() const = 0;
  // virtual void  release()                                          = 0;
  virtual void  waitForFence(uint64_t timeoutNanoSec = UINT64_MAX) = 0;

  // TODO: consider making them pure virtual
  virtual bool isValid() const { return false; }

  virtual bool isComplete(uint64_t fenceValue) const { return false; }

  virtual bool isComplete() const { return false; }
};

class FenceManager {
  public:
  virtual ~FenceManager() {}

  virtual Fence* getOrCreateFence()        = 0;
  virtual void   returnFence(Fence* fence) = 0;
  virtual void   release()                 = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_FENCE_MANAGER_H