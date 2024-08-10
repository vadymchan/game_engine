#ifndef GAME_ENGINE_FENCE_VK_H
#define GAME_ENGINE_FENCE_VK_H

#include "gfx/rhi/fence_manager.h"

#include <vulkan/vulkan.h>

#include <cassert>
#include <unordered_set>

namespace game_engine {

// TODO: override IsComplete methods
class FenceVk : public jFence {
  public:
  FenceVk()
      : Fence(VK_NULL_HANDLE) {}

  virtual ~FenceVk() override;

  virtual void* GetHandle() const override {
    return reinterpret_cast<void*>(Fence);
  }

  void WaitForFence(uint64_t timeout = UINT64_MAX) override;

  // TODO: not used
  void ResetFence() const;

  // TODO
  // private:

  VkFence Fence;
};

class FenceManagerVk : public jFenceManager {
  public:
  ~FenceManagerVk() { Release(); }

  jFence* GetOrCreateFence() override;

  void ReturnFence(jFence* fence) override;

  void Release() override;

  private:
  std::unordered_set<jFence*> UsingFences;
  std::unordered_set<jFence*> PendingFences;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_FENCE_VK_H