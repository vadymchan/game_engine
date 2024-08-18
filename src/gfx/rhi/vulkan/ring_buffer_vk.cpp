
#include "gfx/rhi/vulkan/ring_buffer_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"
#include "utils/memory/align.h"

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
      m_buffer_,
      m_bufferMemory_,
      m_ringBufferSize_);

  m_ringBufferOffset_ = 0;
  m_alignment_        = alignment;

  Map(0, m_ringBufferSize_);
}

void RingBufferVk::Reset() {
  ScopedLock s(&Lock);

  m_ringBufferOffset_ = 0;
}

uint64_t RingBufferVk::Alloc(uint64_t allocSize) {
  ScopedLock s(&Lock);

  const uint64_t allocOffset = Align<uint64_t>(m_ringBufferOffset_, m_alignment_);
  if (allocOffset + allocSize <= m_ringBufferSize_) {
    m_ringBufferOffset_ = allocOffset + allocSize;
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

  if (m_buffer_) {
    vkDestroyBuffer(g_rhi_vk->m_device_, m_buffer_, nullptr);
    m_buffer_ = nullptr;
  }

  if (m_bufferMemory_) {
    vkFreeMemory(g_rhi_vk->m_device_, m_bufferMemory_, nullptr);
    m_bufferMemory_ = nullptr;
  }

  m_ringBufferSize_ = 0;
  m_mappedPointer_  = nullptr;
}

void* RingBufferVk::Map(uint64_t offset, uint64_t size) {
  assert(size);
  assert(offset + size <= m_ringBufferSize_);
  assert(!m_mappedPointer_);
  vkMapMemory(
      g_rhi_vk->m_device_, m_bufferMemory_, offset, size, 0, &m_mappedPointer_);
  return m_mappedPointer_;
}

void* RingBufferVk::Map() {
  assert(m_ringBufferSize_);
  assert(!m_mappedPointer_);
  vkMapMemory(
      g_rhi_vk->m_device_, m_bufferMemory_, 0, VK_WHOLE_SIZE, 0, &m_mappedPointer_);
  return m_mappedPointer_;
}

void RingBufferVk::Unmap() {
  assert(m_mappedPointer_);
  vkUnmapMemory(g_rhi_vk->m_device_, m_bufferMemory_);
  m_mappedPointer_ = nullptr;
}

void RingBufferVk::UpdateBuffer(const void* data, uint64_t size) {
  assert(size <= m_ringBufferSize_);

  if (Map(0, size) != nullptr) {
    memcpy(m_mappedPointer_, data, size);
    Unmap();
  } else {
    GlobalLogger::Log(LogLevel::Error, "Failed to map ring buffer");
  }
}

}  // namespace game_engine
