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
  // ======= BEGIN: public destructor =========================================

  virtual ~SubMemoryAllocatorVk() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void initialize(EVulkanBufferBits usage,
                          EVulkanMemoryBits properties,
                          uint64_t          size) override;

  virtual void* getBuffer() const { return m_buffer_; }

  virtual void* getMemory() const { return m_deviceMemory_; }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  VkBuffer       m_buffer_       = nullptr;
  VkDeviceMemory m_deviceMemory_ = nullptr;

  // ======= END: public misc fields   ========================================
};

class MemoryPoolVk : public MemoryPool {
  public:
  // ======= BEGIN: public destructor =========================================

  virtual ~MemoryPoolVk() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual SubMemoryAllocator* createSubMemoryAllocator() const override;

  // ======= END: public overridden methods   =================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_MEMORY_POOL_VK_H
