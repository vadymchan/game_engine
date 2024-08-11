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
  Buffer             = nullptr;
  m_range.Offset       = 0;
  m_range.DataSize     = 0;
  m_subMemoryAllocator = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// SubMemoryAllocator
Memory SubMemoryAllocator::Alloc(uint64_t InRequstedSize) {
  ScopedLock s(&Lock);

  Memory        AllocMem;
  const uint64_t AlignedRequestedSize
      = (Alignment > 0) ? Align(InRequstedSize, Alignment) : InRequstedSize;

  for (int32_t i = 0; i < (int32_t)FreeLists.size(); ++i) {
    if (FreeLists[i].DataSize >= AlignedRequestedSize) {
      AllocMem.Buffer = GetBuffer();
      assert(AllocMem.Buffer);

      AllocMem.m_range.Offset       = FreeLists[i].Offset;
      AllocMem.m_range.DataSize     = AlignedRequestedSize;
      AllocMem.m_subMemoryAllocator = this;
      FreeLists.erase(FreeLists.begin() + i);
      return AllocMem;
    }
  }

  if ((SubMemoryRange.Offset + AlignedRequestedSize)
      <= SubMemoryRange.DataSize) {
    AllocMem.Buffer = GetBuffer();
    assert(AllocMem.Buffer);

    AllocMem.m_range.Offset       = (Alignment > 0)
                                    ? Align(SubMemoryRange.Offset, Alignment)
                                    : SubMemoryRange.Offset;
    AllocMem.m_range.DataSize     = AlignedRequestedSize;
    AllocMem.m_subMemoryAllocator = this;

    SubMemoryRange.Offset += AlignedRequestedSize;
    AllAllocatedLists.push_back(AllocMem.m_range);

    assert(AllocMem.m_range.Offset + AllocMem.m_range.DataSize
          <= SubMemoryRange.DataSize);
  }
  return AllocMem;
}

Memory MemoryPool::Alloc(EVulkanBufferBits InUsages,
                           EVulkanMemoryBits InProperties,
                           uint64_t          InSize) {
  ScopedLock         s(&Lock);
  const EPoolSizeType PoolSizeType = GetPoolSizeType(InSize);

  std::vector<SubMemoryAllocator*>& SubMemoryAllocators
      = MemoryPools[(int32_t)PoolSizeType];
  for (auto& iter : SubMemoryAllocators) {
    if (!iter->IsMatchType(InUsages, InProperties)) {
      continue;
    }

    const Memory& alloc = iter->Alloc(InSize);
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
          ? InSize
          : SubMemorySize[(uint64_t)PoolSizeType];

  NewSubMemoryAllocator->Initialize(
      InUsages, InProperties, SubMemoryAllocatorSize);
  const Memory& alloc = NewSubMemoryAllocator->Alloc(InSize);
  assert(alloc.IsValid());

  return alloc;
}

void MemoryPool::Free(const Memory& InFreeMemory) {
  ScopedLock s(&Lock);

  const int32_t CurrentFrameNumber = g_rhi->GetCurrentFrameNumber();
  const int32_t OldestFrameToKeep
      = CurrentFrameNumber - NumOfFramesToWaitBeforeReleasing;

  // ProcessPendingMemoryFree
  {
    // Check it is too early
    if (CurrentFrameNumber >= CanReleasePendingFreeMemoryFrameNumber) {
      // Release pending memory
      int32_t i = 0;
      for (; i < PendingFree.size(); ++i) {
        PendingFreeMemory& pendingFreeMemory = PendingFree[i];
        if (pendingFreeMemory.FrameIndex < OldestFrameToKeep) {
          assert(pendingFreeMemory.m_memory.m_subMemoryAllocator);
          pendingFreeMemory.m_memory.m_subMemoryAllocator->Free(
              pendingFreeMemory.m_memory);
        } else {
          CanReleasePendingFreeMemoryFrameNumber
              = pendingFreeMemory.FrameIndex + NumOfFramesToWaitBeforeReleasing
              + 1;
          break;
        }
      }
      if (i > 0) {
        const size_t RemainingSize = (PendingFree.size() - i);
        if (RemainingSize > 0) {
          memcpy(&PendingFree[0],
                 &PendingFree[i],
                 sizeof(PendingFreeMemory) * RemainingSize);
        }
        PendingFree.resize(RemainingSize);
      }
    }
  }

  PendingFree.emplace_back(
      PendingFreeMemory(CurrentFrameNumber, InFreeMemory));
}

}  // namespace game_engine