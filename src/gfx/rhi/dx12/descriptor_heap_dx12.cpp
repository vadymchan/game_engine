#include "gfx/rhi/dx12/descriptor_heap_dx12.h"

#include "gfx/rhi/dx12/rhi_dx12.h"
#include "gfx/rhi/rhi.h"

namespace game_engine {

//////////////////////////////////////////////////////////////////////////
// DescriptorDx12
//////////////////////////////////////////////////////////////////////////
const DescriptorDx12 DescriptorDx12::s_kInvalid;

void DescriptorDx12::Free() {
  if (IsValid()) {
    if (!m_descriptorHeap_.expired()) {
      m_descriptorHeap_.lock()->Free(m_index_);
    }
    m_index_     = -1;
    m_cpuHandle_ = {};
    m_gpuHandle_ = {};
    m_descriptorHeap_.reset();
  }
}

//////////////////////////////////////////////////////////////////////////
// DescriptorHeapDx12
//////////////////////////////////////////////////////////////////////////
void DescriptorHeapDx12::Initialize(EDescriptorHeapTypeDX12 heapType,
                                    bool                    shaderVisible,
                                    uint32_t                numOfDescriptors) {
  ScopedLock s(&m_descriptorLock_);

  assert(!shaderVisible
         || (shaderVisible
             && (heapType != EDescriptorHeapTypeDX12::RTV
                 && heapType != EDescriptorHeapTypeDX12::DSV)));

  m_heapType_             = heapType;
  const auto HeapTypeDX12 = GetDX12DescriptorHeapType(heapType);

  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
  heapDesc.NumDescriptors             = numOfDescriptors;
  heapDesc.Type                       = HeapTypeDX12;
  heapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
                                 : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

  assert(g_rhi_dx12);
  assert(g_rhi_dx12->m_device_);

  HRESULT hr = g_rhi_dx12->m_device_->CreateDescriptorHeap(
      &heapDesc, IID_PPV_ARGS(&m_heap_));
  assert(SUCCEEDED(hr));

  if (FAILED(hr)) {
    return;
  }

  m_cpuHandleStart_ = m_heap_->GetCPUDescriptorHandleForHeapStart();
  if (shaderVisible) {
    m_gpuHandleStart_ = m_heap_->GetGPUDescriptorHandleForHeapStart();
  } else {
    m_gpuHandleStart_.ptr = (UINT64)-1;
  }

  m_descriptorSize_
      = g_rhi_dx12->m_device_->GetDescriptorHandleIncrementSize(HeapTypeDX12);
  m_numOfDescriptors_ = numOfDescriptors;

  for (uint32_t i = 0; i < m_numOfDescriptors_; ++i) {
    m_pools_.insert(i);
  }
}

void DescriptorHeapDx12::Release() {
  ScopedLock s(&m_descriptorLock_);

  m_heap_->Release();
  m_cpuHandleStart_   = {};
  m_gpuHandleStart_   = {};
  m_descriptorSize_   = 0;
  m_numOfDescriptors_ = 0;
  m_pools_.clear();
}

void DescriptorHeapDx12::Free(uint32_t index, uint32_t delayFrames) {
  m_pendingFree_.push_back(
      {.m_descriptorIndex_ = index,
       .m_frameIndex_      = g_rhi_dx12->GetCurrentFrameNumber()});
  ProcessPendingDescriptorPoolFree();
}

void DescriptorHeapDx12::ProcessPendingDescriptorPoolFree() {
  const int32_t CurrentFrameNumber = g_rhi->GetCurrentFrameNumber();
  const int32_t OldestFrameToKeep
      = CurrentFrameNumber - s_kNumOfFramesToWaitBeforeReleasing;

  // Check it is too early
  if (OldestFrameToKeep >= 0
      && CurrentFrameNumber
             >= m_canReleasePendingFreeShaderBindingInstanceFrameNumber_) {
    // Release pending memory
    int32_t i = 0;
    for (; i < m_pendingFree_.size(); ++i) {
      PendingForFree& PendingFreeInstance = m_pendingFree_[i];
      if ((int32_t)PendingFreeInstance.m_frameIndex_ < OldestFrameToKeep) {
        m_pools_.insert(PendingFreeInstance.m_descriptorIndex_);
      } else {
        m_canReleasePendingFreeShaderBindingInstanceFrameNumber_
            = PendingFreeInstance.m_frameIndex_
            + s_kNumOfFramesToWaitBeforeReleasing + 1;
        break;
      }
    }
    if (i > 0) {
      const size_t RemainingSize = (m_pendingFree_.size() - i);
      if (RemainingSize > 0) {
        memcpy(&m_pendingFree_[0],
               &m_pendingFree_[i],
               sizeof(PendingForFree) * RemainingSize);
      }
      m_pendingFree_.resize(RemainingSize);
    }
  }
}

// DescriptorDx12
// DescriptorHeapDx12::OneFrameCreateConstantBufferView(RingBufferDx12*
// buffer, uint64 offset, uint32_t size, ETextureFormat format)
//{
//     assert(g_rhi_dx12);
//     assert(g_rhi_dx12->Device);
//
//     DescriptorDx12 Descriptor = OneFrameAlloc();
//
//     assert(buffer);
//     assert(buffer->m_buffer);
//
//     D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = { };
//     cbvDesc.BufferLocation = buffer->GetGPUAddress() + offset;
//     cbvDesc.SizeInBytes = uint32_t(Align(size,
//     D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT));
//     g_rhi_dx12->Device->CreateConstantBufferView(&cbvDesc,
//     Descriptor.CPUHandle); return Descriptor;
// }

// DescriptorDx12
// DescriptorHeapDx12::OneFrameCreateShaderResourceView(RingBufferDx12*
// buffer, uint64 offset, uint32_t stride, uint32_t numOfElement,
// ETextureFormat format)
//{
//     assert(g_rhi_dx12);
//     assert(g_rhi_dx12->Device);
//
//     D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
//     desc.Format = (format == ETextureFormat::MAX) ? DXGI_FORMAT_UNKNOWN :
//     GetDX12TextureFormat(format); desc.ViewDimension =
//     D3D12_SRV_DIMENSION_BUFFER; desc.Shader4ComponentMapping =
//     D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; desc.m_buffer.FirstElement =
//     uint32_t(offset / stride); desc.m_buffer.Flags =
//     D3D12_BUFFER_SRV_FLAG_NONE; desc.m_buffer.NumElements = numOfElement;
//     desc.m_buffer.StructureByteStride = stride;
//
//     DescriptorDx12 Descriptor = OneFrameAlloc();
//
//     assert(buffer);
//     assert(buffer->m_buffer);
//     g_rhi_dx12->Device->CreateShaderResourceView(buffer->m_buffer.Get(),
//     &desc, Descriptor.CPUHandle); return Descriptor;
// }

//////////////////////////////////////////////////////////////////////////
// OnlineDescriptorHeapDx12
//////////////////////////////////////////////////////////////////////////
void OnlineDescriptorHeapBlocksDx12::Initialize(
    EDescriptorHeapTypeDX12 heapType,
    uint32_t                totalHeapSize,
    uint32_t                descriptorsInBlock) {
  ScopedLock s(&m_descriptorBlockLock_);

  assert((heapType == EDescriptorHeapTypeDX12::CBV_SRV_UAV)
         || (heapType == EDescriptorHeapTypeDX12::SAMPLER));

  const auto HeapTypeDX12 = GetDX12DescriptorHeapType(heapType);

  m_numOfDescriptors_ = totalHeapSize;

  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
  heapDesc.NumDescriptors             = m_numOfDescriptors_;
  heapDesc.Type                       = HeapTypeDX12;
  heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

  assert(g_rhi_dx12);
  assert(g_rhi_dx12->m_device_);

  HRESULT hr = g_rhi_dx12->m_device_->CreateDescriptorHeap(
      &heapDesc, IID_PPV_ARGS(&m_heap_));
  assert(SUCCEEDED(hr));

  if (FAILED(hr)) {
    return;
  }

  m_cpuHandleStart_ = m_heap_->GetCPUDescriptorHandleForHeapStart();
  m_gpuHandleStart_ = m_heap_->GetGPUDescriptorHandleForHeapStart();

  m_descriptorSize_
      = g_rhi_dx12->m_device_->GetDescriptorHandleIncrementSize(HeapTypeDX12);

  const uint32_t BlockSize = totalHeapSize / descriptorsInBlock;

  m_descriptorBlocks_.resize(BlockSize);
  m_onlineDescriptorHeap_.resize(BlockSize);

  int32_t Index = 0;
  for (uint32_t i = 0; i < m_numOfDescriptors_; ++i) {
    int32_t AllocatedSize = m_descriptorBlocks_[Index].m_allocatedSize_;
    if (AllocatedSize >= (int32_t)descriptorsInBlock) {
      ++Index;
      AllocatedSize = m_descriptorBlocks_[Index].m_allocatedSize_;
    }

    if (m_descriptorBlocks_[Index].m_descriptors_.size()
        != descriptorsInBlock) {
      m_descriptorBlocks_[Index].m_descriptors_.resize(descriptorsInBlock);
    }

    DescriptorDx12& Descriptor
        = m_descriptorBlocks_[Index].m_descriptors_[AllocatedSize];
    Descriptor.m_index_          = i;
    Descriptor.m_cpuHandle_      = m_cpuHandleStart_;
    Descriptor.m_cpuHandle_.ptr += Descriptor.m_index_ * m_descriptorSize_;

    Descriptor.m_gpuHandle_      = m_gpuHandleStart_;
    Descriptor.m_gpuHandle_.ptr += Descriptor.m_index_ * m_descriptorSize_;

    ++m_descriptorBlocks_[Index].m_allocatedSize_;
  }

  for (int32_t i = 0; i < m_descriptorBlocks_.size(); ++i) {
    m_descriptorBlocks_[i].m_descriptorHeapBlocks_ = this;
    m_descriptorBlocks_[i].m_index_                = i;
    m_descriptorBlocks_[i].m_heapType_             = m_heapType_;

    m_onlineDescriptorHeap_[i] = new OnlineDescriptorHeapDx12();
    m_onlineDescriptorHeap_[i]->Initialize(&m_descriptorBlocks_[i]);

    m_freeLists_.insert({.m_releasedFrame_ = 0, .m_index_ = i});
  }
}

void OnlineDescriptorHeapBlocksDx12::Release() {
  ScopedLock s(&m_descriptorBlockLock_);

  if (m_heap_) {
    m_heap_->Release();
  }
  m_cpuHandleStart_   = {};
  m_gpuHandleStart_   = {};
  m_descriptorSize_   = 0;
  m_numOfDescriptors_ = 0;
  m_descriptorBlocks_.clear();

  for (int32_t i = 0; i < m_onlineDescriptorHeap_.size(); ++i) {
    delete m_onlineDescriptorHeap_[i];
  }
  m_onlineDescriptorHeap_.clear();
}

OnlineDescriptorHeapDx12* OnlineDescriptorHeapBlocksDx12::Alloc() {
  ScopedLock s(&m_descriptorBlockLock_);

  if (m_freeLists_.empty()) {
    return nullptr;
  }

  for (auto it = m_freeLists_.begin(); m_freeLists_.end() != it; ++it) {
    FreeData freeData = *it;

    const bool CanAlloc
        = (g_rhi_dx12->m_currentFrameNumber_ - freeData.m_releasedFrame_
           > s_kNumOfFramesToWaitBeforeReleasing)
       || (0 == freeData.m_releasedFrame_);
    if (CanAlloc) {
      m_freeLists_.erase(it);
      m_onlineDescriptorHeap_[freeData.m_index_]->Reset();
      return m_onlineDescriptorHeap_[freeData.m_index_];
    }
  }

  return nullptr;
}

void OnlineDescriptorHeapBlocksDx12::Free(int32_t index) {
  ScopedLock s(&m_descriptorBlockLock_);

  FreeData freeData = {.m_releasedFrame_ = g_rhi_dx12->m_currentFrameNumber_,
                       .m_index_         = index};
  assert(!m_freeLists_.contains(freeData));
  m_freeLists_.insert(freeData);
}

}  // namespace game_engine