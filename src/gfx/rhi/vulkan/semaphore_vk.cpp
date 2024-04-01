#include "gfx/rhi/vulkan/semaphore_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

// SemaphoreVk
// ====================================================

SemaphoreVk ::~SemaphoreVk() {
  if (Semaphore != VK_NULL_HANDLE) {
    vkDestroySemaphore(g_rhi_vk->m_device_, Semaphore, nullptr);
  }
}

// SemaphoreManagerVk
// ====================================================

SemaphoreVk* SemaphoreManagerVk::GetOrCreateSemaphore() {
  if (!PendingSemaphores.empty()) {
    SemaphoreVk* semaphore = *PendingSemaphores.begin();
    PendingSemaphores.erase(PendingSemaphores.begin());
    UsingSemaphores.insert(semaphore);
    return semaphore;
  }

  auto*                 newSemaphore  = new SemaphoreVk();
  VkSemaphoreCreateInfo semaphoreInfo = {};
  semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  if (vkCreateSemaphore(g_rhi_vk->m_device_,
                        &semaphoreInfo,
                        nullptr,
                        &newSemaphore->Semaphore)
      == VK_SUCCESS) {
    UsingSemaphores.insert(newSemaphore);
  } else {
    assert(false && "Failed to create semaphore");
    delete newSemaphore;
    return nullptr;
  }

  return newSemaphore;
}

void SemaphoreManagerVk::ReturnSemaphore(SemaphoreVk* semaphore) {
  UsingSemaphores.erase(semaphore);
  PendingSemaphores.insert(semaphore);
}

void SemaphoreManagerVk::Release() {
  for (auto& semaphore : UsingSemaphores) {
    delete semaphore;
  }
  UsingSemaphores.clear();

  for (auto& semaphore : PendingSemaphores) {
    delete semaphore;
  }
  PendingSemaphores.clear();
}

}  // namespace game_engine
