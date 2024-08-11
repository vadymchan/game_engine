#ifndef GAME_ENGINE_FENCE_MANAGER_H
#define GAME_ENGINE_FENCE_MANAGER_H

#include <cstdint>

namespace game_engine {

class Fence {
  public:
  virtual ~Fence() {}

  virtual void* GetHandle() const = 0;
  // virtual void  Release()                                          = 0;
  virtual void  WaitForFence(uint64_t InTimeoutNanoSec = UINT64_MAX) = 0;

  // TODO: consider making them pure virtual
  virtual bool IsValid() const { return false; }

  virtual bool IsComplete(uint64_t InFenceValue) const { return false; }

  virtual bool IsComplete() const { return false; }
};

class FenceManager {
  public:
  virtual ~FenceManager() {}

  virtual Fence* GetOrCreateFence()        = 0;
  virtual void   ReturnFence(Fence* fence) = 0;
  virtual void   Release()                 = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_FENCE_MANAGER_H