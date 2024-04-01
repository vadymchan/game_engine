#ifndef GAME_ENGINE_FENCE_VK_H
#define GAME_ENGINE_FENCE_VK_H

#include <vulkan/vulkan.h>

#include <cassert>
#include <unordered_set>

namespace game_engine {

class FenceVk {
  public:
  FenceVk()
      : Fence(VK_NULL_HANDLE) {}

  virtual ~FenceVk();

  virtual void* GetHandle() const { return reinterpret_cast<void*>(Fence); }

  void WaitForFence(uint64_t timeout = UINT64_MAX) const;

  void ResetFence() const;

  // TODO
  // private:

  VkFence Fence;
};

class FenceManagerVk {
  public:
  ~FenceManagerVk() { Release(); }

  FenceVk* GetOrCreateFence();

  void ReturnFence(FenceVk* fence);

  void Release();

  private:
  std::unordered_set<FenceVk*> UsingFences;
  std::unordered_set<FenceVk*> PendingFences;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_FENCE_VK_H