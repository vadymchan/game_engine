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

  virtual void* GetMappedPointer() const override { return MappedPointer; }

  // TODO: for map use 1 method (combine to 1)
  virtual void* Map(uint64_t offset, uint64_t size) override;

  virtual void* Map() override;

  virtual void Unmap() override;

  virtual void UpdateBuffer(const void* data, uint64_t size) override;

  virtual void* GetHandle() const override { return Buffer; }

  virtual uint64_t GetAllocatedSize() const override { return RingBufferSize; }

  virtual uint64_t GetBufferSize() const override { return RingBufferSize; }

  virtual uint64_t GetOffset() const override { return RingBufferOffset; }

  uint64_t       RingBufferOffset = 0;
  uint32_t       Alignment        = 16;
  VkBuffer       Buffer           = nullptr;
  VkDeviceMemory BufferMemory     = nullptr;
  uint64_t       RingBufferSize   = 0;
  void*          MappedPointer    = nullptr;

  MutexLock Lock;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RING_BUFFER_VK_H