#ifndef GAME_ENGINE_RING_BUFFER_VK_H
#define GAME_ENGINE_RING_BUFFER_VK_H

#include "gfx/rhi/buffer.h"
#include "gfx/rhi/lock.h"
#include "gfx/rhi/vulkan/utils_vk.h"

#include <cstdint>

namespace game_engine {

struct RingBufferVk : public IBuffer {
  RingBufferVk() = default;

  virtual ~RingBufferVk() { release(); }

  virtual void create(EVulkanBufferBits bufferBits,
                      uint64_t          totalSize,
                      uint32_t          alignment = 16);

  virtual void reset();

  virtual uint64_t alloc(uint64_t allocSize);

  virtual void release() override;

  virtual void* getMappedPointer() const override { return m_mappedPointer_; }

  // TODO: for map use 1 method (combine to 1)
  virtual void* map(uint64_t offset, uint64_t size) override;

  virtual void* map() override;

  virtual void unmap() override;

  virtual void updateBuffer(const void* data, uint64_t size) override;

  virtual void* getHandle() const override { return m_buffer_; }

  virtual uint64_t getAllocatedSize() const override { return m_ringBufferSize_; }

  virtual uint64_t getBufferSize() const override { return m_ringBufferSize_; }

  virtual uint64_t getOffset() const override { return m_ringBufferOffset_; }

  uint64_t       m_ringBufferOffset_ = 0;
  uint32_t       m_alignment_        = 16;
  VkBuffer       m_buffer_           = nullptr;
  VkDeviceMemory m_bufferMemory_     = nullptr;
  uint64_t       m_ringBufferSize_   = 0;
  void*          m_mappedPointer_    = nullptr;

  MutexLock m_lock_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RING_BUFFER_VK_H