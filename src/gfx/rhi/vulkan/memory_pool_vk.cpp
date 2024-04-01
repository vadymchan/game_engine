

#include "gfx/rhi/vulkan/memory_pool_vk.h"

#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

void* MemoryVk::GetMappedPointer() const {
  return subMemoryAllocator->GetMappedPointer();
}

void* MemoryVk::GetMemory() const {
  return subMemoryAllocator->GetMemory();
}

void MemoryVk::Free() {
  g_rhi_vk->GetMemoryPool()->Free(*this);
  Reset();
}

// TODO: some mess with the order of parameters
void SubMemoryAllocator::Initialize(EVulkanBufferBits InUsage,
                                    EVulkanMemoryBits InProperties,
                                    uint64_t          InSize) {
  assert(SubMemoryRange.Offset == 0 && SubMemoryRange.DataSize == 0);
  assert(0 == FreeLists.size());

  SubMemoryRange.Offset = 0;
  CreateBuffer_LowLevel(InUsage,
                        InProperties,
                        InSize,
                        Buffer,
                        DeviceMemory,
                        SubMemoryRange.DataSize);
  Usages     = InUsage;
  Properties = InProperties;

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(g_rhi_vk->m_device_, Buffer, &memRequirements);
  Alignment = memRequirements.alignment;

  if (!!(InProperties & EVulkanMemoryBits::HOST_VISIBLE)) {
    assert(vkMapMemory(g_rhi_vk->m_device_,
                       DeviceMemory,
                       0,
                       SubMemoryRange.DataSize,
                       0,
                       &MappedPointer)
           == VK_SUCCESS);
  }
}

MemoryVk SubMemoryAllocator::Alloc(uint64_t InRequstedSize) {
  ScopedLock s(&Lock);

  MemoryVk       AllocMem;
  const uint64_t AlignedRequestedSize
      = (Alignment > 0) ? Align(InRequstedSize, Alignment) : InRequstedSize;

  for (int32_t i = 0; i < (int32_t)FreeLists.size(); ++i) {
    if (FreeLists[i].DataSize >= AlignedRequestedSize) {
      AllocMem.Buffer = GetBuffer();
      assert(AllocMem.Buffer);

      AllocMem.Range.Offset       = FreeLists[i].Offset;
      AllocMem.Range.DataSize     = AlignedRequestedSize;
      AllocMem.subMemoryAllocator = this;
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
    AllocMem.subMemoryAllocator = this;

    SubMemoryRange.Offset += AlignedRequestedSize;
    AllAllocatedLists.push_back(AllocMem.Range);

    assert(AllocMem.Range.Offset + AllocMem.Range.DataSize
           <= SubMemoryRange.DataSize);
  }
  return AllocMem;
}

void SubMemoryAllocator::Free(const MemoryVk& InFreeMemory) {
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

SubMemoryAllocator* MemoryPoolVk::CreateSubMemoryAllocator() const {
  return new SubMemoryAllocator();
}

MemoryPoolVk::EPoolSizeType MemoryPoolVk::GetPoolSizeType(
    uint64_t InSize) const {
  for (int32_t i = 0; i < (int32_t)EPoolSizeType::MAX; ++i) {
    if (MemorySize[i] > InSize) {
      return (EPoolSizeType)i;
    }
  }
  return EPoolSizeType::MAX;
}

MemoryVk MemoryPoolVk::Alloc(EVulkanBufferBits InUsages,
                             EVulkanMemoryBits InProperties,
                             uint64_t          InSize) {
  ScopedLock          s(&Lock);
  const EPoolSizeType PoolSizeType = GetPoolSizeType(InSize);

  std::vector<SubMemoryAllocator*>& SubMemoryAllocators
      = MemoryPools[(int32_t)PoolSizeType];
  for (auto& iter : SubMemoryAllocators) {
    if (!iter->IsMatchType(InUsages, InProperties)) {
      continue;
    }

    const MemoryVk& alloc = iter->Alloc(InSize);
    if (alloc.IsValid()) {
      return alloc;
    }
  }

  SubMemoryAllocator* NewSubMemoryAllocator = CreateSubMemoryAllocator();
  SubMemoryAllocators.push_back(NewSubMemoryAllocator);

  const uint64_t SubMemoryAllocatorSize
      = (PoolSizeType == EPoolSizeType::MAX)
          ? InSize
          : SubMemorySize[(uint64_t)PoolSizeType];

  NewSubMemoryAllocator->Initialize(
      InUsages, InProperties, SubMemoryAllocatorSize);
  const MemoryVk& alloc = NewSubMemoryAllocator->Alloc(InSize);
  assert(alloc.IsValid());

  return alloc;
}

void MemoryPoolVk::Free(const MemoryVk& InFreeMemory) {
  ScopedLock s(&Lock);

  const int32_t CurrentFrameNumber = g_rhi_vk->GetCurrentFrameNumber();
  const int32_t OldestFrameToKeep
      = CurrentFrameNumber - NumOfFramesToWaitBeforeReleasing;

  // ProcessPendingMemoryFree
  {
    // Check it is too early
    if (CurrentFrameNumber >= CanReleasePendingFreeMemoryFrameNumber) {
      // Release pending memory
      int32_t i = 0;
      for (; i < PendingFree.size(); ++i) {
        PendingFreeMemory& PendingFreeMemory = PendingFree[i];
        if (PendingFreeMemory.FrameIndex < OldestFrameToKeep) {
          assert(PendingFreeMemory.Memory.subMemoryAllocator);
          PendingFreeMemory.Memory.subMemoryAllocator->Free(
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
                 sizeof(PendingFreeMemory) * RemainingSize);
        }
        PendingFree.resize(RemainingSize);
      }
    }
  }

  PendingFree.emplace_back(PendingFreeMemory(CurrentFrameNumber, InFreeMemory));
}

}  // namespace game_engine
