

#include "gfx/rhi/vulkan/uniform_buffer_object_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

void UniformBufferBlockVk::Init(size_t size) {
  assert(size);

  size = Align<uint64_t>(
      size,
      g_rhi_vk->m_deviceProperties_.limits.minUniformBufferOffsetAlignment);

  if (IsUseRingBuffer()) {
    Buffer.HasBufferOwnership = false;  // Prevent destroy the ring buffer
    Buffer.AllocatedSize      = size;
  } else {
    Buffer.HasBufferOwnership = false;  // Prevent destroy the ring buffer
    Buffer.AllocatedSize      = size;
  }
}

void UniformBufferBlockVk::UpdateBufferData(const void* InData, size_t InSize) {
  assert(Buffer.AllocatedSize >= InSize);

  if (IsUseRingBuffer()) {
    RingBufferVk* ringBuffer = g_rhi_vk->GetOneFrameUniformRingBuffer();
    Buffer.Offset            = ringBuffer->Alloc(InSize);
    Buffer.AllocatedSize     = InSize;
    Buffer.m_buffer          = ringBuffer->Buffer;
    Buffer.m_memory          = ringBuffer->BufferMemory;

    if (ringBuffer->MappedPointer) {
      uint8_t* startAddr
          = ((uint8_t*)ringBuffer->MappedPointer) + Buffer.Offset;
      if (InData) {
        memcpy(startAddr, InData, InSize);
      } else {
        memset(startAddr, 0, InSize);
      }
    }
  } else {
#if USE_VK_MEMORY_POOL
    assert(Buffer.AllocatedSize);
    AllocBufferFromGlobalMemory(Buffer.AllocatedSize);

    if (Buffer.MappedPointer) {
      uint8_t* startAddr = ((uint8_t*)Buffer.MappedPointer) + Buffer.Offset;
      if (InData) {
        memcpy(startAddr, InData, InSize);
      } else {
        memset(startAddr, 0, InSize);
      }
    }
#else
    if (Buffer.m_buffer && Buffer.m_memory) {
      void* data = nullptr;
      vkMapMemory(g_rhi_vk->m_device_,
                  Buffer.m_memory,
                  Buffer.Offset,
                  Buffer.AllocatedSize,
                  0,
                  &data);
      if (InData) {
        memcpy(data, InData, InSize);
      } else {
        memset(data, 0, InSize);
      }
      vkUnmapMemory(g_rhi_vk->m_device_, Buffer.m_memory);
    }
#endif
  }
}

void UniformBufferBlockVk::ClearBuffer(int32_t clearValue) {
  if (IsUseRingBuffer()) {
    RingBufferVk* ringBuffer = g_rhi_vk->GetOneFrameUniformRingBuffer();
    Buffer.Offset            = ringBuffer->Alloc(Buffer.AllocatedSize);
    Buffer.AllocatedSize     = Buffer.AllocatedSize;
    Buffer.m_buffer          = ringBuffer->Buffer;
    Buffer.m_memory          = ringBuffer->BufferMemory;

    if (ringBuffer->MappedPointer) {
      memset(((uint8_t*)ringBuffer->MappedPointer) + Buffer.Offset,
             0,
             Buffer.AllocatedSize);
    }
  } else {
#if USE_VK_MEMORY_POOL
    assert(Buffer.AllocatedSize);
    AllocBufferFromGlobalMemory(Buffer.AllocatedSize);

    if (Buffer.MappedPointer) {
      memset(((uint8_t*)Buffer.MappedPointer) + Buffer.Offset,
             0,
             Buffer.AllocatedSize);
    }
#else
    if (Buffer.m_buffer && Buffer.m_memory) {
      void* data = nullptr;
      vkMapMemory(g_rhi_vk->m_device_,
                  Buffer.m_memory,
                  Buffer.Offset,
                  Buffer.AllocatedSize,
                  0,
                  &data);
      memset(data, clearValue, Buffer.AllocatedSize);
      vkUnmapMemory(g_rhi_vk->m_device_, Buffer.m_memory);
    }
#endif
  }
}

void UniformBufferBlockVk::AllocBufferFromGlobalMemory(size_t size) {
  // If we have allocated memory frame global memory, reallocate it again.
  Buffer.Release();
  Buffer.InitializeWithMemory(g_rhi_vk->GetMemoryPool()->Alloc(
      EVulkanBufferBits::UNIFORM_BUFFER,
      EVulkanMemoryBits::HOST_VISIBLE | EVulkanMemoryBits::HOST_COHERENT,
      VkDeviceSize(size)));


  assert(Buffer.Memory.IsValid());
}

}  // namespace game_engine  
