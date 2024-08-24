

#include "gfx/rhi/vulkan/memory_pool_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

// TODO: some mess with the order of parameters
void SubMemoryAllocatorVk::initialize(EVulkanBufferBits usage,
                                    EVulkanMemoryBits properties,
                                    uint64_t          size) {
  assert(m_subMemoryRange_.m_offset_ == 0 && m_subMemoryRange_.m_dataSize_ == 0);
  assert(0 == m_freeLists_.size());

  m_subMemoryRange_.m_offset_ = 0;
  g_createBufferLowLevel(usage,
                        properties,
                        size,
                        m_buffer_,
                        m_deviceMemory_,
                        m_subMemoryRange_.m_dataSize_);
  m_usages_     = usage;
  m_properties_ = properties;

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(g_rhiVk->m_device_, m_buffer_, &memRequirements);
  m_alignment_ = memRequirements.alignment;

  if (!!(properties & EVulkanMemoryBits::HOST_VISIBLE)) {
    assert(vkMapMemory(g_rhiVk->m_device_,
                       m_deviceMemory_,
                       0,
                       m_subMemoryRange_.m_dataSize_,
                       0,
                       &m_mappedPointer_)
           == VK_SUCCESS);
  }
}



SubMemoryAllocator* MemoryPoolVk::createSubMemoryAllocator() const {
  return new SubMemoryAllocatorVk();
}


}  // namespace game_engine
