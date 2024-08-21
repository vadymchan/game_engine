#include "gfx/rhi/memory_pool.h"

#include "gfx/rhi/rhi.h"
#include "utils/memory/align.h"

#include <cassert>

namespace game_engine {

void* Memory::getMappedPointer() const {
  return m_subMemoryAllocator->getMappedPointer();
}

void* Memory::getMemory() const {
  return m_subMemoryAllocator->getMemory();
}

//////////////////////////////////////////////////////////////////////////
// Memory
void Memory::free() {
  g_rhi->getMemoryPool()->free(*this);
  reset();
}

void Memory::reset() {
  m_buffer             = nullptr;
  m_range.m_offset_    = 0;
  m_range.m_dataSize_  = 0;
  m_subMemoryAllocator = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// SubMemoryAllocator
Memory SubMemoryAllocator::alloc(uint64_t requstedSize) {
  ScopedLock s(&m_lock_);

  Memory         AllocMem;
  const uint64_t AlignedRequestedSize
      = (m_alignment_ > 0) ? g_align(requstedSize, m_alignment_) : requstedSize;

  for (int32_t i = 0; i < (int32_t)m_freeLists_.size(); ++i) {
    if (m_freeLists_[i].m_dataSize_ >= AlignedRequestedSize) {
      AllocMem.m_buffer = getBuffer();
      assert(AllocMem.m_buffer);

      AllocMem.m_range.m_offset_    = m_freeLists_[i].m_offset_;
      AllocMem.m_range.m_dataSize_  = AlignedRequestedSize;
      AllocMem.m_subMemoryAllocator = this;
      m_freeLists_.erase(m_freeLists_.begin() + i);
      return AllocMem;
    }
  }

  if ((m_subMemoryRange_.m_offset_ + AlignedRequestedSize)
      <= m_subMemoryRange_.m_dataSize_) {
    AllocMem.m_buffer = getBuffer();
    assert(AllocMem.m_buffer);

    AllocMem.m_range.m_offset_
        = (m_alignment_ > 0) ? g_align(m_subMemoryRange_.m_offset_, m_alignment_)
                             : m_subMemoryRange_.m_offset_;
    AllocMem.m_range.m_dataSize_  = AlignedRequestedSize;
    AllocMem.m_subMemoryAllocator = this;

    m_subMemoryRange_.m_offset_ += AlignedRequestedSize;
    m_allAllocatedLists_.push_back(AllocMem.m_range);

    assert(AllocMem.m_range.m_offset_ + AllocMem.m_range.m_dataSize_
           <= m_subMemoryRange_.m_dataSize_);
  }
  return AllocMem;
}

Memory MemoryPool::alloc(EVulkanBufferBits usages,
                         EVulkanMemoryBits properties,
                         uint64_t          size) {
  ScopedLock          s(&m_lock_);
  const EPoolSizeType PoolSizeType = getPoolSizeType(size);

  std::vector<SubMemoryAllocator*>& SubMemoryAllocators
      = m_memoryPools_[(int32_t)PoolSizeType];
  for (auto& iter : SubMemoryAllocators) {
    if (!iter->isMatchType(usages, properties)) {
      continue;
    }

    const Memory& alloc = iter->alloc(size);
    if (alloc.isValid()) {
      return alloc;
    }
  }

  // Add new memory
  SubMemoryAllocator* NewSubMemoryAllocator = createSubMemoryAllocator();
  SubMemoryAllocators.push_back(NewSubMemoryAllocator);

  // Use the entire SubMemoryAllocator that exceeds the maximum supported memory
  // size
  const uint64_t SubMemoryAllocatorSize
      = (PoolSizeType == EPoolSizeType::MAX)
          ? size
          : s_kSubMemorySize[(uint64_t)PoolSizeType];

  NewSubMemoryAllocator->initialize(
      usages, properties, SubMemoryAllocatorSize);
  const Memory& alloc = NewSubMemoryAllocator->alloc(size);
  assert(alloc.isValid());

  return alloc;
}

void MemoryPool::free(const Memory& freeMemory) {
  ScopedLock s(&m_lock_);

  const int32_t CurrentFrameNumber = g_rhi->getCurrentFrameNumber();
  const int32_t OldestFrameToKeep
      = CurrentFrameNumber - s_kNumOfFramesToWaitBeforeReleasing;

  // ProcessPendingMemoryFree
  {
    // Check it is too early
    if (CurrentFrameNumber >= m_canReleasePendingFreeMemoryFrameNumber_) {
      // Release pending memory
      int32_t i = 0;
      for (; i < m_pendingFree_.size(); ++i) {
        PendingFreeMemory& pendingFreeMemory = m_pendingFree_[i];
        if (pendingFreeMemory.m_frameIndex_ < OldestFrameToKeep) {
          assert(pendingFreeMemory.m_memory_.m_subMemoryAllocator);
          pendingFreeMemory.m_memory_.m_subMemoryAllocator->free_(
              pendingFreeMemory.m_memory_);
        } else {
          m_canReleasePendingFreeMemoryFrameNumber_
              = pendingFreeMemory.m_frameIndex_
              + s_kNumOfFramesToWaitBeforeReleasing + 1;
          break;
        }
      }
      if (i > 0) {
        const size_t RemainingSize = (m_pendingFree_.size() - i);
        if (RemainingSize > 0) {
          memcpy(&m_pendingFree_[0],
                 &m_pendingFree_[i],
                 sizeof(PendingFreeMemory) * RemainingSize);
        }
        m_pendingFree_.resize(RemainingSize);
      }
    }
  }

  m_pendingFree_.emplace_back(
      PendingFreeMemory(CurrentFrameNumber, freeMemory));
}

}  // namespace game_engine