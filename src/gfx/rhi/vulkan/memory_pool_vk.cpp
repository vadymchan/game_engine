

#include "gfx/rhi/vulkan/memory_pool_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

// TODO: some mess with the order of parameters
void SubMemoryAllocatorVk::Initialize(EVulkanBufferBits InUsage,
                                    EVulkanMemoryBits InProperties,
                                    uint64_t          InSize) {
  assert(SubMemoryRange.Offset == 0 && SubMemoryRange.DataSize == 0);
  assert(0 == FreeLists.size());

  SubMemoryRange.Offset = 0;
  CreateBuffer_LowLevel(InUsage,
                        InProperties,
                        InSize,
                        m_buffer,
                        DeviceMemory,
                        SubMemoryRange.DataSize);
  Usages     = InUsage;
  Properties = InProperties;

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(g_rhi_vk->m_device_, m_buffer, &memRequirements);
  Alignment = memRequirements.alignment;

  if (!!(InProperties & EVulkanMemoryBits::HOST_VISIBLE)) {
    assert(vkMapMemory(g_rhi_vk->m_device_,
                       DeviceMemory,
                       0,
                       SubMemoryRange.DataSize,
                       0,
                       &MappedPointer)
           == VK_SUCCESS);
  }
}



SubMemoryAllocator* MemoryPoolVk::CreateSubMemoryAllocator() const {
  return new SubMemoryAllocatorVk();
}


}  // namespace game_engine
