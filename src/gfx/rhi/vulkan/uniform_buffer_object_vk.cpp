

#include "gfx/rhi/vulkan/uniform_buffer_object_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"
#include "utils/memory/align.h"


namespace game_engine {

void UniformBufferBlockVk::Init(size_t size) {
  assert(size);

  size = Align<uint64_t>(
      size,
      g_rhi_vk->m_deviceProperties_.limits.minUniformBufferOffsetAlignment);

  if (IsUseRingBuffer()) {
    m_buffer.HasBufferOwnership = false;  // Prevent destroy the ring buffer
    m_buffer.AllocatedSize      = size;
  } else {
    m_buffer.HasBufferOwnership = false;  // Prevent destroy the ring buffer
    m_buffer.AllocatedSize      = size;
  }
}

void UniformBufferBlockVk::UpdateBufferData(const void* InData, size_t InSize) {
  assert(m_buffer.AllocatedSize >= InSize);

  if (IsUseRingBuffer()) {
    RingBufferVk* ringBuffer = g_rhi_vk->GetOneFrameUniformRingBuffer();
    m_buffer.Offset            = ringBuffer->Alloc(InSize);
    m_buffer.AllocatedSize     = InSize;
    m_buffer.m_buffer          = ringBuffer->m_buffer;
    m_buffer.m_deviceMemory          = ringBuffer->BufferMemory;

    if (ringBuffer->MappedPointer) {
      uint8_t* startAddr
          = ((uint8_t*)ringBuffer->MappedPointer) + m_buffer.Offset;
      if (InData) {
        memcpy(startAddr, InData, InSize);
      } else {
        memset(startAddr, 0, InSize);
      }
    }
  } else {
#if USE_VK_MEMORY_POOL
    assert(m_buffer.AllocatedSize);
    AllocBufferFromGlobalMemory(m_buffer.AllocatedSize);

    if (m_buffer.MappedPointer) {
      uint8_t* startAddr = ((uint8_t*)m_buffer.MappedPointer) + m_buffer.Offset;
      if (InData) {
        memcpy(startAddr, InData, InSize);
      } else {
        memset(startAddr, 0, InSize);
      }
    }
#else
    if (m_buffer.m_buffer && m_buffer.m_deviceMemory) {
      void* data = nullptr;
      vkMapMemory(g_rhi_vk->m_device_,
                  m_buffer.m_deviceMemory,
                  m_buffer.Offset,
                  m_buffer.AllocatedSize,
                  0,
                  &data);
      if (InData) {
        memcpy(data, InData, InSize);
      } else {
        memset(data, 0, InSize);
      }
      vkUnmapMemory(g_rhi_vk->m_device_, m_buffer.m_deviceMemory);
    }
#endif
  }
}

void UniformBufferBlockVk::ClearBuffer(int32_t clearValue) {
  if (IsUseRingBuffer()) {
    RingBufferVk* ringBuffer = g_rhi_vk->GetOneFrameUniformRingBuffer();
    m_buffer.Offset            = ringBuffer->Alloc(m_buffer.AllocatedSize);
    m_buffer.AllocatedSize     = m_buffer.AllocatedSize;
    m_buffer.m_buffer          = ringBuffer->m_buffer;
    m_buffer.m_deviceMemory          = ringBuffer->BufferMemory;

    if (ringBuffer->MappedPointer) {
      memset(((uint8_t*)ringBuffer->MappedPointer) + m_buffer.Offset,
             0,
             m_buffer.AllocatedSize);
    }
  } else {
#if USE_VK_MEMORY_POOL
    assert(m_buffer.AllocatedSize);
    AllocBufferFromGlobalMemory(m_buffer.AllocatedSize);

    if (m_buffer.MappedPointer) {
      memset(((uint8_t*)m_buffer.MappedPointer) + m_buffer.Offset,
             0,
             m_buffer.AllocatedSize);
    }
#else
    if (m_buffer.m_buffer && m_buffer.m_deviceMemory) {
      void* data = nullptr;
      vkMapMemory(g_rhi_vk->m_device_,
                  m_buffer.m_deviceMemory,
                  m_buffer.Offset,
                  m_buffer.AllocatedSize,
                  0,
                  &data);
      memset(data, clearValue, m_buffer.AllocatedSize);
      vkUnmapMemory(g_rhi_vk->m_device_, m_buffer.m_deviceMemory);
    }
#endif
  }
}

void UniformBufferBlockVk::AllocBufferFromGlobalMemory(size_t size) {
  // If we have allocated memory frame global memory, reallocate it again.
  m_buffer.Release();
  m_buffer.InitializeWithMemory(g_rhi_vk->GetMemoryPool()->Alloc(
      EVulkanBufferBits::UNIFORM_BUFFER,
      EVulkanMemoryBits::HOST_VISIBLE | EVulkanMemoryBits::HOST_COHERENT,
      VkDeviceSize(size)));


  assert(m_buffer.m_memory.IsValid());
}

}  // namespace game_engine  
