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
      : m_fence_(VK_NULL_HANDLE) {}

  virtual ~FenceVk() override;

  virtual void* GetHandle() const override {
    return reinterpret_cast<void*>(m_fence_);
  }

  void WaitForFence(uint64_t timeout = UINT64_MAX) override;

  // TODO: not used
  void ResetFence() const;

  // TODO
  // private:

  VkFence m_fence_;
};

class FenceManagerVk : public FenceManager {
  public:
  ~FenceManagerVk() { Release(); }

  Fence* GetOrCreateFence() override;

  void ReturnFence(Fence* fence) override;

  void Release() override;

  private:
  std::unordered_set<Fence*> m_usingFences_;
  std::unordered_set<Fence*> m_pendingFences_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_FENCE_VK_H