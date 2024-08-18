

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
    m_buffer_.m_hasBufferOwnership_ = false;  // Prevent destroy the ring buffer
    m_buffer_.m_allocatedSize_      = size;
  } else {
    m_buffer_.m_hasBufferOwnership_ = false;  // Prevent destroy the ring buffer
    m_buffer_.m_allocatedSize_      = size;
  }
}

void UniformBufferBlockVk::UpdateBufferData(const void* data, size_t size) {
  assert(m_buffer_.m_allocatedSize_ >= size);

  if (IsUseRingBuffer()) {
    RingBufferVk* ringBuffer = g_rhi_vk->GetOneFrameUniformRingBuffer();
    m_buffer_.m_offset_            = ringBuffer->Alloc(size);
    m_buffer_.m_allocatedSize_     = size;
    m_buffer_.m_buffer_          = ringBuffer->m_buffer_;
    m_buffer_.m_deviceMemory_          = ringBuffer->m_bufferMemory_;

    if (ringBuffer->m_mappedPointer_) {
      uint8_t* startAddr
          = ((uint8_t*)ringBuffer->m_mappedPointer_) + m_buffer_.m_offset_;
      if (data) {
        memcpy(startAddr, data, size);
      } else {
        memset(startAddr, 0, size);
      }
    }
  } else {
#if USE_VK_MEMORY_POOL
    assert(m_buffer_.m_allocatedSize_);
    AllocBufferFromGlobalMemory(m_buffer_.m_allocatedSize_);

    if (m_buffer_.m_mappedPointer_) {
      uint8_t* startAddr = ((uint8_t*)m_buffer_.m_mappedPointer_) + m_buffer_.m_offset_;
      if (data) {
        memcpy(startAddr, data, size);
      } else {
        memset(startAddr, 0, size);
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
      if (data) {
        memcpy(data, data, size);
      } else {
        memset(data, 0, size);
      }
      vkUnmapMemory(g_rhi_vk->m_device_, m_buffer.m_deviceMemory);
    }
#endif
  }
}

void UniformBufferBlockVk::ClearBuffer(int32_t clearValue) {
  if (IsUseRingBuffer()) {
    RingBufferVk* ringBuffer = g_rhi_vk->GetOneFrameUniformRingBuffer();
    m_buffer_.m_offset_            = ringBuffer->Alloc(m_buffer_.m_allocatedSize_);
    m_buffer_.m_allocatedSize_     = m_buffer_.m_allocatedSize_;
    m_buffer_.m_buffer_          = ringBuffer->m_buffer_;
    m_buffer_.m_deviceMemory_          = ringBuffer->m_bufferMemory_;

    if (ringBuffer->m_mappedPointer_) {
      memset(((uint8_t*)ringBuffer->m_mappedPointer_) + m_buffer_.m_offset_,
             0,
             m_buffer_.m_allocatedSize_);
    }
  } else {
#if USE_VK_MEMORY_POOL
    assert(m_buffer_.m_allocatedSize_);
    AllocBufferFromGlobalMemory(m_buffer_.m_allocatedSize_);

    if (m_buffer_.m_mappedPointer_) {
      memset(((uint8_t*)m_buffer_.m_mappedPointer_) + m_buffer_.m_offset_,
             0,
             m_buffer_.m_allocatedSize_);
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
  m_buffer_.Release();
  m_buffer_.InitializeWithMemory(g_rhi_vk->GetMemoryPool()->Alloc(
      EVulkanBufferBits::UNIFORM_BUFFER,
      EVulkanMemoryBits::HOST_VISIBLE | EVulkanMemoryBits::HOST_COHERENT,
      VkDeviceSize(size)));


  assert(m_buffer_.m_memory_.IsValid());
}

}  // namespace game_engine  
