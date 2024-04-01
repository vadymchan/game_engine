
#include "gfx/rhi/vulkan/ring_buffer_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

#include <cstdint>

namespace game_engine {

void RingBufferVk::Create(EVulkanBufferBits bufferBits,
                          uint64_t          totalSize,
                          uint32_t          alignment) {
  ScopedLock s(&Lock);

  Release();

  CreateBuffer_LowLevel(
      bufferBits,
      EVulkanMemoryBits::HOST_VISIBLE | EVulkanMemoryBits::HOST_COHERENT,
      VkDeviceSize(totalSize),
      Buffer,
      BufferMemory,
      RingBufferSize);

  RingBufferOffset = 0;
  Alignment        = alignment;

  Map(0, RingBufferSize);
}

void RingBufferVk::Reset() {
  ScopedLock s(&Lock);

  RingBufferOffset = 0;
}

uint64_t RingBufferVk::Alloc(uint64_t allocSize) {
  ScopedLock s(&Lock);

  const uint64_t allocOffset = Align<uint64_t>(RingBufferOffset, Alignment);
  if (allocOffset + allocSize <= RingBufferSize) {
    RingBufferOffset = allocOffset + allocSize;
    return allocOffset;
  }

  assert(0);

  // TODO: No wrap-around handling in code above. this is not correct ring
  // buffer, it should start from the begining after it exceeds the ring
  // buffer size. Below is proposed solution (still WIP)
  // uint64_t endOffset = allocOffset + allocSize;
  // if (endOffset <= RingBufferSize) {
  //  RingBufferOffset = endOffset;  // Normal allocation
  //} else {
  //  // Check if there's enough space from the start of the buffer
  //  if (allocSize <= allocOffset) {  // Assumes single wrap around is enough
  //    RingBufferOffset = allocSize;  // Wrap around
  //    allocOffset      = 0;          // Start from the beginning
  //  } else {
  //    assert(0);  // Not enough space, handle as an error or implement a
  //    more
  //                // complex allocation strategy
  //  }
  //}

  return 0;
}

void RingBufferVk::Release() {
  assert(g_rhi_vk->m_device_);

  if (Buffer) {
    vkDestroyBuffer(g_rhi_vk->m_device_, Buffer, nullptr);
    Buffer = nullptr;
  }

  if (BufferMemory) {
    vkFreeMemory(g_rhi_vk->m_device_, BufferMemory, nullptr);
    BufferMemory = nullptr;
  }

  RingBufferSize = 0;
  MappedPointer  = nullptr;
}

void* RingBufferVk::Map(uint64_t offset, uint64_t size) {
  assert(size);
  assert(offset + size <= RingBufferSize);
  assert(!MappedPointer);
  vkMapMemory(
      g_rhi_vk->m_device_, BufferMemory, offset, size, 0, &MappedPointer);
  return MappedPointer;
}

void* RingBufferVk::Map() {
  assert(RingBufferSize);
  assert(!MappedPointer);
  vkMapMemory(
      g_rhi_vk->m_device_, BufferMemory, 0, VK_WHOLE_SIZE, 0, &MappedPointer);
  return MappedPointer;
}

void RingBufferVk::Unmap() {
  assert(MappedPointer);
  vkUnmapMemory(g_rhi_vk->m_device_, BufferMemory);
  MappedPointer = nullptr;
}

void RingBufferVk::UpdateBuffer(const void* data, uint64_t size) {
  assert(size <= RingBufferSize);

  if (Map(0, size) != nullptr) {
    memcpy(MappedPointer, data, size);
    Unmap();
  } else {
    GlobalLogger::Log(LogLevel::Error, "Failed to map ring buffer");
  }
}

}  // namespace game_engine
