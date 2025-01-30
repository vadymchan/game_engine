#ifndef GAME_ENGINE_DESCRIPTOR_HEAP_DX12_H
#define GAME_ENGINE_DESCRIPTOR_HEAP_DX12_H

#include "platform/windows/windows_platform_setup.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/dx12/rhi_type_dx12.h"
#include "gfx/rhi/lock.h"

#include <set>

namespace game_engine {

// TODO: consider moving this to a separate file

// describes one descriptor (CPU / GPU handle, index and reference to heap where
// it's located) in a descriptor heap
struct DescriptorDx12 {
  // ======= BEGIN: public static fields ======================================

  static const DescriptorDx12 s_kInvalid;

  // ======= END: public static fields   ======================================

  // ======= BEGIN: public misc methods =======================================

  void free();

  bool isValid() const { return m_index_ != -1; }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  D3D12_CPU_DESCRIPTOR_HANDLE             m_cpuHandle_ = {};
  D3D12_GPU_DESCRIPTOR_HANDLE             m_gpuHandle_ = {};
  uint32_t                                m_index_     = uint32_t(-1);
  std::weak_ptr<class DescriptorHeapDx12> m_descriptorHeap_;

  // ======= END: public misc fields   ========================================
};

// mostly for OfflineDescriptorHeapDx12, manages an entire descriptor heap
class DescriptorHeapDx12
    : public std::enable_shared_from_this<DescriptorHeapDx12> {
  public:
  // ======= BEGIN: public nested types =======================================

  struct PendingForFree {
    uint32_t m_descriptorIndex_ = UINT_MAX;
    uint32_t m_frameIndex_      = 0;
  };

  // ======= END: public nested types   =======================================

  // ======= BEGIN: public static fields ======================================

  static constexpr int32_t s_kNumOfFramesToWaitBeforeReleasing = 3;

  // ======= END: public static fields   ======================================

  // ======= BEGIN: public destructor =========================================

  ~DescriptorHeapDx12() { release(); }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public misc methods =======================================

  void initialize(EDescriptorHeapTypeDX12 heapType,
                  bool                    shaderVisible,
                  uint32_t                numOfDescriptors = 1024);
  void release();

  DescriptorDx12 alloc() {
    ScopedLock s(&m_descriptorLock_);

    if (m_pools_.empty()) {
      return DescriptorDx12();
    }

    DescriptorDx12 descriptor;
    descriptor.m_index_ = *m_pools_.begin();
    m_pools_.erase(m_pools_.begin());

    descriptor.m_cpuHandle_      = m_cpuHandleStart_;
    descriptor.m_cpuHandle_.ptr += descriptor.m_index_ * m_descriptorSize_;

    descriptor.m_gpuHandle_      = m_gpuHandleStart_;
    descriptor.m_gpuHandle_.ptr += descriptor.m_index_ * m_descriptorSize_;

    descriptor.m_descriptorHeap_ = shared_from_this();
    return descriptor;
  }

  DescriptorDx12 oneFrameAlloc() {
    DescriptorDx12 newAlloc = alloc();
    free(newAlloc.m_index_, s_kNumOfFramesToWaitBeforeReleasing);
    return newAlloc;
  }

  void free(uint32_t index) {
    ScopedLock s(&m_descriptorLock_);

    assert(!m_pools_.contains(index));
    m_pools_.insert(index);
  }

  void free(uint32_t index, uint32_t delayFrames);
  void processPendingDescriptorPoolFree();

  // Create a descriptor that will be used only for this frame
  // DescriptorDx12 OneFrameCreateConstantBufferView(RingBufferDx12*
  // buffer, uint64_t offset, uint32_t size, ETextureFormat format =
  // ETextureFormat::MAX); DescriptorDx12
  // OneFrameCreateShaderResourceView(RingBufferDx12* buffer, uint64_t
  // offset, uint32_t stride, uint32_t numOfElement, ETextureFormat
  // format = ETextureFormat::MAX);

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

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

  // ======= END: public misc fields   ========================================
};

// Stores info about a block of descriptors in a descriptor heap (their type,
// start index, size, etc.) and a reference to the heap itself. Essentially a
// data (array of descriptors)
struct DescriptorBlockDx12 {
  // ======= BEGIN: public misc fields ========================================

  class OnlineDescriptorHeapBlocksDx12* m_descriptorHeapBlocks_ = nullptr;
  EDescriptorHeapTypeDX12 m_heapType_ = EDescriptorHeapTypeDX12::CBV_SRV_UAV;
  int32_t                 m_index_    = 0;
  int32_t                 m_allocatedSize_ = 0;
  std::vector<DescriptorDx12> m_descriptors_;

  // ======= END: public misc fields   ========================================
};

class OnlineDescriptorHeapDx12;

// Class that manages a single Heap by dividing it into multiple Blocks
// - The Block name is OnlineDescriptorHeapDx12
class OnlineDescriptorHeapBlocksDx12 {
  public:
  // ======= BEGIN: public nested types =======================================

  struct FreeData {
    bool isValid() const { return m_index_ != -1; }

    uint32_t m_releasedFrame_ = 0;
    int32_t  m_index_         = -1;
  };

  struct FreeDataLessReleasedFrameFirstComp {
    bool operator()(const FreeData& a, const FreeData& b) const {
      return ((uint64_t)a.m_releasedFrame_ << 32 | (uint64_t)a.m_index_)
           < ((uint64_t)b.m_releasedFrame_ << 32 | (uint64_t)b.m_index_);
    }
  };

  // ======= END: public nested types   =======================================

  // ======= BEGIN: public static fields ======================================

  static constexpr int32_t s_kDescriptorsInBlock        = 5000;
  static constexpr int32_t s_kTotalHeapSize             = 500'000;
  static constexpr int32_t s_kSamplerDescriptorsInBlock = 100;
  static constexpr int32_t s_kSamplerTotalHeapSize      = 2000;

  static constexpr int32_t s_kNumOfFramesToWaitBeforeReleasing = 3;

  // ======= END: public static fields   ======================================

  // ======= BEGIN: public misc methods =======================================

  void initialize(EDescriptorHeapTypeDX12 heapType,
                  uint32_t                totalHeapSize,
                  uint32_t                descriptorsInBlock);
  void release();

  OnlineDescriptorHeapDx12* alloc();
  void                      free(int32_t index);

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

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

  // ======= END: public misc fields   ========================================
};

// it's a class that manages a block of descriptors in a descriptor heap (gives
// usefull methods to allocate and free descriptors)
// Each CommandList has its own OnlineDescriptorHeap, allocated from
// OnlineDescriptorHeapBlocksDx12.
// - By allocating a block per CommandList, we avoid multiple CommandLists
// competing for allocations from the OnlineDescriptor.
class OnlineDescriptorHeapDx12 {
  public:
  // ======= BEGIN: public getters ============================================

  int32_t getNumOfAllocated() const { return m_numOfAllocated_; }

  D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle(size_t index) const {
    return D3D12_GPU_DESCRIPTOR_HANDLE(
        m_gpuHandle_.ptr
        + index
              * m_descriptorBlocks_->m_descriptorHeapBlocks_
                    ->m_descriptorSize_);
  }

  // TODO: not used
  D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandle() const { return m_cpuHandle_; }

  // TODO: not used
  D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle() const { return m_gpuHandle_; }

  ID3D12DescriptorHeap* getHeap() const { return m_heap_; }

  // TODO: not used
  uint32_t getDescriptorSize() const {
    assert(m_descriptorBlocks_);
    assert(m_descriptorBlocks_->m_descriptorHeapBlocks_);
    return m_descriptorBlocks_->m_descriptorHeapBlocks_->m_descriptorSize_;
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  void initialize(DescriptorBlockDx12* descriptorBlocks) {
    m_descriptorBlocks_ = descriptorBlocks;
    if (m_descriptorBlocks_) {
      m_cpuHandle_ = m_descriptorBlocks_->m_descriptors_[0].m_cpuHandle_;
      m_gpuHandle_ = m_descriptorBlocks_->m_descriptors_[0].m_gpuHandle_;
      m_heap_ = m_descriptorBlocks_->m_descriptorHeapBlocks_->m_heap_.Get();
    }
  }

  void release() {
    if (m_descriptorBlocks_) {
      assert(m_descriptorBlocks_->m_descriptorHeapBlocks_);
      m_descriptorBlocks_->m_descriptorHeapBlocks_->free(
          m_descriptorBlocks_->m_index_);
    }
  }

  DescriptorDx12 alloc() {
    if (m_numOfAllocated_ < m_descriptorBlocks_->m_descriptors_.size()) {
      return m_descriptorBlocks_->m_descriptors_[m_numOfAllocated_++];
    }

    return DescriptorDx12();
  }

  void reset() { m_numOfAllocated_ = 0; }

  bool canAllocate(int32_t size) const {
    return (m_descriptorBlocks_->m_descriptors_.size() - m_numOfAllocated_)
        >= size;
  }

  // ======= END: public misc methods   =======================================

  private:
  // ======= BEGIN: private misc fields =======================================

  ID3D12DescriptorHeap*       m_heap_             = nullptr;
  DescriptorBlockDx12*        m_descriptorBlocks_ = nullptr;
  D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle_        = {};
  D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle_        = {};
  int32_t                     m_numOfAllocated_   = 0;

  // ======= END: private misc fields   =======================================
};

// Manages the OnlineDescriptorHeapBlock and allocates additional
// DescriptorHeapBlocks when needed.
class OnlineDescriptorManager {
  public:
  // ======= BEGIN: public misc methods =======================================

  OnlineDescriptorHeapDx12* alloc(EDescriptorHeapTypeDX12 type) {
    std::vector<OnlineDescriptorHeapBlocksDx12*>& descriptorHeapBlocks
        = m_onlineDescriptorHeapBlocks_[(int32_t)type];

    // Check if allocation is possible from an existing HeapBlock
    for (int32_t i = 0; i < static_cast<int32_t>(descriptorHeapBlocks.size());
         ++i) {
      assert(descriptorHeapBlocks[i]);
      OnlineDescriptorHeapDx12* allocatedBlocks
          = descriptorHeapBlocks[i]->alloc();
      if (allocatedBlocks) {
        return allocatedBlocks;
      }
    }

    // If all existing HeapBlocks are full, add a new HeapBlock
    auto selectedHeapBlocks = new OnlineDescriptorHeapBlocksDx12();

    switch (type) {
      case EDescriptorHeapTypeDX12::CBV_SRV_UAV: {
        selectedHeapBlocks->initialize(
            EDescriptorHeapTypeDX12::CBV_SRV_UAV,
            OnlineDescriptorHeapBlocksDx12::s_kTotalHeapSize,
            OnlineDescriptorHeapBlocksDx12::s_kDescriptorsInBlock);
        break;
      }
      case EDescriptorHeapTypeDX12::SAMPLER: {
        selectedHeapBlocks->initialize(
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
    descriptorHeapBlocks.push_back(selectedHeapBlocks);

    // Sort HeapBlocks so that the ones with the most available allocations are
    // first
    std::sort(descriptorHeapBlocks.begin(),
              descriptorHeapBlocks.end(),
              [](OnlineDescriptorHeapBlocksDx12* a,
                 OnlineDescriptorHeapBlocksDx12* b) {
                return a->m_freeLists_.size() > b->m_freeLists_.size();
              });

    OnlineDescriptorHeapDx12* allocatedBlocks = selectedHeapBlocks->alloc();
    assert(allocatedBlocks);
    return allocatedBlocks;
  }

  void free(OnlineDescriptorHeapDx12* descriptorHeap) {
    assert(descriptorHeap);
    descriptorHeap->release();
  }

  void release() {
    for (int32_t i = 0; i < (int32_t)EDescriptorHeapTypeDX12::MAX; ++i) {
      for (OnlineDescriptorHeapBlocksDx12* iter :
           m_onlineDescriptorHeapBlocks_[i]) {
        delete iter;
      }
      m_onlineDescriptorHeapBlocks_[i].clear();
    }
  }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  std::vector<OnlineDescriptorHeapBlocksDx12*>
      m_onlineDescriptorHeapBlocks_[(int32_t)EDescriptorHeapTypeDX12::MAX];

  // ======= END: public misc fields   ========================================
};

// CPU only descriptor heap (not shader visible)
class OfflineDescriptorHeapDx12 {
  public:
  std::shared_ptr<DescriptorHeapDx12> getCurrentHeap() {
    return m_currentHeap_;
  }

  // ======= BEGIN: public misc methods =======================================

  void initialize(EDescriptorHeapTypeDX12 heapType) {
    assert(!m_isInitialized_);

    m_heapType_      = heapType;
    m_currentHeap_   = createDescriptorHeap_();
    m_isInitialized_ = true;
  }

  DescriptorDx12 alloc() {
    if (!m_isInitialized_) {
      return DescriptorDx12();
    }

    if (!m_currentHeap_) {
      m_currentHeap_ = createDescriptorHeap_();
      assert(m_currentHeap_);
    }

    DescriptorDx12 newDescriptor = m_currentHeap_->alloc();
    if (!newDescriptor.isValid()) {
      if (m_heap_.size() > 0) {
        // Reorder the Heap to place those with more available allocations at
        // the front
        std::sort(m_heap_.begin(),
                  m_heap_.end(),
                  [](const std::shared_ptr<DescriptorHeapDx12>& a,
                     const std::shared_ptr<DescriptorHeapDx12>& b) {
                    return a->m_pools_.size() > b->m_pools_.size();
                  });

        if (m_heap_[0]->m_pools_.size() > 0) {
          m_currentHeap_ = m_heap_[0];

          newDescriptor = m_currentHeap_->alloc();
          assert(newDescriptor.isValid());
          return newDescriptor;
        }
      }

      m_currentHeap_ = createDescriptorHeap_();
      assert(m_currentHeap_);

      newDescriptor = m_currentHeap_->alloc();
      assert(newDescriptor.isValid());
    }

    return newDescriptor;
  }

  void free(const DescriptorDx12& descriptor) {
    if (!descriptor.m_descriptorHeap_.expired()) {
      descriptor.m_descriptorHeap_.lock()->free(descriptor.m_index_);
    }
  }

  void release() {
    if (!m_isInitialized_) {
      return;
    }

    m_heap_.clear();
  }

  // ======= END: public misc methods   =======================================

  private:
  // ======= BEGIN: private misc methods ======================================

  std::shared_ptr<DescriptorHeapDx12> createDescriptorHeap_() {
    auto descriptorHeap = std::make_shared<DescriptorHeapDx12>();
    assert(descriptorHeap);

    // Offline Descriptor Heap is not shader visible
    descriptorHeap->initialize(m_heapType_, false);

    m_heap_.push_back(descriptorHeap);
    return descriptorHeap;
  }

  // ======= END: private misc methods   ======================================

  // ======= BEGIN: private misc fields =======================================

  bool                    m_isInitialized_ = false;
  EDescriptorHeapTypeDX12 m_heapType_ = EDescriptorHeapTypeDX12::CBV_SRV_UAV;

  std::shared_ptr<DescriptorHeapDx12>              m_currentHeap_;
  std::vector<std::shared_ptr<DescriptorHeapDx12>> m_heap_;

  // ======= END: private misc fields   =======================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12

#endif  // GAME_ENGINE_DESCRIPTOR_HEAP_DX12_H
