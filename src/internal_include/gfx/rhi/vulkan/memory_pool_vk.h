#ifndef GAME_ENGINE_MEMORY_POOL_VK_H
#define GAME_ENGINE_MEMORY_POOL_VK_H

#define USE_VK_MEMORY_POOL 1

#include "gfx/rhi/lock.h"
#include "gfx/rhi/vulkan/utils_vk.h"
#include "gfx/rhi/vulkan/rhi_type_vk.h"

#include <vulkan/vulkan.h>

#include <cassert>
#include <vector>

namespace game_engine {

class SubMemoryAllocator;

// Memory range in sub memory
struct Range {
  uint64_t Offset   = 0;
  uint64_t DataSize = 0;
};

struct MemoryVk {
  bool IsValid() const { return Buffer; }

  void* GetBuffer() const { return Buffer; }

  void* GetMappedPointer() const;

  void* GetMemory() const;
  void Free();

  void Reset() {
    Buffer             = nullptr;
    Range.Offset       = 0;
    Range.DataSize     = 0;
    subMemoryAllocator = nullptr;
  }

  void*               Buffer = nullptr;
  Range               Range;
  SubMemoryAllocator* subMemoryAllocator = nullptr;
};

class SubMemoryAllocator {
  public:
  friend class MemoryPoolVk;

  virtual ~SubMemoryAllocator() {}

  virtual void Initialize(EVulkanBufferBits InUsage,
                          EVulkanMemoryBits InProperties,
                          uint64_t              InSize);

  virtual void* GetBuffer() const { return Buffer; }

  virtual void* GetMemory() const { return DeviceMemory; }

  virtual void* GetMappedPointer() const { return MappedPointer; }

  virtual MemoryVk Alloc(uint64_t InRequstedSize);

  virtual bool IsMatchType(EVulkanBufferBits InUsages,
                           EVulkanMemoryBits InProperties) const {
    return (Usages == InUsages) && (Properties == InProperties);
  }

  protected:
  virtual void Free(const MemoryVk& InFreeMemory);

  MutexLock             Lock;
  void*                 MappedPointer = nullptr;
  std::vector<Range>    FreeLists;
  std::vector<Range>    AllAllocatedLists;
  Range                 SubMemoryRange;
  EVulkanBufferBits     Usages     = EVulkanBufferBits::TRANSFER_SRC;
  EVulkanMemoryBits     Properties = EVulkanMemoryBits::DEVICE_LOCAL;
  uint64_t              Alignment  = 16;

  VkBuffer       Buffer       = nullptr;
  VkDeviceMemory DeviceMemory = nullptr;
};

class MemoryPoolVk {
  public:
  virtual ~MemoryPoolVk() {}
  enum class EPoolSizeType : uint8_t {
    E128,
    E256,
    E512,
    E1K,
    E2K,
    E4K,
    E8K,
    E16K,
    MAX
  };

  // enum class EPoolSize : uint64_t
  static constexpr uint64_t MemorySize[(int32_t)EPoolSizeType::MAX] = {
    128,        // E128
    256,        // E256
    512,        // E512
    1024,       // E1K
    2048,       // E2K
    4096,       // E4K
    8192,       // E8K
    16 * 1024,  // E16K
  };

  static constexpr uint64_t SubMemorySize[(int32_t)EPoolSizeType::MAX] = {
    128 * 1024,
    128 * 1024,
    256 * 1024,
    256 * 1024,
    512 * 1024,
    512 * 1024,
    1024 * 1024,
    1024 * 1024,
  };

  struct PendingFreeMemory {
    PendingFreeMemory() = default;

    PendingFreeMemory(int32_t InFrameIndex, const MemoryVk& InMemory)
        : FrameIndex(InFrameIndex)
        , Memory(InMemory) {}

    int32_t  FrameIndex = 0;
    MemoryVk Memory;
  };

  static constexpr int32_t NumOfFramesToWaitBeforeReleasing = 3;

  virtual SubMemoryAllocator* CreateSubMemoryAllocator() const;

  virtual EPoolSizeType GetPoolSizeType(uint64_t InSize) const;

  virtual MemoryVk Alloc(EVulkanBufferBits InUsages,
                         EVulkanMemoryBits InProperties,
                         uint64_t          InSize);

  virtual void Free(const MemoryVk& InFreeMemory);

  MutexLock                        Lock;
  std::vector<SubMemoryAllocator*> MemoryPools[(int32_t)EPoolSizeType::MAX + 1];
  std::vector<PendingFreeMemory>   PendingFree;
  int32_t                          CanReleasePendingFreeMemoryFrameNumber = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_MEMORY_POOL_VK_H
