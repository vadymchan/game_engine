#include "gfx/rhi/memory_pool.h"

#include "gfx/rhi/rhi.h"
#include "utils/memory/align.h"

#include <cassert>

namespace game_engine {

void* Memory::GetMappedPointer() const {
  return m_subMemoryAllocator->GetMappedPointer();
}

void* Memory::GetMemory() const {
  return m_subMemoryAllocator->GetMemory();
}

//////////////////////////////////////////////////////////////////////////
// Memory
void Memory::Free() {
  g_rhi->GetMemoryPool()->Free(*this);
  Reset();
}

void Memory::Reset() {
  m_buffer             = nullptr;
  m_range.m_offset_    = 0;
  m_range.m_dataSize_  = 0;
  m_subMemoryAllocator = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// SubMemoryAllocator
Memory SubMemoryAllocator::Alloc(uint64_t requstedSize) {
  ScopedLock s(&m_lock_);

  Memory         AllocMem;
  const uint64_t AlignedRequestedSize
      = (m_alignment_ > 0) ? Align(requstedSize, m_alignment_) : requstedSize;

  for (int32_t i = 0; i < (int32_t)m_freeLists_.size(); ++i) {
    if (m_freeLists_[i].m_dataSize_ >= AlignedRequestedSize) {
      AllocMem.m_buffer = GetBuffer();
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
    AllocMem.m_buffer = GetBuffer();
    assert(AllocMem.m_buffer);

    AllocMem.m_range.m_offset_
        = (m_alignment_ > 0) ? Align(m_subMemoryRange_.m_offset_, m_alignment_)
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

Memory MemoryPool::Alloc(EVulkanBufferBits usages,
                         EVulkanMemoryBits properties,
                         uint64_t          size) {
  ScopedLock          s(&m_lock_);
  const EPoolSizeType PoolSizeType = GetPoolSizeType(size);

  std::vector<SubMemoryAllocator*>& SubMemoryAllocators
      = m_memoryPools_[(int32_t)PoolSizeType];
  for (auto& iter : SubMemoryAllocators) {
    if (!iter->IsMatchType(usages, properties)) {
      continue;
    }

    const Memory& alloc = iter->Alloc(size);
    if (alloc.IsValid()) {
      return alloc;
    }
  }

  // Add new memory
  SubMemoryAllocator* NewSubMemoryAllocator = CreateSubMemoryAllocator();
  SubMemoryAllocators.push_back(NewSubMemoryAllocator);

  // Use the entire SubMemoryAllocator that exceeds the maximum supported memory
  // size
  const uint64_t SubMemoryAllocatorSize
      = (PoolSizeType == EPoolSizeType::MAX)
          ? size
          : s_kSubMemorySize[(uint64_t)PoolSizeType];

  NewSubMemoryAllocator->Initialize(
      usages, properties, SubMemoryAllocatorSize);
  const Memory& alloc = NewSubMemoryAllocator->Alloc(size);
  assert(alloc.IsValid());

  return alloc;
}

void MemoryPool::Free(const Memory& freeMemory) {
  ScopedLock s(&m_lock_);

  const int32_t CurrentFrameNumber = g_rhi->GetCurrentFrameNumber();
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
          pendingFreeMemory.m_memory_.m_subMemoryAllocator->Free(
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