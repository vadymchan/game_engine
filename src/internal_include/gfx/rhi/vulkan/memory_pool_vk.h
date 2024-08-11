#ifndef GAME_ENGINE_MEMORY_POOL_VK_H
#define GAME_ENGINE_MEMORY_POOL_VK_H

#define USE_VK_MEMORY_POOL 1

#include "gfx/rhi/lock.h"
#include "gfx/rhi/memory_pool.h"
#include "gfx/rhi/vulkan/rhi_type_vk.h"
#include "gfx/rhi/vulkan/utils_vk.h"

#include <vulkan/vulkan.h>

#include <cassert>
#include <vector>

namespace game_engine {


class SubMemoryAllocatorVk : public SubMemoryAllocator {
  public:

  virtual ~SubMemoryAllocatorVk() {}

  virtual void Initialize(EVulkanBufferBits InUsage,
                          EVulkanMemoryBits InProperties,
                          uint64_t          InSize) override;

  virtual void* GetBuffer() const { return Buffer; }

  virtual void* GetMemory() const { return DeviceMemory; }

  VkBuffer       Buffer       = nullptr;
  VkDeviceMemory DeviceMemory = nullptr;
};

class MemoryPoolVk : public MemoryPool {
  public:
  virtual ~MemoryPoolVk() {}

  virtual SubMemoryAllocator* CreateSubMemoryAllocator() const override;

};

}  // namespace game_engine

#endif  // GAME_ENGINE_MEMORY_POOL_VK_H
