

#include "gfx/rhi/vulkan/memory_pool_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

// TODO: some mess with the order of parameters
void SubMemoryAllocatorVk::Initialize(EVulkanBufferBits InUsage,
                                    EVulkanMemoryBits InProperties,
                                    uint64_t          size) {
  assert(m_subMemoryRange_.m_offset_ == 0 && m_subMemoryRange_.m_dataSize_ == 0);
  assert(0 == m_freeLists_.size());

  m_subMemoryRange_.m_offset_ = 0;
  CreateBuffer_LowLevel(InUsage,
                        InProperties,
                        size,
                        m_buffer_,
                        m_deviceMemory_,
                        m_subMemoryRange_.m_dataSize_);
  m_usages_     = InUsage;
  m_properties_ = InProperties;

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(g_rhi_vk->m_device_, m_buffer_, &memRequirements);
  m_alignment_ = memRequirements.alignment;

  if (!!(InProperties & EVulkanMemoryBits::HOST_VISIBLE)) {
    assert(vkMapMemory(g_rhi_vk->m_device_,
                       m_deviceMemory_,
                       0,
                       m_subMemoryRange_.m_dataSize_,
                       0,
                       &m_mappedPointer_)
           == VK_SUCCESS);
  }
}



SubMemoryAllocator* MemoryPoolVk::CreateSubMemoryAllocator() const {
  return new SubMemoryAllocatorVk();
}


}  // namespace game_engine
