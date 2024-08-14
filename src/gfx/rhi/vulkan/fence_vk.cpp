
#include "gfx/rhi/vulkan/fence_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

// FenceVk
// ====================================================

FenceVk::~FenceVk() {
  if (m_fence != VK_NULL_HANDLE) {
    vkDestroyFence(g_rhi_vk->m_device_, m_fence, nullptr);
  }
}

void FenceVk::WaitForFence(uint64_t timeout) {
  assert(m_fence != VK_NULL_HANDLE);
  vkWaitForFences(g_rhi_vk->m_device_, 1, &m_fence, VK_TRUE, timeout);
}

void FenceVk::ResetFence() const {
  assert(m_fence != VK_NULL_HANDLE);
  vkResetFences(g_rhi_vk->m_device_, 1, &m_fence);
}

// FenceManagerVk
// ====================================================

Fence* FenceManagerVk::GetOrCreateFence() {
  if (!PendingFences.empty()) {
    Fence* fence = *PendingFences.begin();
    PendingFences.erase(PendingFences.begin());
    UsingFences.insert(fence);
    return fence;
  }

  auto*             newFence  = new FenceVk();
  VkFenceCreateInfo fenceInfo = {};
  fenceInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags             = VK_FENCE_CREATE_SIGNALED_BIT;
  if (vkCreateFence(g_rhi_vk->m_device_, &fenceInfo, nullptr, &newFence->m_fence)
      == VK_SUCCESS) {
    UsingFences.insert(newFence);
  } else {
    assert(false && "Failed to create fence");
    delete newFence;
    return nullptr;
  }

  return newFence;
}

void FenceManagerVk::ReturnFence(Fence* fence) {
  UsingFences.erase(fence);
  PendingFences.insert(fence);
}

void FenceManagerVk::Release() {
  for (auto& fence : UsingFences) {
    delete fence;
  }
  UsingFences.clear();

  for (auto& fence : PendingFences) {
    delete fence;
  }
  PendingFences.clear();
}

}  // namespace game_engine
