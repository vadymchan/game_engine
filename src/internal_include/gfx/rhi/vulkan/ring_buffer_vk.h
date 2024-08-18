#ifndef GAME_ENGINE_RING_BUFFER_VK_H
#define GAME_ENGINE_RING_BUFFER_VK_H

#include "gfx/rhi/buffer.h"
#include "gfx/rhi/lock.h"
#include "gfx/rhi/vulkan/utils_vk.h"

#include <cstdint>

namespace game_engine {

struct RingBufferVk : public Buffer {
  RingBufferVk() = default;

  virtual ~RingBufferVk() { Release(); }

  virtual void Create(EVulkanBufferBits bufferBits,
                      uint64_t          totalSize,
                      uint32_t          alignment = 16);

  virtual void Reset();

  virtual uint64_t Alloc(uint64_t allocSize);

  virtual void Release() override;

  virtual void* GetMappedPointer() const override { return m_mappedPointer_; }

  // TODO: for map use 1 method (combine to 1)
  virtual void* Map(uint64_t offset, uint64_t size) override;

  virtual void* Map() override;

  virtual void Unmap() override;

  virtual void UpdateBuffer(const void* data, uint64_t size) override;

  virtual void* GetHandle() const override { return m_buffer_; }

  virtual uint64_t GetAllocatedSize() const override { return m_ringBufferSize_; }

  virtual uint64_t GetBufferSize() const override { return m_ringBufferSize_; }

  virtual uint64_t GetOffset() const override { return m_ringBufferOffset_; }

  uint64_t       m_ringBufferOffset_ = 0;
  uint32_t       m_alignment_        = 16;
  VkBuffer       m_buffer_           = nullptr;
  VkDeviceMemory m_bufferMemory_     = nullptr;
  uint64_t       m_ringBufferSize_   = 0;
  void*          m_mappedPointer_    = nullptr;

  MutexLock Lock;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RING_BUFFER_VK_H