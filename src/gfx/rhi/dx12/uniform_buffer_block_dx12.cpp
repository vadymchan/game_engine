#include "gfx/rhi/dx12/uniform_buffer_block_dx12.h"

#include "gfx/rhi/dx12/rhi_dx12.h"
#include "gfx/rhi/dx12/utils_dx12.h"
#include "utils/memory/align.h"

namespace game_engine {

UniformBufferBlockDx12::~UniformBufferBlockDx12() {
  Release();
}

void UniformBufferBlockDx12::Init(size_t size) {
  assert(size);

  size = Align<uint64_t>(size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

  if (LifeTimeType::MultiFrame == LifeType) {
    BufferPtr = CreateBuffer(size,
                             D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT,
                             EBufferCreateFlag::CPUAccess,
                             EResourceLayout::GENERAL,
                             nullptr,
                             0,
                             TEXT(L"UniformBufferBlock"));
    CreateConstantBufferView(BufferPtr.get());
  }
}

void UniformBufferBlockDx12::Release() {
}

void UniformBufferBlockDx12::UpdateBufferData(const void* InData,
                                                size_t      InSize) {
  if (LifeTimeType::MultiFrame == LifeType) {
    assert(BufferPtr);
    assert(BufferPtr->GetAllocatedSize() >= InSize);

    bool isMapped = BufferPtr->Map();
    assert(isMapped);

    if (isMapped) {
      uint8_t* startAddr = ((uint8_t*)BufferPtr->GetMappedPointer());
      if (InData) {
        memcpy(startAddr, InData, InSize);
      } else {
        memset(startAddr, 0, InSize);
      }
      BufferPtr->Unmap();
    }
  } else {
    RingBuffer = g_rhi_dx12->GetOneFrameUniformRingBuffer();
    RingBufferAllocatedSize
        = Align(InSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
    RingBufferOffset = RingBuffer->Alloc(RingBufferAllocatedSize);
    RingBufferDestAddress
        = ((uint8_t*)RingBuffer->GetMappedPointer()) + RingBufferOffset;
    if (InData) {
      memcpy(RingBufferDestAddress, InData, InSize);
    } else {
      memset(RingBufferDestAddress, 0, InSize);
    }
  }
}

void UniformBufferBlockDx12::ClearBuffer(int32_t clearValue) {
  assert(BufferPtr);
  UpdateBufferData(nullptr, BufferPtr->GetAllocatedSize());
}

void* UniformBufferBlockDx12::GetLowLevelResource() const {
  return (LifeTimeType::MultiFrame == LifeType) ? BufferPtr->GetHandle()
                                                : RingBuffer->GetHandle();
}

void* UniformBufferBlockDx12::GetLowLevelMemory() const {
  return (LifeTimeType::MultiFrame == LifeType) ? BufferPtr->CPUAddress
                                                : RingBufferDestAddress;
}

size_t UniformBufferBlockDx12::GetBufferSize() const {
  return (LifeTimeType::MultiFrame == LifeType) ? BufferPtr->Size
                                                : RingBufferAllocatedSize;
}

const DescriptorDx12& UniformBufferBlockDx12::GetCBV() const {
  if (LifeTimeType::MultiFrame == LifeType) {
    return BufferPtr->CBV;
  }

  return RingBuffer->CBV;
}

uint64_t UniformBufferBlockDx12::GetGPUAddress() const {
  return (LifeTimeType::MultiFrame == LifeType)
           ? BufferPtr->GetGPUAddress()
           : (RingBuffer->GetGPUAddress() + RingBufferOffset);
}

}  // namespace game_engine