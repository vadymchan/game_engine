#ifndef GAME_ENGINE_FENCE_MANAGER_H
#define GAME_ENGINE_FENCE_MANAGER_H

#include <cstdint>

namespace game_engine {

class jFence {
  public:
  virtual ~jFence() {}

  virtual void* GetHandle() const = 0;
  // virtual void  Release()                                          = 0;
  virtual void  WaitForFence(uint64_t InTimeoutNanoSec = UINT64_MAX) = 0;

  //virtual bool IsValid() const { return false; }

  //virtual bool IsComplete(uint64_t InFenceValue) const { return false; }

  //virtual bool IsComplete() const { return false; }
};

class jFenceManager {
  public:
  virtual ~jFenceManager() {}

  virtual jFence* GetOrCreateFence()         = 0;
  virtual void    ReturnFence(jFence* fence) = 0;
  virtual void    Release()                  = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_FENCE_MANAGER_H