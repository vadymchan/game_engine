#ifndef GAME_ENGINE_FENCE_VK_H
#define GAME_ENGINE_FENCE_VK_H

#include "gfx/rhi/fence_manager.h"

#include <vulkan/vulkan.h>

#include <cassert>
#include <unordered_set>

namespace game_engine {

// TODO: override IsComplete methods
class FenceVk : public Fence {
  public:
  FenceVk()
      : m_fence(VK_NULL_HANDLE) {}

  virtual ~FenceVk() override;

  virtual void* GetHandle() const override {
    return reinterpret_cast<void*>(m_fence);
  }

  void WaitForFence(uint64_t timeout = UINT64_MAX) override;

  // TODO: not used
  void ResetFence() const;

  // TODO
  // private:

  VkFence m_fence;
};

class FenceManagerVk : public FenceManager {
  public:
  ~FenceManagerVk() { Release(); }

  Fence* GetOrCreateFence() override;

  void ReturnFence(Fence* fence) override;

  void Release() override;

  private:
  std::unordered_set<Fence*> UsingFences;
  std::unordered_set<Fence*> PendingFences;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_FENCE_VK_H