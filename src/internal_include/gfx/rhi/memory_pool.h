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
  // ======= BEGIN: public misc fields ========================================

  uint64_t m_offset_   = 0;
  uint64_t m_dataSize_ = 0;

  // ======= END: public misc fields   ========================================
};

struct Memory {
  // ======= BEGIN: public getters ============================================

  void* getBuffer() const { return m_buffer; }

  void* getMappedPointer() const;
  void* getMemory() const;

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  bool isValid() const { return m_buffer; }

  void free();
  void reset();

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  void*               m_buffer = nullptr;
  Range               m_range;
  SubMemoryAllocator* m_subMemoryAllocator = nullptr;

  // ======= END: public misc fields   ========================================
};

class SubMemoryAllocator {
  public:
  // ======= BEGIN: public nested types =======================================

  friend class MemoryPool;

  // ======= END: public nested types   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~SubMemoryAllocator() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void initialize(EVulkanBufferBits usage,
                          EVulkanMemoryBits properties,
                          uint64_t          size)
      = 0;

  virtual Memory alloc(uint64_t requstedSize);

  virtual bool isMatchType(EVulkanBufferBits usages,
                           EVulkanMemoryBits properties) const {
    return (m_usages_ == usages) && (m_properties_ == properties);
  }

  virtual void* getBuffer() const { return nullptr; }

  virtual void* getMemory() const { return nullptr; }

  virtual void* getMappedPointer() const { return m_mappedPointer_; }

  // ======= END: public overridden methods   =================================

  protected:
  // ======= BEGIN: protected misc methods ====================================

  virtual void free_(const Memory& freeMemory) {
    ScopedLock s(&m_lock_);

    // If All of the allocated memory returned then clear allocated list to use
    // all area of the submemory allocator
    if (m_allAllocatedLists_.size() == m_freeLists_.size() + 1) {
      m_allAllocatedLists_.clear();
      m_freeLists_.clear();
    } else {
      m_freeLists_.push_back(freeMemory.m_range);
    }
  }

  // ======= END: protected misc methods   ====================================

  // ======= BEGIN: protected misc fields =====================================

  MutexLock          m_lock_;
  void*              m_mappedPointer_ = nullptr;
  std::vector<Range> m_freeLists_;
  std::vector<Range> m_allAllocatedLists_;
  Range              m_subMemoryRange_;
  EVulkanBufferBits  m_usages_     = EVulkanBufferBits::TRANSFER_SRC;
  EVulkanMemoryBits  m_properties_ = EVulkanMemoryBits::DEVICE_LOCAL;
  uint64_t           m_alignment_  = 16;

  // ======= END: protected misc fields   =====================================
};

class MemoryPool {
  public:
  // ======= BEGIN: public nested types =======================================

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

    PendingFreeMemory(int32_t frameIndex, const Memory& memory)
        : m_frameIndex_(frameIndex)
        , m_memory_(memory) {}

    int32_t m_frameIndex_ = 0;
    Memory  m_memory_;
  };

  // ======= END: public nested types   =======================================

  // ======= BEGIN: public static fields ======================================

  static constexpr int32_t s_kNumOfFramesToWaitBeforeReleasing = 3;

  // ======= END: public static fields   ======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~MemoryPool() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual SubMemoryAllocator* createSubMemoryAllocator() const = 0;

  virtual Memory alloc(EVulkanBufferBits usages,
                       EVulkanMemoryBits properties,
                       uint64_t          size);

  virtual void free(const Memory& freeMemory);

  // select the appropriate PoolSize
  virtual EPoolSizeType getPoolSizeType(uint64_t size) const {
    for (int32_t i = 0; i < (int32_t)EPoolSizeType::MAX; ++i) {
      if (s_kMemorySize[i] > size) {
        return (EPoolSizeType)i;
      }
    }
    return EPoolSizeType::MAX;
  }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  MutexLock m_lock_;
  std::vector<SubMemoryAllocator*>
      m_memoryPools_[(int32_t)EPoolSizeType::MAX + 1];
  std::vector<PendingFreeMemory> m_pendingFree_;
  int32_t                        m_canReleasePendingFreeMemoryFrameNumber_ = 0;

  // ======= END: public misc fields   ========================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_MEMORY_POOL_H