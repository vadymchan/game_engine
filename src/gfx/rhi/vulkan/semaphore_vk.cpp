#include "gfx/rhi/vulkan/semaphore_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

// SemaphoreVk
// ====================================================

SemaphoreVk ::~SemaphoreVk() {
  if (m_semaphore_ != VK_NULL_HANDLE) {
    vkDestroySemaphore(g_rhi_vk->m_device_, m_semaphore_, nullptr);
  }
}

// SemaphoreManagerVk
// ====================================================

ISemaphore* SemaphoreManagerVk::getOrCreateSemaphore() {
  if (!m_pendingSemaphores_.empty()) {
    ISemaphore* semaphore = *m_pendingSemaphores_.begin();
    m_pendingSemaphores_.erase(m_pendingSemaphores_.begin());
    m_usingSemaphores_.insert(semaphore);
    return semaphore;
  }

  auto*                 newSemaphore  = new SemaphoreVk();
  VkSemaphoreCreateInfo semaphoreInfo = {};
  semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  if (vkCreateSemaphore(g_rhi_vk->m_device_,
                        &semaphoreInfo,
                        nullptr,
                        &newSemaphore->m_semaphore_)
      == VK_SUCCESS) {
    m_usingSemaphores_.insert(newSemaphore);
  } else {
    assert(false && "Failed to create semaphore");
    delete newSemaphore;
    return nullptr;
  }

  return newSemaphore;
}

void SemaphoreManagerVk::returnSemaphore(ISemaphore* semaphore) {
  m_usingSemaphores_.erase(semaphore);
  m_pendingSemaphores_.insert(semaphore);
}

void SemaphoreManagerVk::release() {
  for (auto& semaphore : m_usingSemaphores_) {
    delete semaphore;
  }
  m_usingSemaphores_.clear();

  for (auto& semaphore : m_pendingSemaphores_) {
    delete semaphore;
  }
  m_pendingSemaphores_.clear();
}

}  // namespace game_engine
