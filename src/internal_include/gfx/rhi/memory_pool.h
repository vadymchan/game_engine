#ifndef GAME_ENGINE_MEMORY_POOL_H
#define GAME_ENGINE_MEMORY_POOL_H

#include "gfx/rhi/lock.h"
#include "gfx/rhi/rhi_type.h"

#include <cstdint>
#include <vector>

namespace game_engine {

class SubMemoryAllocator;

// Memory range in sub memory
struct Range {
  uint64_t m_offset_   = 0;
  uint64_t m_dataSize_ = 0;
};

struct Memory {
  bool IsValid() const { return m_buffer; }

  void* GetBuffer() const { return m_buffer; }

  void*               GetMappedPointer() const;
  void*               GetMemory() const;
  void                Free();
  void                Reset();
  void*               m_buffer = nullptr;
  Range               m_range;
  SubMemoryAllocator* m_subMemoryAllocator = nullptr;
};

class SubMemoryAllocator {
  public:
  friend class MemoryPool;

  virtual ~SubMemoryAllocator() {}

  virtual void Initialize(EVulkanBufferBits InUsage,
                          EVulkanMemoryBits InProperties,
                          uint64_t          size)
      = 0;

  virtual void* GetBuffer() const { return nullptr; }

  virtual void* GetMemory() const { return nullptr; }

  virtual void* GetMappedPointer() const { return m_mappedPointer_; }

  virtual Memory Alloc(uint64_t InRequstedSize);

  virtual bool IsMatchType(EVulkanBufferBits InUsages,
                           EVulkanMemoryBits InProperties) const {
    return (m_usages_ == InUsages) && (m_properties_ == InProperties);
  }

  protected:
  virtual void Free(const Memory& InFreeMemory) {
    ScopedLock s(&m_lock_);

    // If All of the allocated memory returned then clear allocated list to use
    // all area of the submemory allocator
    if (m_allAllocatedLists_.size() == m_freeLists_.size() + 1) {
      m_allAllocatedLists_.clear();
      m_freeLists_.clear();
    } else {
      m_freeLists_.push_back(InFreeMemory.m_range);
    }
  }

  MutexLock          m_lock_;
  void*              m_mappedPointer_ = nullptr;
  std::vector<Range> m_freeLists_;
  std::vector<Range> m_allAllocatedLists_;
  Range              m_subMemoryRange_;
  EVulkanBufferBits  m_usages_     = EVulkanBufferBits::TRANSFER_SRC;
  EVulkanMemoryBits  m_properties_ = EVulkanMemoryBits::DEVICE_LOCAL;
  uint64_t           m_alignment_  = 16;
};

class MemoryPool {
  public:
  virtual ~MemoryPool() {}
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
  static constexpr uint64_t s_kMemorySize[(int32_t)EPoolSizeType::MAX] = {
    128,        // E128
    256,        // E256
    512,        // E512
    1024,       // E1K
    2048,       // E2K
    4096,       // E4K
    8192,       // E8K
    16 * 1024,  // E16K
  };

  static constexpr uint64_t s_kSubMemorySize[(int32_t)EPoolSizeType::MAX] = {
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

    PendingFreeMemory(int32_t InFrameIndex, const Memory& InMemory)
        : m_frameIndex_(InFrameIndex)
        , m_memory_(InMemory) {}

    int32_t m_frameIndex_ = 0;
    Memory  m_memory_;
  };

  static constexpr int32_t s_kNumOfFramesToWaitBeforeReleasing = 3;

  virtual SubMemoryAllocator* CreateSubMemoryAllocator() const = 0;

  // select the appropriate PoolSize
  virtual EPoolSizeType GetPoolSizeType(uint64_t size) const {
    for (int32_t i = 0; i < (int32_t)EPoolSizeType::MAX; ++i) {
      if (s_kMemorySize[i] > size) {
        return (EPoolSizeType)i;
      }
    }
    return EPoolSizeType::MAX;
  }

  virtual Memory Alloc(EVulkanBufferBits InUsages,
                       EVulkanMemoryBits InProperties,
                       uint64_t          size);

  virtual void Free(const Memory& InFreeMemory);

  MutexLock                        m_lock_;
  std::vector<SubMemoryAllocator*> m_memoryPools_[(int32_t)EPoolSizeType::MAX + 1];
  std::vector<PendingFreeMemory>   m_pendingFree_;
  int32_t                          m_canReleasePendingFreeMemoryFrameNumber_ = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_MEMORY_POOL_H