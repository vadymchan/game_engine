#include "gfx/rhi/dx12/descriptor_heap_dx12.h"
#include "gfx/rhi/rhi.h"
#include "gfx/rhi/dx12/rhi_dx12.h"


namespace game_engine {

//////////////////////////////////////////////////////////////////////////
// DescriptorDx12
//////////////////////////////////////////////////////////////////////////
const DescriptorDx12 DescriptorDx12::Invalid;

void DescriptorDx12::Free() {
  if (IsValid()) {
    if (!DescriptorHeap.expired()) {
      DescriptorHeap.lock()->Free(Index);
    }
    Index     = -1;
    CPUHandle = {};
    GPUHandle = {};
    DescriptorHeap.reset();
  }
}

//////////////////////////////////////////////////////////////////////////
// DescriptorHeapDx12
//////////////////////////////////////////////////////////////////////////
void DescriptorHeapDx12::Initialize(EDescriptorHeapTypeDX12 InHeapType,
                                      bool                    InShaderVisible,
                                      uint32_t InNumOfDescriptors) {
  ScopedLock s(&DescriptorLock);

  assert(!InShaderVisible
        || (InShaderVisible
            && (InHeapType != EDescriptorHeapTypeDX12::RTV
                && InHeapType != EDescriptorHeapTypeDX12::DSV)));

  HeapType                = InHeapType;
  const auto HeapTypeDX12 = GetDX12DescriptorHeapType(InHeapType);

  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
  heapDesc.NumDescriptors             = InNumOfDescriptors;
  heapDesc.Type                       = HeapTypeDX12;
  heapDesc.Flags = InShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
                                   : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

  assert(g_rhi_dx12);
  assert(g_rhi_dx12->Device);

  HRESULT hr = g_rhi_dx12->Device->CreateDescriptorHeap(&heapDesc,
                                                        IID_PPV_ARGS(&Heap));
  assert(SUCCEEDED(hr));

  if (FAILED(hr)) {
    return;
  }

  CPUHandleStart = Heap->GetCPUDescriptorHandleForHeapStart();
  if (InShaderVisible) {
    GPUHandleStart = Heap->GetGPUDescriptorHandleForHeapStart();
  } else {
    GPUHandleStart.ptr = (UINT64)-1;
  }

  DescriptorSize
      = g_rhi_dx12->Device->GetDescriptorHandleIncrementSize(HeapTypeDX12);
  NumOfDescriptors = InNumOfDescriptors;

  for (uint32_t i = 0; i < NumOfDescriptors; ++i) {
    Pools.insert(i);
  }
}

void DescriptorHeapDx12::Release() {
  ScopedLock s(&DescriptorLock);

  Heap->Release();
  CPUHandleStart   = {};
  GPUHandleStart   = {};
  DescriptorSize   = 0;
  NumOfDescriptors = 0;
  Pools.clear();
}

void DescriptorHeapDx12::Free(uint32_t InIndex, uint32_t InDelayFrames) {
  PendingFree.push_back({.DescriptorIndex = InIndex,
                         .FrameIndex = g_rhi_dx12->GetCurrentFrameNumber()});
  ProcessPendingDescriptorPoolFree();
}

void DescriptorHeapDx12::ProcessPendingDescriptorPoolFree() {
  const int32_t CurrentFrameNumber = g_rhi->GetCurrentFrameNumber();
  const int32_t OldestFrameToKeep
      = CurrentFrameNumber - NumOfFramesToWaitBeforeReleasing;

  // Check it is too early
  if (OldestFrameToKeep >= 0
      && CurrentFrameNumber
             >= CanReleasePendingFreeShaderBindingInstanceFrameNumber) {
    // Release pending memory
    int32_t i = 0;
    for (; i < PendingFree.size(); ++i) {
      PendingForFree& PendingFreeInstance = PendingFree[i];
      if ((int32_t)PendingFreeInstance.FrameIndex < OldestFrameToKeep) {
        Pools.insert(PendingFreeInstance.DescriptorIndex);
      } else {
        CanReleasePendingFreeShaderBindingInstanceFrameNumber
            = PendingFreeInstance.FrameIndex + NumOfFramesToWaitBeforeReleasing
            + 1;
        break;
      }
    }
    if (i > 0) {
      const size_t RemainingSize = (PendingFree.size() - i);
      if (RemainingSize > 0) {
        memcpy(&PendingFree[0],
               &PendingFree[i],
               sizeof(PendingForFree) * RemainingSize);
      }
      PendingFree.resize(RemainingSize);
    }
  }
}

// DescriptorDx12
// DescriptorHeapDx12::OneFrameCreateConstantBufferView(RingBufferDx12*
// InBuffer, uint64 InOffset, uint32_t InSize, ETextureFormat InFormat)
//{
//     assert(g_rhi_dx12);
//     assert(g_rhi_dx12->Device);
//
//     DescriptorDx12 Descriptor = OneFrameAlloc();
//
//     assert(InBuffer);
//     assert(InBuffer->m_buffer);
//
//     D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = { };
//     cbvDesc.BufferLocation = InBuffer->GetGPUAddress() + InOffset;
//     cbvDesc.SizeInBytes = uint32_t(Align(InSize,
//     D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT));
//     g_rhi_dx12->Device->CreateConstantBufferView(&cbvDesc,
//     Descriptor.CPUHandle); return Descriptor;
// }

// DescriptorDx12
// DescriptorHeapDx12::OneFrameCreateShaderResourceView(RingBufferDx12*
// InBuffer, uint64 InOffset, uint32_t InStride, uint32_t InNumOfElement,
// ETextureFormat InFormat)
//{
//     assert(g_rhi_dx12);
//     assert(g_rhi_dx12->Device);
//
//     D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
//     desc.Format = (InFormat == ETextureFormat::MAX) ? DXGI_FORMAT_UNKNOWN :
//     GetDX12TextureFormat(InFormat); desc.ViewDimension =
//     D3D12_SRV_DIMENSION_BUFFER; desc.Shader4ComponentMapping =
//     D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; desc.m_buffer.FirstElement =
//     uint32_t(InOffset / InStride); desc.m_buffer.Flags =
//     D3D12_BUFFER_SRV_FLAG_NONE; desc.m_buffer.NumElements = InNumOfElement;
//     desc.m_buffer.StructureByteStride = InStride;
//
//     DescriptorDx12 Descriptor = OneFrameAlloc();
//
//     assert(InBuffer);
//     assert(InBuffer->m_buffer);
//     g_rhi_dx12->Device->CreateShaderResourceView(InBuffer->m_buffer.Get(),
//     &desc, Descriptor.CPUHandle); return Descriptor;
// }

//////////////////////////////////////////////////////////////////////////
// OnlineDescriptorHeapDx12
//////////////////////////////////////////////////////////////////////////
void OnlineDescriptorHeapBlocksDx12::Initialize(
    EDescriptorHeapTypeDX12 InHeapType,
    uint32_t                  InTotalHeapSize,
    uint32_t                  InDescriptorsInBlock) {
  ScopedLock s(&DescriptorBlockLock);

  assert((InHeapType == EDescriptorHeapTypeDX12::CBV_SRV_UAV)
        || (InHeapType == EDescriptorHeapTypeDX12::SAMPLER));

  const auto HeapTypeDX12 = GetDX12DescriptorHeapType(InHeapType);

  NumOfDescriptors = InTotalHeapSize;

  D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
  heapDesc.NumDescriptors             = NumOfDescriptors;
  heapDesc.Type                       = HeapTypeDX12;
  heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

  assert(g_rhi_dx12);
  assert(g_rhi_dx12->Device);
  
  HRESULT hr = g_rhi_dx12->Device->CreateDescriptorHeap(&heapDesc,
                                                        IID_PPV_ARGS(&Heap));
  assert(SUCCEEDED(hr));

  if (FAILED(hr)) {
    return;
  }

  CPUHandleStart = Heap->GetCPUDescriptorHandleForHeapStart();
  GPUHandleStart = Heap->GetGPUDescriptorHandleForHeapStart();

  DescriptorSize
      = g_rhi_dx12->Device->GetDescriptorHandleIncrementSize(HeapTypeDX12);

  const uint32_t BlockSize = InTotalHeapSize / InDescriptorsInBlock;

  DescriptorBlocks.resize(BlockSize);
  OnlineDescriptorHeap.resize(BlockSize);

  int32_t Index = 0;
  for (uint32_t i = 0; i < NumOfDescriptors; ++i) {
    int32_t AllocatedSize = DescriptorBlocks[Index].AllocatedSize;
    if (AllocatedSize >= (int32_t)InDescriptorsInBlock) {
      ++Index;
      AllocatedSize = DescriptorBlocks[Index].AllocatedSize;
    }

    if (DescriptorBlocks[Index].Descriptors.size() != InDescriptorsInBlock) {
      DescriptorBlocks[Index].Descriptors.resize(InDescriptorsInBlock);
    }

    DescriptorDx12& Descriptor
        = DescriptorBlocks[Index].Descriptors[AllocatedSize];
    Descriptor.Index          = i;
    Descriptor.CPUHandle      = CPUHandleStart;
    Descriptor.CPUHandle.ptr += Descriptor.Index * DescriptorSize;

    Descriptor.GPUHandle      = GPUHandleStart;
    Descriptor.GPUHandle.ptr += Descriptor.Index * DescriptorSize;

    ++DescriptorBlocks[Index].AllocatedSize;
  }

  for (int32_t i = 0; i < DescriptorBlocks.size(); ++i) {
    DescriptorBlocks[i].DescriptorHeapBlocks = this;
    DescriptorBlocks[i].Index                = i;
    DescriptorBlocks[i].HeapType             = HeapType;

    OnlineDescriptorHeap[i] = new OnlineDescriptorHeapDx12();
    OnlineDescriptorHeap[i]->Initialize(&DescriptorBlocks[i]);

    FreeLists.insert({.ReleasedFrame = 0, .Index = i});
  }
}

void OnlineDescriptorHeapBlocksDx12::Release() {
  ScopedLock s(&DescriptorBlockLock);

  if (Heap) {
    Heap->Release();
  }
  CPUHandleStart   = {};
  GPUHandleStart   = {};
  DescriptorSize   = 0;
  NumOfDescriptors = 0;
  DescriptorBlocks.clear();

  for (int32_t i = 0; i < OnlineDescriptorHeap.size(); ++i) {
    delete OnlineDescriptorHeap[i];
  }
  OnlineDescriptorHeap.clear();
}

OnlineDescriptorHeapDx12* OnlineDescriptorHeapBlocksDx12::Alloc() {
  ScopedLock s(&DescriptorBlockLock);

  if (FreeLists.empty()) {
    return nullptr;
  }

  for (auto it = FreeLists.begin(); FreeLists.end() != it; ++it) {
    FreeData freeData = *it;

    const bool CanAlloc
        = (g_rhi_dx12->CurrentFrameNumber - freeData.ReleasedFrame
           > NumOfFramesToWaitBeforeReleasing)
       || (0 == freeData.ReleasedFrame);
    if (CanAlloc) {
      FreeLists.erase(it);
      OnlineDescriptorHeap[freeData.Index]->Reset();
      return OnlineDescriptorHeap[freeData.Index];
    }
  }

  return nullptr;
}

void OnlineDescriptorHeapBlocksDx12::Free(int32_t InIndex) {
  ScopedLock s(&DescriptorBlockLock);

  FreeData freeData
      = {.ReleasedFrame = g_rhi_dx12->CurrentFrameNumber, .Index = InIndex};
  assert(!FreeLists.contains(freeData));
  FreeLists.insert(freeData);
}

}  // namespace game_engine