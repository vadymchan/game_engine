#ifndef GAME_ENGINE_RING_BUFFER_VK_H
#define GAME_ENGINE_RING_BUFFER_VK_H

#include "gfx/rhi/lock.h"
#include "gfx/rhi/vulkan/utils_vk.h"

#include <cstdint>

namespace game_engine {

struct RingBufferVk {
  RingBufferVk() = default;

  virtual ~RingBufferVk() { Release(); }

  virtual void Create(EVulkanBufferBits bufferBits,
                      uint64_t          totalSize,
                      uint32_t          alignment = 16);

  virtual void Reset();

  virtual uint64_t Alloc(uint64_t allocSize);

  virtual void Release();

  virtual void* GetMappedPointer() const { return MappedPointer; }

  virtual void* Map(uint64_t offset, uint64_t size);

  virtual void* Map();

  virtual void Unmap();

  virtual void UpdateBuffer(const void* data, uint64_t size);

  virtual void* GetHandle() const { return Buffer; }

  virtual uint64_t GetAllocatedSize() const { return RingBufferSize; }

  virtual uint64_t GetBufferSize() const { return RingBufferSize; }

  virtual uint64_t GetOffset() const { return RingBufferOffset; }

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