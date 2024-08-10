#ifndef GAME_ENGINE_DESCRIPTOR_HEAP_DX12_H
#define GAME_ENGINE_DESCRIPTOR_HEAP_DX12_H

#include "gfx/rhi/dx12/rhi_type_dx12.h"
#include "gfx/rhi/lock.h"

#include <set>

namespace game_engine {

// TODO: consider moving this to a separate file
struct jDescriptor_DX12 {
  static const jDescriptor_DX12 Invalid;

  D3D12_CPU_DESCRIPTOR_HANDLE               CPUHandle = {};
  D3D12_GPU_DESCRIPTOR_HANDLE               GPUHandle = {};
  uint32_t                                    Index     = uint32_t(-1);
  std::weak_ptr<class jDescriptorHeap_DX12> DescriptorHeap;

  void Free();

  bool IsValid() const { return Index != -1; }
};

class jDescriptorHeap_DX12
    : public std::enable_shared_from_this<jDescriptorHeap_DX12> {
  public:
  static constexpr int32_t NumOfFramesToWaitBeforeReleasing = 3;

  struct PendingForFree {
    uint32_t DescriptorIndex = UINT_MAX;
    uint32_t FrameIndex      = 0;
  };

  void Initialize(EDescriptorHeapTypeDX12 InHeapType,
                  bool                    InShaderVisible,
                  uint32_t                  InNumOfDescriptors = 1024);
  void Release();

  jDescriptor_DX12 Alloc() {
    ScopedLock s(&DescriptorLock);

    if (Pools.empty()) {
      return jDescriptor_DX12();
    }

    jDescriptor_DX12 Descriptor;
    Descriptor.Index = *Pools.begin();
    Pools.erase(Pools.begin());

    Descriptor.CPUHandle      = CPUHandleStart;
    Descriptor.CPUHandle.ptr += Descriptor.Index * DescriptorSize;

    Descriptor.GPUHandle      = GPUHandleStart;
    Descriptor.GPUHandle.ptr += Descriptor.Index * DescriptorSize;

    Descriptor.DescriptorHeap = shared_from_this();
    return Descriptor;
  }

  jDescriptor_DX12 OneFrameAlloc() {
    jDescriptor_DX12 NewAlloc = Alloc();
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

  // 이번 프레임에만 사용할 Descriptor 생성
  // jDescriptor_DX12 OneFrameCreateConstantBufferView(jRingBuffer_DX12*
  // InBuffer, uint64_t InOffset, uint32_t InSize, ETextureFormat InFormat =
  // ETextureFormat::MAX); jDescriptor_DX12
  // OneFrameCreateShaderResourceView(jRingBuffer_DX12* InBuffer, uint64_t
  // InOffset, uint32_t InStride, uint32_t InNumOfElement, ETextureFormat InFormat =
  // ETextureFormat::MAX);

  ComPtr<ID3D12DescriptorHeap> Heap;
  EDescriptorHeapTypeDX12      HeapType = EDescriptorHeapTypeDX12::CBV_SRV_UAV;
  D3D12_CPU_DESCRIPTOR_HANDLE  CPUHandleStart   = {};
  D3D12_GPU_DESCRIPTOR_HANDLE  GPUHandleStart   = {};
  uint32_t                       DescriptorSize   = 0;
  uint32_t                       NumOfDescriptors = 0;
  std::set<uint32_t>             Pools;
  mutable MutexLock           DescriptorLock;

  std::vector<PendingForFree> PendingFree;
  int32_t CanReleasePendingFreeShaderBindingInstanceFrameNumber = 0;
};

struct jDescriptorBlock_DX12 {
  class jOnlineDescriptorHeapBlocks_DX12* DescriptorHeapBlocks = nullptr;
  EDescriptorHeapTypeDX12       HeapType = EDescriptorHeapTypeDX12::CBV_SRV_UAV;
  int32_t                         Index    = 0;
  int32_t                         AllocatedSize = 0;
  std::vector<jDescriptor_DX12> Descriptors;
};

class jOnlineDescriptorHeap_DX12;

// 한개의 Heap 을 여러개의 Block 으로 쪼개서 관리하는 클래스
// - Block 이름은 jOnlineDescriptorHeap_DX12 임.
class jOnlineDescriptorHeapBlocks_DX12 {
  public:
  static constexpr int32_t DescriptorsInBlock        = 5000;
  static constexpr int32_t TotalHeapSize             = 500'000;
  static constexpr int32_t SamplerDescriptorsInBlock = 100;
  static constexpr int32_t SamplerTotalHeapSize      = 2000;

  static constexpr int32_t NumOfFramesToWaitBeforeReleasing = 3;

  struct jFreeData {
    bool IsValid() const { return Index != -1; }

    uint32_t ReleasedFrame = 0;
    int32_t  Index         = -1;
  };

  struct jFreeDataLessReleasedFrameFirstComp {
    bool operator()(const jFreeData& InA, const jFreeData& InB) const {
      return ((uint64_t)InA.ReleasedFrame << 32 | (uint64_t)InA.Index)
           < ((uint64_t)InB.ReleasedFrame << 32 | (uint64_t)InB.Index);
    }
  };

  void Initialize(EDescriptorHeapTypeDX12 InHeapType,
                  uint32_t                  InTotalHeapSize,
                  uint32_t                  InDescriptorsInBlock);
  void Release();

  jOnlineDescriptorHeap_DX12* Alloc();
  void                        Free(int32_t InIndex);

  ComPtr<ID3D12DescriptorHeap> Heap;
  EDescriptorHeapTypeDX12      HeapType = EDescriptorHeapTypeDX12::CBV_SRV_UAV;
  D3D12_CPU_DESCRIPTOR_HANDLE  CPUHandleStart   = {};
  D3D12_GPU_DESCRIPTOR_HANDLE  GPUHandleStart   = {};
  uint32_t                       DescriptorSize   = 0;
  uint32_t                       NumOfDescriptors = 0;
  std::set<jFreeData, jFreeDataLessReleasedFrameFirstComp> FreeLists;

  std::vector<jOnlineDescriptorHeap_DX12*> OnlineDescriptorHeap;
  std::vector<jDescriptorBlock_DX12>       DescriptorBlocks;

  mutable MutexLock DescriptorBlockLock;
};

// CommandList 당 한개씩 가지게 되는 OnlineDescriptorHeap,
// jOnlineDescriptorHeapBlocks_DX12 으로 부터 할당 받음.
// - CommandList 당 Block 을 할당하는 방식으로 여러개의 CommandList 가
// OnlineDescriptor 에서 Allocation 경쟁 하는 부분을 피함.
class jOnlineDescriptorHeap_DX12 {
  public:
  void Initialize(jDescriptorBlock_DX12* InDescriptorBlocks) {
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

  jDescriptor_DX12 Alloc() {
    if (NumOfAllocated < DescriptorBlocks->Descriptors.size()) {
      return DescriptorBlocks->Descriptors[NumOfAllocated++];
    }

    return jDescriptor_DX12();
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
  jDescriptorBlock_DX12*      DescriptorBlocks = nullptr;
  D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle        = {};
  D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle        = {};
  int32_t                       NumOfAllocated   = 0;
};

// OnlineDescriptorHeapBlock 을 관리하는 객체, 필요한 경우 추가
// DescriptorHeapBlock 을 할당함.
class jOnlineDescriptorManager {
  public:
  jOnlineDescriptorHeap_DX12* Alloc(EDescriptorHeapTypeDX12 InType) {
    std::vector<jOnlineDescriptorHeapBlocks_DX12*>& DescriptorHeapBlocks
        = OnlineDescriptorHeapBlocks[(int32_t)InType];

    // 기존 HeapBlock 에서 할당 가능한지 확인
    for (int32_t i = 0; i < (int32_t)DescriptorHeapBlocks.size(); ++i) {
      assert(DescriptorHeapBlocks[i]);
      jOnlineDescriptorHeap_DX12* AllocatedBlocks
          = DescriptorHeapBlocks[i]->Alloc();
      if (AllocatedBlocks) {
        return AllocatedBlocks;
      }
    }

    // 기존 HeapBlock 이 가득 찬 상태라 HeapBlock 추가
    auto SelectedHeapBlocks = new jOnlineDescriptorHeapBlocks_DX12();

    switch (InType) {
      case EDescriptorHeapTypeDX12::CBV_SRV_UAV: {
        SelectedHeapBlocks->Initialize(
            EDescriptorHeapTypeDX12::CBV_SRV_UAV,
            jOnlineDescriptorHeapBlocks_DX12::TotalHeapSize,
            jOnlineDescriptorHeapBlocks_DX12::DescriptorsInBlock);
        break;
      }
      case EDescriptorHeapTypeDX12::SAMPLER: {
        SelectedHeapBlocks->Initialize(
            EDescriptorHeapTypeDX12::SAMPLER,
            jOnlineDescriptorHeapBlocks_DX12::SamplerTotalHeapSize,
            jOnlineDescriptorHeapBlocks_DX12::SamplerDescriptorsInBlock);
        break;
      }
      default:
        assert(0);
        break;
    }

    DescriptorHeapBlocks.push_back(SelectedHeapBlocks);

    // 할당 할 수 있는 HeapBlock 이 더 많은 것을 앞쪽에 배치함
    std::sort(DescriptorHeapBlocks.begin(),
              DescriptorHeapBlocks.end(),
              [](jOnlineDescriptorHeapBlocks_DX12* InA,
                 jOnlineDescriptorHeapBlocks_DX12* InB) {
                return InA->FreeLists.size() > InB->FreeLists.size();
              });

    jOnlineDescriptorHeap_DX12* AllocatedBlocks = SelectedHeapBlocks->Alloc();
    assert(AllocatedBlocks);
    return AllocatedBlocks;
  }

  void Free(jOnlineDescriptorHeap_DX12* InDescriptorHeap) {
    assert(InDescriptorHeap);
    InDescriptorHeap->Release();
  }

  void Release() {
    for (int32_t i = 0; i < (int32_t)EDescriptorHeapTypeDX12::MAX; ++i) {
      for (jOnlineDescriptorHeapBlocks_DX12* iter :
           OnlineDescriptorHeapBlocks[i]) {
        delete iter;
      }
      OnlineDescriptorHeapBlocks[i].clear();
    }
  }

  std::vector<jOnlineDescriptorHeapBlocks_DX12*>
      OnlineDescriptorHeapBlocks[(int32_t)EDescriptorHeapTypeDX12::MAX];
};

class jOfflineDescriptorHeap_DX12 {
  public:
  void Initialize(EDescriptorHeapTypeDX12 InHeapType) {
    assert(!IsInitialized);

    HeapType      = InHeapType;
    CurrentHeap   = CreateDescriptorHeap();
    IsInitialized = true;
  }

  jDescriptor_DX12 Alloc() {
    if (!IsInitialized) {
      return jDescriptor_DX12();
    }

    if (!CurrentHeap) {
      CurrentHeap = CreateDescriptorHeap();
      assert(CurrentHeap);
    }

    jDescriptor_DX12 NewDescriptor = CurrentHeap->Alloc();
    if (!NewDescriptor.IsValid()) {
      if (Heap.size() > 0) {
        // 할당 할 수 있는 Heap 이 더 많은 것을 앞쪽에 배치함
        std::sort(Heap.begin(),
                  Heap.end(),
                  [](const std::shared_ptr<jDescriptorHeap_DX12>& InA,
                     const std::shared_ptr<jDescriptorHeap_DX12>& InB) {
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

  void Free(const jDescriptor_DX12& InDescriptor) {
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
  std::shared_ptr<jDescriptorHeap_DX12> CreateDescriptorHeap() {
    auto DescriptorHeap = std::make_shared<jDescriptorHeap_DX12>();
    assert(DescriptorHeap);

    DescriptorHeap->Initialize(HeapType, false);

    Heap.push_back(DescriptorHeap);
    return DescriptorHeap;
  }

  bool                    IsInitialized = false;
  EDescriptorHeapTypeDX12 HeapType      = EDescriptorHeapTypeDX12::CBV_SRV_UAV;

  std::shared_ptr<jDescriptorHeap_DX12>              CurrentHeap;
  std::vector<std::shared_ptr<jDescriptorHeap_DX12>> Heap;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_DESCRIPTOR_HEAP_DX12_H