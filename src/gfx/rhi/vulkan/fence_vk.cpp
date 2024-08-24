
#include "gfx/rhi/vulkan/fence_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

// FenceVk
// ====================================================

FenceVk::~FenceVk() {
  if (m_fence_ != VK_NULL_HANDLE) {
    vkDestroyFence(g_rhiVk->m_device_, m_fence_, nullptr);
  }
}

void FenceVk::waitForFence(uint64_t timeout) {
  assert(m_fence_ != VK_NULL_HANDLE);
  vkWaitForFences(g_rhiVk->m_device_, 1, &m_fence_, VK_TRUE, timeout);
}

void FenceVk::resetFence() const {
  assert(m_fence_ != VK_NULL_HANDLE);
  vkResetFences(g_rhiVk->m_device_, 1, &m_fence_);
}

// FenceManagerVk
// ====================================================

IFence* FenceManagerVk::getOrCreateFence() {
  if (!m_pendingFences_.empty()) {
    IFence* fence = *m_pendingFences_.begin();
    m_pendingFences_.erase(m_pendingFences_.begin());
    m_usingFences_.insert(fence);
    return fence;
  }

  auto*             newFence  = new FenceVk();
  VkFenceCreateInfo fenceInfo = {};
  fenceInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags             = VK_FENCE_CREATE_SIGNALED_BIT;
  if (vkCreateFence(g_rhiVk->m_device_, &fenceInfo, nullptr, &newFence->m_fence_)
      == VK_SUCCESS) {
    m_usingFences_.insert(newFence);
  } else {
    assert(false && "Failed to create fence");
    delete newFence;
    return nullptr;
  }

  return newFence;
}

void FenceManagerVk::returnFence(IFence* fence) {
  m_usingFences_.erase(fence);
  m_pendingFences_.insert(fence);
}

void FenceManagerVk::release() {
  for (auto& fence : m_usingFences_) {
    delete fence;
  }
  m_usingFences_.clear();

  for (auto& fence : m_pendingFences_) {
    delete fence;
  }
  m_pendingFences_.clear();
}

}  // namespace game_engine
