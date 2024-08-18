#ifndef GAME_ENGINE_DESCRIPTOR_HEAP_DX12_H
#define GAME_ENGINE_DESCRIPTOR_HEAP_DX12_H

#include "gfx/rhi/dx12/rhi_type_dx12.h"
#include "gfx/rhi/lock.h"

#include <set>

namespace game_engine {

// TODO: consider moving this to a separate file
struct DescriptorDx12 {
  void Free();

  bool IsValid() const { return m_index_ != -1; }

  static const DescriptorDx12 s_kInvalid;

  D3D12_CPU_DESCRIPTOR_HANDLE             m_cpuHandle_ = {};
  D3D12_GPU_DESCRIPTOR_HANDLE             m_gpuHandle_ = {};
  uint32_t                                m_index_     = uint32_t(-1);
  std::weak_ptr<class DescriptorHeapDx12> m_descriptorHeap_;
};

class DescriptorHeapDx12
    : public std::enable_shared_from_this<DescriptorHeapDx12> {
  public:
  static constexpr int32_t s_kNumOfFramesToWaitBeforeReleasing = 3;

  struct PendingForFree {
    uint32_t m_descriptorIndex_ = UINT_MAX;
    uint32_t m_frameIndex_      = 0;
  };

  void Initialize(EDescriptorHeapTypeDX12 heapType,
                  bool                    shaderVisible,
                  uint32_t                numOfDescriptors = 1024);
  void Release();

  DescriptorDx12 Alloc() {
    ScopedLock s(&m_descriptorLock_);

    if (m_pools_.empty()) {
      return DescriptorDx12();
    }

    DescriptorDx12 Descriptor;
    Descriptor.m_index_ = *m_pools_.begin();
    m_pools_.erase(m_pools_.begin());

    Descriptor.m_cpuHandle_      = m_cpuHandleStart_;
    Descriptor.m_cpuHandle_.ptr += Descriptor.m_index_ * m_descriptorSize_;

    Descriptor.m_gpuHandle_      = m_gpuHandleStart_;
    Descriptor.m_gpuHandle_.ptr += Descriptor.m_index_ * m_descriptorSize_;

    Descriptor.m_descriptorHeap_ = shared_from_this();
    return Descriptor;
  }

  DescriptorDx12 OneFrameAlloc() {
    DescriptorDx12 NewAlloc = Alloc();
    Free(NewAlloc.m_index_, s_kNumOfFramesToWaitBeforeReleasing);
    return NewAlloc;
  }

  void Free(uint32_t index) {
    ScopedLock s(&m_descriptorLock_);

    assert(!m_pools_.contains(index));
    m_pools_.insert(index);
  }

  void Free(uint32_t index, uint32_t delayFrames);
  void ProcessPendingDescriptorPoolFree();

  // Create a Descriptor that will be used only for this frame
  // DescriptorDx12 OneFrameCreateConstantBufferView(RingBufferDx12*
  // buffer, uint64_t offset, uint32_t size, ETextureFormat format =
  // ETextureFormat::MAX); DescriptorDx12
  // OneFrameCreateShaderResourceView(RingBufferDx12* buffer, uint64_t
  // offset, uint32_t stride, uint32_t numOfElement, ETextureFormat
  // format = ETextureFormat::MAX);

  ComPtr<ID3D12DescriptorHeap> m_heap_;
  EDescriptorHeapTypeDX12 m_heapType_ = EDescriptorHeapTypeDX12::CBV_SRV_UAV;
  D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandleStart_   = {};
  D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandleStart_   = {};
  uint32_t                    m_descriptorSize_   = 0;
  uint32_t                    m_numOfDescriptors_ = 0;
  std::set<uint32_t>          m_pools_;
  mutable MutexLock           m_descriptorLock_;

  std::vector<PendingForFree> m_pendingFree_;
  int32_t m_canReleasePendingFreeShaderBindingInstanceFrameNumber_ = 0;
};

struct DescriptorBlockDx12 {
  class OnlineDescriptorHeapBlocksDx12* m_descriptorHeapBlocks_ = nullptr;
  EDescriptorHeapTypeDX12 m_heapType_ = EDescriptorHeapTypeDX12::CBV_SRV_UAV;
  int32_t                 m_index_    = 0;
  int32_t                 m_allocatedSize_ = 0;
  std::vector<DescriptorDx12> m_descriptors_;
};

class OnlineDescriptorHeapDx12;

// Class that manages a single Heap by dividing it into multiple Blocks
// - The Block name is OnlineDescriptorHeapDx12
class OnlineDescriptorHeapBlocksDx12 {
  public:
  static constexpr int32_t s_kDescriptorsInBlock        = 5000;
  static constexpr int32_t s_kTotalHeapSize             = 500'000;
  static constexpr int32_t s_kSamplerDescriptorsInBlock = 100;
  static constexpr int32_t s_kSamplerTotalHeapSize      = 2000;

  static constexpr int32_t s_kNumOfFramesToWaitBeforeReleasing = 3;

  struct FreeData {
    bool IsValid() const { return m_index_ != -1; }

    uint32_t m_releasedFrame_ = 0;
    int32_t  m_index_         = -1;
  };

  struct FreeDataLessReleasedFrameFirstComp {
    bool operator()(const FreeData& InA, const FreeData& InB) const {
      return ((uint64_t)InA.m_releasedFrame_ << 32 | (uint64_t)InA.m_index_)
           < ((uint64_t)InB.m_releasedFrame_ << 32 | (uint64_t)InB.m_index_);
    }
  };

  void Initialize(EDescriptorHeapTypeDX12 heapType,
                  uint32_t                totalHeapSize,
                  uint32_t                descriptorsInBlock);
  void Release();

  OnlineDescriptorHeapDx12* Alloc();
  void                      Free(int32_t index);

  ComPtr<ID3D12DescriptorHeap> m_heap_;
  EDescriptorHeapTypeDX12 m_heapType_ = EDescriptorHeapTypeDX12::CBV_SRV_UAV;
  D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandleStart_   = {};
  D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandleStart_   = {};
  uint32_t                    m_descriptorSize_   = 0;
  uint32_t                    m_numOfDescriptors_ = 0;
  std::set<FreeData, FreeDataLessReleasedFrameFirstComp> m_freeLists_;

  std::vector<OnlineDescriptorHeapDx12*> m_onlineDescriptorHeap_;
  std::vector<DescriptorBlockDx12>       m_descriptorBlocks_;

  mutable MutexLock m_descriptorBlockLock_;
};

// Each CommandList has its own OnlineDescriptorHeap, allocated from
// OnlineDescriptorHeapBlocksDx12.
// - By allocating a block per CommandList, we avoid multiple CommandLists
// competing for allocations from the OnlineDescriptor.
class OnlineDescriptorHeapDx12 {
  public:
  void Initialize(DescriptorBlockDx12* InDescriptorBlocks) {
    m_descriptorBlocks_ = InDescriptorBlocks;
    if (m_descriptorBlocks_) {
      m_cpuHandle_ = m_descriptorBlocks_->m_descriptors_[0].m_cpuHandle_;
      m_gpuHandle_ = m_descriptorBlocks_->m_descriptors_[0].m_gpuHandle_;
      m_heap_ = m_descriptorBlocks_->m_descriptorHeapBlocks_->m_heap_.Get();
    }
  }

  void Release() {
    if (m_descriptorBlocks_) {
      assert(m_descriptorBlocks_->m_descriptorHeapBlocks_);
      m_descriptorBlocks_->m_descriptorHeapBlocks_->Free(
          m_descriptorBlocks_->m_index_);
    }
  }

  DescriptorDx12 Alloc() {
    if (m_numOfAllocated_ < m_descriptorBlocks_->m_descriptors_.size()) {
      return m_descriptorBlocks_->m_descriptors_[m_numOfAllocated_++];
    }

    return DescriptorDx12();
  }

  void Reset() { m_numOfAllocated_ = 0; }

  bool CanAllocate(int32_t size) const {
    return (m_descriptorBlocks_->m_descriptors_.size() - m_numOfAllocated_)
        >= size;
  }

  int32_t GetNumOfAllocated() const { return m_numOfAllocated_; }

  D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(size_t index) const {
    return D3D12_GPU_DESCRIPTOR_HANDLE(
        m_gpuHandle_.ptr
        + index
              * m_descriptorBlocks_->m_descriptorHeapBlocks_
                    ->m_descriptorSize_);
  }

  D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle() const { return m_cpuHandle_; }

  D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle() const { return m_gpuHandle_; }

  ID3D12DescriptorHeap* GetHeap() const { return m_heap_; }

  uint32_t GetDescriptorSize() const {
    assert(m_descriptorBlocks_);
    assert(m_descriptorBlocks_->m_descriptorHeapBlocks_);
    return m_descriptorBlocks_->m_descriptorHeapBlocks_->m_descriptorSize_;
  }

  private:
  ID3D12DescriptorHeap*       m_heap_             = nullptr;
  DescriptorBlockDx12*        m_descriptorBlocks_ = nullptr;
  D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle_        = {};
  D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle_        = {};
  int32_t                     m_numOfAllocated_   = 0;
};

// Manages the OnlineDescriptorHeapBlock and allocates additional
// DescriptorHeapBlocks when needed.
class OnlineDescriptorManager {
  public:
  OnlineDescriptorHeapDx12* Alloc(EDescriptorHeapTypeDX12 type) {
    std::vector<OnlineDescriptorHeapBlocksDx12*>& DescriptorHeapBlocks
        = m_onlineDescriptorHeapBlocks_[(int32_t)type];

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

    switch (type) {
      case EDescriptorHeapTypeDX12::CBV_SRV_UAV: {
        SelectedHeapBlocks->Initialize(
            EDescriptorHeapTypeDX12::CBV_SRV_UAV,
            OnlineDescriptorHeapBlocksDx12::s_kTotalHeapSize,
            OnlineDescriptorHeapBlocksDx12::s_kDescriptorsInBlock);
        break;
      }
      case EDescriptorHeapTypeDX12::SAMPLER: {
        SelectedHeapBlocks->Initialize(
            EDescriptorHeapTypeDX12::SAMPLER,
            OnlineDescriptorHeapBlocksDx12::s_kSamplerTotalHeapSize,
            OnlineDescriptorHeapBlocksDx12::s_kSamplerDescriptorsInBlock);
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
                return InA->m_freeLists_.size() > InB->m_freeLists_.size();
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
           m_onlineDescriptorHeapBlocks_[i]) {
        delete iter;
      }
      m_onlineDescriptorHeapBlocks_[i].clear();
    }
  }

  std::vector<OnlineDescriptorHeapBlocksDx12*>
      m_onlineDescriptorHeapBlocks_[(int32_t)EDescriptorHeapTypeDX12::MAX];
};

class OfflineDescriptorHeapDx12 {
  public:
  void Initialize(EDescriptorHeapTypeDX12 heapType) {
    assert(!m_isInitialized_);

    m_heapType_      = heapType;
    m_currentHeap_   = CreateDescriptorHeap();
    m_isInitialized_ = true;
  }

  DescriptorDx12 Alloc() {
    if (!m_isInitialized_) {
      return DescriptorDx12();
    }

    if (!m_currentHeap_) {
      m_currentHeap_ = CreateDescriptorHeap();
      assert(m_currentHeap_);
    }

    DescriptorDx12 NewDescriptor = m_currentHeap_->Alloc();
    if (!NewDescriptor.IsValid()) {
      if (m_heap_.size() > 0) {
        // Reorder the Heap to place those with more available allocations at
        // the front
        std::sort(m_heap_.begin(),
                  m_heap_.end(),
                  [](const std::shared_ptr<DescriptorHeapDx12>& InA,
                     const std::shared_ptr<DescriptorHeapDx12>& InB) {
                    return InA->m_pools_.size() > InB->m_pools_.size();
                  });

        if (m_heap_[0]->m_pools_.size() > 0) {
          m_currentHeap_ = m_heap_[0];

          NewDescriptor = m_currentHeap_->Alloc();
          assert(NewDescriptor.IsValid());
          return NewDescriptor;
        }
      }

      m_currentHeap_ = CreateDescriptorHeap();
      assert(m_currentHeap_);

      NewDescriptor = m_currentHeap_->Alloc();
      assert(NewDescriptor.IsValid());
    }

    return NewDescriptor;
  }

  void Free(const DescriptorDx12& InDescriptor) {
    if (!InDescriptor.m_descriptorHeap_.expired()) {
      InDescriptor.m_descriptorHeap_.lock()->Free(InDescriptor.m_index_);
    }
  }

  void Release() {
    if (!m_isInitialized_) {
      return;
    }

    m_heap_.clear();
  }

  private:
  std::shared_ptr<DescriptorHeapDx12> CreateDescriptorHeap() {
    auto DescriptorHeap = std::make_shared<DescriptorHeapDx12>();
    assert(DescriptorHeap);

    DescriptorHeap->Initialize(m_heapType_, false);

    m_heap_.push_back(DescriptorHeap);
    return DescriptorHeap;
  }

  bool                    m_isInitialized_ = false;
  EDescriptorHeapTypeDX12 m_heapType_ = EDescriptorHeapTypeDX12::CBV_SRV_UAV;

  std::shared_ptr<DescriptorHeapDx12>              m_currentHeap_;
  std::vector<std::shared_ptr<DescriptorHeapDx12>> m_heap_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_DESCRIPTOR_HEAP_DX12_H