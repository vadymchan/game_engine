#ifndef GAME_ENGINE_MEMORY_POOL_H
#define GAME_ENGINE_MEMORY_POOL_H

#include "gfx/rhi/lock.h"
#include "gfx/rhi/rhi_type.h"

#include <cstdint>
#include <vector>

namespace game_engine {

class jSubMemoryAllocator;

// Memory range in sub memory
struct jRange {
  uint64_t Offset   = 0;
  uint64_t DataSize = 0;
};

struct jMemory {
  bool IsValid() const { return Buffer; }

  void* GetBuffer() const { return Buffer; }

  void*                GetMappedPointer() const;
  void*                GetMemory() const;
  void                 Free();
  void                 Reset();
  void*                Buffer = nullptr;
  jRange               Range;
  jSubMemoryAllocator* SubMemoryAllocator = nullptr;
};

class jSubMemoryAllocator {
  public:
  friend class jMemoryPool;

  virtual ~jSubMemoryAllocator() {}

  virtual void Initialize(EVulkanBufferBits InUsage,
                          EVulkanMemoryBits InProperties,
                          uint64_t          InSize)
      = 0;

  virtual void* GetBuffer() const { return nullptr; }

  virtual void* GetMemory() const { return nullptr; }

  virtual void* GetMappedPointer() const { return MappedPointer; }

  virtual jMemory Alloc(uint64_t InRequstedSize);

  virtual bool IsMatchType(EVulkanBufferBits InUsages,
                           EVulkanMemoryBits InProperties) const {
    return (Usages == InUsages) && (Properties == InProperties);
  }

  protected:
  virtual void Free(const jMemory& InFreeMemory) {
    ScopedLock s(&Lock);

    // If All of the allocated memory returned then clear allocated list to use
    // all area of the submemory allocator
    if (AllAllocatedLists.size() == FreeLists.size() + 1) {
      AllAllocatedLists.clear();
      FreeLists.clear();
    } else {
      FreeLists.push_back(InFreeMemory.Range);
    }
  }

  MutexLock           Lock;
  void*               MappedPointer = nullptr;
  std::vector<jRange> FreeLists;
  std::vector<jRange> AllAllocatedLists;
  jRange              SubMemoryRange;
  EVulkanBufferBits   Usages     = EVulkanBufferBits::TRANSFER_SRC;
  EVulkanMemoryBits   Properties = EVulkanMemoryBits::DEVICE_LOCAL;
  uint64_t            Alignment  = 16;
};

class jMemoryPool {
  public:
  virtual ~jMemoryPool() {}
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

  // enum class EPoolSize : uint64
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

  struct jPendingFreeMemory {
    jPendingFreeMemory() = default;

    jPendingFreeMemory(int32_t InFrameIndex, const jMemory& InMemory)
        : FrameIndex(InFrameIndex)
        , Memory(InMemory) {}

    int32_t FrameIndex = 0;
    jMemory Memory;
  };

  static constexpr int32_t NumOfFramesToWaitBeforeReleasing = 3;

  virtual jSubMemoryAllocator* CreateSubMemoryAllocator() const = 0;

  // select the appropriate PoolSize
  virtual EPoolSizeType GetPoolSizeType(uint64_t InSize) const {
    for (int32_t i = 0; i < (int32_t)EPoolSizeType::MAX; ++i) {
      if (MemorySize[i] > InSize) {
        return (EPoolSizeType)i;
      }
    }
    return EPoolSizeType::MAX;
  }

  virtual jMemory Alloc(EVulkanBufferBits InUsages,
                        EVulkanMemoryBits InProperties,
                        uint64_t          InSize);

  virtual void Free(const jMemory& InFreeMemory);

  MutexLock Lock;
  std::vector<jSubMemoryAllocator*>
                                  MemoryPools[(int32_t)EPoolSizeType::MAX + 1];
  std::vector<jPendingFreeMemory> PendingFree;
  int32_t                         CanReleasePendingFreeMemoryFrameNumber = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_MEMORY_POOL_H