#include "gfx/rhi/memory_pool.h"

#include "gfx/rhi/rhi.h"
#include "utils/memory/align.h"

#include <cassert>

namespace game_engine {

void* jMemory::GetMappedPointer() const {
  return SubMemoryAllocator->GetMappedPointer();
}

void* jMemory::GetMemory() const {
  return SubMemoryAllocator->GetMemory();
}

//////////////////////////////////////////////////////////////////////////
// jMemory
void jMemory::Free() {
  g_rhi->GetMemoryPool()->Free(*this);
  Reset();
}

void jMemory::Reset() {
  Buffer             = nullptr;
  Range.Offset       = 0;
  Range.DataSize     = 0;
  SubMemoryAllocator = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// jSubMemoryAllocator
jMemory jSubMemoryAllocator::Alloc(uint64_t InRequstedSize) {
  ScopedLock s(&Lock);

  jMemory        AllocMem;
  const uint64_t AlignedRequestedSize
      = (Alignment > 0) ? Align(InRequstedSize, Alignment) : InRequstedSize;

  for (int32_t i = 0; i < (int32_t)FreeLists.size(); ++i) {
    if (FreeLists[i].DataSize >= AlignedRequestedSize) {
      AllocMem.Buffer = GetBuffer();
      assert(AllocMem.Buffer);

      AllocMem.Range.Offset       = FreeLists[i].Offset;
      AllocMem.Range.DataSize     = AlignedRequestedSize;
      AllocMem.SubMemoryAllocator = this;
      FreeLists.erase(FreeLists.begin() + i);
      return AllocMem;
    }
  }

  if ((SubMemoryRange.Offset + AlignedRequestedSize)
      <= SubMemoryRange.DataSize) {
    AllocMem.Buffer = GetBuffer();
    assert(AllocMem.Buffer);

    AllocMem.Range.Offset       = (Alignment > 0)
                                    ? Align(SubMemoryRange.Offset, Alignment)
                                    : SubMemoryRange.Offset;
    AllocMem.Range.DataSize     = AlignedRequestedSize;
    AllocMem.SubMemoryAllocator = this;

    SubMemoryRange.Offset += AlignedRequestedSize;
    AllAllocatedLists.push_back(AllocMem.Range);

    assert(AllocMem.Range.Offset + AllocMem.Range.DataSize
          <= SubMemoryRange.DataSize);
  }
  return AllocMem;
}

jMemory jMemoryPool::Alloc(EVulkanBufferBits InUsages,
                           EVulkanMemoryBits InProperties,
                           uint64_t          InSize) {
  ScopedLock         s(&Lock);
  const EPoolSizeType PoolSizeType = GetPoolSizeType(InSize);

  std::vector<jSubMemoryAllocator*>& SubMemoryAllocators
      = MemoryPools[(int32_t)PoolSizeType];
  for (auto& iter : SubMemoryAllocators) {
    if (!iter->IsMatchType(InUsages, InProperties)) {
      continue;
    }

    const jMemory& alloc = iter->Alloc(InSize);
    if (alloc.IsValid()) {
      return alloc;
    }
  }

  // Add new memory
  jSubMemoryAllocator* NewSubMemoryAllocator = CreateSubMemoryAllocator();
  SubMemoryAllocators.push_back(NewSubMemoryAllocator);

  // Use the entire SubMemoryAllocator that exceeds the maximum supported memory
  // size
  const uint64_t SubMemoryAllocatorSize
      = (PoolSizeType == EPoolSizeType::MAX)
          ? InSize
          : SubMemorySize[(uint64_t)PoolSizeType];

  NewSubMemoryAllocator->Initialize(
      InUsages, InProperties, SubMemoryAllocatorSize);
  const jMemory& alloc = NewSubMemoryAllocator->Alloc(InSize);
  assert(alloc.IsValid());

  return alloc;
}

void jMemoryPool::Free(const jMemory& InFreeMemory) {
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
        jPendingFreeMemory& PendingFreeMemory = PendingFree[i];
        if (PendingFreeMemory.FrameIndex < OldestFrameToKeep) {
          assert(PendingFreeMemory.Memory.SubMemoryAllocator);
          PendingFreeMemory.Memory.SubMemoryAllocator->Free(
              PendingFreeMemory.Memory);
        } else {
          CanReleasePendingFreeMemoryFrameNumber
              = PendingFreeMemory.FrameIndex + NumOfFramesToWaitBeforeReleasing
              + 1;
          break;
        }
      }
      if (i > 0) {
        const size_t RemainingSize = (PendingFree.size() - i);
        if (RemainingSize > 0) {
          memcpy(&PendingFree[0],
                 &PendingFree[i],
                 sizeof(jPendingFreeMemory) * RemainingSize);
        }
        PendingFree.resize(RemainingSize);
      }
    }
  }

  PendingFree.emplace_back(
      jPendingFreeMemory(CurrentFrameNumber, InFreeMemory));
}

}  // namespace game_engine