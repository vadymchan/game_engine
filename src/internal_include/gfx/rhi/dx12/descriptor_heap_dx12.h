#ifndef GAME_ENGINE_DESCRIPTOR_HEAP_DX12_H
#define GAME_ENGINE_DESCRIPTOR_HEAP_DX12_H

#include "gfx/rhi/dx12/rhi_type_dx12.h"
#include "gfx/rhi/lock.h"

#include <set>

namespace game_engine {

// TODO: consider moving this to a separate file
struct DescriptorDx12 {
  static const DescriptorDx12 Invalid;

  D3D12_CPU_DESCRIPTOR_HANDLE               CPUHandle = {};
  D3D12_GPU_DESCRIPTOR_HANDLE               GPUHandle = {};
  uint32_t                                  Index     = uint32_t(-1);
  std::weak_ptr<class DescriptorHeapDx12> DescriptorHeap;

  void Free();

  bool IsValid() const { return Index != -1; }
};

class DescriptorHeapDx12
    : public std::enable_shared_from_this<DescriptorHeapDx12> {
  public:
  static constexpr int32_t NumOfFramesToWaitBeforeReleasing = 3;

  struct PendingForFree {
    uint32_t DescriptorIndex = UINT_MAX;
    uint32_t FrameIndex      = 0;
  };

  void Initialize(EDescriptorHeapTypeDX12 InHeapType,
                  bool                    InShaderVisible,
                  uint32_t                InNumOfDescriptors = 1024);
  void Release();

  DescriptorDx12 Alloc() {
    ScopedLock s(&DescriptorLock);

    if (Pools.empty()) {
      return DescriptorDx12();
    }

    DescriptorDx12 Descriptor;
    Descriptor.Index = *Pools.begin();
    Pools.erase(Pools.begin());

    Descriptor.CPUHandle      = CPUHandleStart;
    Descriptor.CPUHandle.ptr += Descriptor.Index * DescriptorSize;

    Descriptor.GPUHandle      = GPUHandleStart;
    Descriptor.GPUHandle.ptr += Descriptor.Index * DescriptorSize;

    Descriptor.DescriptorHeap = shared_from_this();
    return Descriptor;
  }

  DescriptorDx12 OneFrameAlloc() {
    DescriptorDx12 NewAlloc = Alloc();
    Free(NewAlloc.Index, NumOfFramesToWaitBeforeReleasing);
    return NewAlloc;
  }

  void Free(uint32_t InIndex) {
    ScopedLock s(&DescriptorLock);

    assert(!Pools.contains(InIndex));
    Pools.insert(InIndex);
  }

  void Free(uint32_t InIndex, uint32_t InDelayFrames);
  void ProcessPendingDescriptorPoolFree();

  // Create a Descriptor that will be used only for this frame
  // DescriptorDx12 OneFrameCreateConstantBufferView(RingBufferDx12*
  // InBuffer, uint64_t InOffset, uint32_t InSize, ETextureFormat InFormat =
  // ETextureFormat::MAX); DescriptorDx12
  // OneFrameCreateShaderResourceView(RingBufferDx12* InBuffer, uint64_t
  // InOffset, uint32_t InStride, uint32_t InNumOfElement, ETextureFormat
  // InFormat = ETextureFormat::MAX);

  ComPtr<ID3D12DescriptorHeap> Heap;
  EDescriptorHeapTypeDX12      HeapType = EDescriptorHeapTypeDX12::CBV_SRV_UAV;
  D3D12_CPU_DESCRIPTOR_HANDLE  CPUHandleStart   = {};
  D3D12_GPU_DESCRIPTOR_HANDLE  GPUHandleStart   = {};
  uint32_t                     DescriptorSize   = 0;
  uint32_t                     NumOfDescriptors = 0;
  std::set<uint32_t>           Pools;
  mutable MutexLock            DescriptorLock;

  std::vector<PendingForFree> PendingFree;
  int32_t CanReleasePendingFreeShaderBindingInstanceFrameNumber = 0;
};

struct DescriptorBlockDx12 {
  class OnlineDescriptorHeapBlocksDx12* DescriptorHeapBlocks = nullptr;
  EDescriptorHeapTypeDX12       HeapType = EDescriptorHeapTypeDX12::CBV_SRV_UAV;
  int32_t                       Index    = 0;
  int32_t                       AllocatedSize = 0;
  std::vector<DescriptorDx12> Descriptors;
};

class OnlineDescriptorHeapDx12;

// Class that manages a single Heap by dividing it into multiple Blocks
// - The Block name is OnlineDescriptorHeapDx12
class OnlineDescriptorHeapBlocksDx12 {
  public:
  static constexpr int32_t DescriptorsInBlock        = 5000;
  static constexpr int32_t TotalHeapSize             = 500'000;
  static constexpr int32_t SamplerDescriptorsInBlock = 100;
  static constexpr int32_t SamplerTotalHeapSize      = 2000;

  static constexpr int32_t NumOfFramesToWaitBeforeReleasing = 3;

  struct FreeData {
    bool IsValid() const { return Index != -1; }

    uint32_t ReleasedFrame = 0;
    int32_t  Index         = -1;
  };

  struct FreeDataLessReleasedFrameFirstComp {
    bool operator()(const FreeData& InA, const FreeData& InB) const {
      return ((uint64_t)InA.ReleasedFrame << 32 | (uint64_t)InA.Index)
           < ((uint64_t)InB.ReleasedFrame << 32 | (uint64_t)InB.Index);
    }
  };

  void Initialize(EDescriptorHeapTypeDX12 InHeapType,
                  uint32_t                InTotalHeapSize,
                  uint32_t                InDescriptorsInBlock);
  void Release();

  OnlineDescriptorHeapDx12* Alloc();
  void                        Free(int32_t InIndex);

  ComPtr<ID3D12DescriptorHeap> Heap;
  EDescriptorHeapTypeDX12      HeapType = EDescriptorHeapTypeDX12::CBV_SRV_UAV;
  D3D12_CPU_DESCRIPTOR_HANDLE  CPUHandleStart   = {};
  D3D12_GPU_DESCRIPTOR_HANDLE  GPUHandleStart   = {};
  uint32_t                     DescriptorSize   = 0;
  uint32_t                     NumOfDescriptors = 0;
  std::set<FreeData, FreeDataLessReleasedFrameFirstComp> FreeLists;

  std::vector<OnlineDescriptorHeapDx12*> OnlineDescriptorHeap;
  std::vector<DescriptorBlockDx12>       DescriptorBlocks;

  mutable MutexLock DescriptorBlockLock;
};

// Each CommandList has its own OnlineDescriptorHeap, allocated from
// OnlineDescriptorHeapBlocksDx12.
// - By allocating a block per CommandList, we avoid multiple CommandLists
// competing for allocations from the OnlineDescriptor.
class OnlineDescriptorHeapDx12 {
  public:
  void Initialize(DescriptorBlockDx12* InDescriptorBlocks) {
    DescriptorBlocks = InDescriptorBlocks;
    if (DescriptorBlocks) {
      CPUHandle = DescriptorBlocks->Descriptors[0].CPUHandle;
      GPUHandle = DescriptorBlocks->Descriptors[0].GPUHandle;
      Heap      = DescriptorBlocks->DescriptorHeapBlocks->Heap.Get();
    }
  }

  void Release() {
    if (DescriptorBlocks) {
      assert(DescriptorBlocks->DescriptorHeapBlocks);
      DescriptorBlocks->DescriptorHeapBlocks->Free(DescriptorBlocks->Index);
    }
  }

  DescriptorDx12 Alloc() {
    if (NumOfAllocated < DescriptorBlocks->Descriptors.size()) {
      return DescriptorBlocks->Descriptors[NumOfAllocated++];
    }

    return DescriptorDx12();
  }

  void Reset() { NumOfAllocated = 0; }

  bool CanAllocate(int32_t InSize) const {
    return (DescriptorBlocks->Descriptors.size() - NumOfAllocated) >= InSize;
  }

  int32_t GetNumOfAllocated() const { return NumOfAllocated; }

  D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(size_t InIndex) const {
    return D3D12_GPU_DESCRIPTOR_HANDLE(
        GPUHandle.ptr
        + InIndex * DescriptorBlocks->DescriptorHeapBlocks->DescriptorSize);
  }

  D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return CPUHandle; }

  D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return GPUHandle; }

  ID3D12DescriptorHeap* GetHeap() const { return Heap; }

  uint32_t GetDescriptorSize() const {
    assert(DescriptorBlocks);
    assert(DescriptorBlocks->DescriptorHeapBlocks);
    return DescriptorBlocks->DescriptorHeapBlocks->DescriptorSize;
  }

  private:
  ID3D12DescriptorHeap*       Heap             = nullptr;
  DescriptorBlockDx12*      DescriptorBlocks = nullptr;
  D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle        = {};
  D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle        = {};
  int32_t                     NumOfAllocated   = 0;
};

// Manages the OnlineDescriptorHeapBlock and allocates additional
// DescriptorHeapBlocks when needed.
class OnlineDescriptorManager {
  public:
  OnlineDescriptorHeapDx12* Alloc(EDescriptorHeapTypeDX12 InType) {
    std::vector<OnlineDescriptorHeapBlocksDx12*>& DescriptorHeapBlocks
        = OnlineDescriptorHeapBlocks[(int32_t)InType];

    // Check if allocation is possible from an existing HeapBlock
    for (int32_t i = 0; i < static_cast<int32_t>(DescriptorHeapBlocks.size());
         ++i) {
      assert(DescriptorHeapBlocks[i]);
      OnlineDescriptorHeapDx12* AllocatedBlocks
          = DescriptorHeapBlocks[i]->Alloc();
      if (AllocatedBlocks) {
        return AllocatedBlocks;
      }
    }

    // If all existing HeapBlocks are full, add a new HeapBlock
    auto SelectedHeapBlocks = new OnlineDescriptorHeapBlocksDx12();

    switch (InType) {
      case EDescriptorHeapTypeDX12::CBV_SRV_UAV: {
        SelectedHeapBlocks->Initialize(
            EDescriptorHeapTypeDX12::CBV_SRV_UAV,
            OnlineDescriptorHeapBlocksDx12::TotalHeapSize,
            OnlineDescriptorHeapBlocksDx12::DescriptorsInBlock);
        break;
      }
      case EDescriptorHeapTypeDX12::SAMPLER: {
        SelectedHeapBlocks->Initialize(
            EDescriptorHeapTypeDX12::SAMPLER,
            OnlineDescriptorHeapBlocksDx12::SamplerTotalHeapSize,
            OnlineDescriptorHeapBlocksDx12::SamplerDescriptorsInBlock);
        break;
      }
      default:
        assert(0);
        break;
    }

    // Add the new HeapBlock to the list
    DescriptorHeapBlocks.push_back(SelectedHeapBlocks);

    // Sort HeapBlocks so that the ones with the most available allocations are
    // first
    std::sort(DescriptorHeapBlocks.begin(),
              DescriptorHeapBlocks.end(),
              [](OnlineDescriptorHeapBlocksDx12* InA,
                 OnlineDescriptorHeapBlocksDx12* InB) {
                return InA->FreeLists.size() > InB->FreeLists.size();
              });

    OnlineDescriptorHeapDx12* AllocatedBlocks = SelectedHeapBlocks->Alloc();
    assert(AllocatedBlocks);
    return AllocatedBlocks;
  }

  void Free(OnlineDescriptorHeapDx12* InDescriptorHeap) {
    assert(InDescriptorHeap);
    InDescriptorHeap->Release();
  }

  void Release() {
    for (int32_t i = 0; i < (int32_t)EDescriptorHeapTypeDX12::MAX; ++i) {
      for (OnlineDescriptorHeapBlocksDx12* iter :
           OnlineDescriptorHeapBlocks[i]) {
        delete iter;
      }
      OnlineDescriptorHeapBlocks[i].clear();
    }
  }

  std::vector<OnlineDescriptorHeapBlocksDx12*>
      OnlineDescriptorHeapBlocks[(int32_t)EDescriptorHeapTypeDX12::MAX];
};

class OfflineDescriptorHeapDx12 {
  public:
  void Initialize(EDescriptorHeapTypeDX12 InHeapType) {
    assert(!IsInitialized);

    HeapType      = InHeapType;
    CurrentHeap   = CreateDescriptorHeap();
    IsInitialized = true;
  }

  DescriptorDx12 Alloc() {
    if (!IsInitialized) {
      return DescriptorDx12();
    }

    if (!CurrentHeap) {
      CurrentHeap = CreateDescriptorHeap();
      assert(CurrentHeap);
    }

    DescriptorDx12 NewDescriptor = CurrentHeap->Alloc();
    if (!NewDescriptor.IsValid()) {
      if (Heap.size() > 0) {
        // Reorder the Heap to place those with more available allocations at
        // the front
        std::sort(Heap.begin(),
                  Heap.end(),
                  [](const std::shared_ptr<DescriptorHeapDx12>& InA,
                     const std::shared_ptr<DescriptorHeapDx12>& InB) {
                    return InA->Pools.size() > InB->Pools.size();
                  });

        if (Heap[0]->Pools.size() > 0) {
          CurrentHeap = Heap[0];

          NewDescriptor = CurrentHeap->Alloc();
          assert(NewDescriptor.IsValid());
          return NewDescriptor;
        }
      }

      CurrentHeap = CreateDescriptorHeap();
      assert(CurrentHeap);

      NewDescriptor = CurrentHeap->Alloc();
      assert(NewDescriptor.IsValid());
    }

    return NewDescriptor;
  }

  void Free(const DescriptorDx12& InDescriptor) {
    if (!InDescriptor.DescriptorHeap.expired()) {
      InDescriptor.DescriptorHeap.lock()->Free(InDescriptor.Index);
    }
  }

  void Release() {
    if (!IsInitialized) {
      return;
    }

    Heap.clear();
  }

  private:
  std::shared_ptr<DescriptorHeapDx12> CreateDescriptorHeap() {
    auto DescriptorHeap = std::make_shared<DescriptorHeapDx12>();
    assert(DescriptorHeap);

    DescriptorHeap->Initialize(HeapType, false);

    Heap.push_back(DescriptorHeap);
    return DescriptorHeap;
  }

  bool                    IsInitialized = false;
  EDescriptorHeapTypeDX12 HeapType      = EDescriptorHeapTypeDX12::CBV_SRV_UAV;

  std::shared_ptr<DescriptorHeapDx12>              CurrentHeap;
  std::vector<std::shared_ptr<DescriptorHeapDx12>> Heap;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_DESCRIPTOR_HEAP_DX12_H