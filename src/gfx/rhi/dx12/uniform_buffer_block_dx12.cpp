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

  if (LifeTimeType::MultiFrame == m_LifeType_) {
    m_bufferPtr_ = CreateBuffer(size,
                             D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT,
                             EBufferCreateFlag::CPUAccess,
                             EResourceLayout::GENERAL,
                             nullptr,
                             0,
                             TEXT(L"UniformBufferBlock"));
    CreateConstantBufferView(m_bufferPtr_.get());
  }
}

void UniformBufferBlockDx12::Release() {
}

void UniformBufferBlockDx12::UpdateBufferData(const void* data,
                                                size_t      size) {
  if (LifeTimeType::MultiFrame == m_LifeType_) {
    assert(m_bufferPtr_);
    assert(m_bufferPtr_->GetAllocatedSize() >= size);

    bool isMapped = m_bufferPtr_->Map();
    assert(isMapped);

    if (isMapped) {
      uint8_t* startAddr = ((uint8_t*)m_bufferPtr_->GetMappedPointer());
      if (data) {
        memcpy(startAddr, data, size);
      } else {
        memset(startAddr, 0, size);
      }
      m_bufferPtr_->Unmap();
    }
  } else {
    m_ringBuffer_ = g_rhi_dx12->GetOneFrameUniformRingBuffer();
    m_ringBufferAllocatedSize_
        = Align(size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
    m_ringBufferOffset_ = m_ringBuffer_->Alloc(m_ringBufferAllocatedSize_);
    m_ringBufferDestAddress_
        = ((uint8_t*)m_ringBuffer_->GetMappedPointer()) + m_ringBufferOffset_;
    if (data) {
      memcpy(m_ringBufferDestAddress_, data, size);
    } else {
      memset(m_ringBufferDestAddress_, 0, size);
    }
  }
}

void UniformBufferBlockDx12::ClearBuffer(int32_t clearValue) {
  assert(m_bufferPtr_);
  UpdateBufferData(nullptr, m_bufferPtr_->GetAllocatedSize());
}

void* UniformBufferBlockDx12::GetLowLevelResource() const {
  return (LifeTimeType::MultiFrame == m_LifeType_) ? m_bufferPtr_->GetHandle()
                                                : m_ringBuffer_->GetHandle();
}

void* UniformBufferBlockDx12::GetLowLevelMemory() const {
  return (LifeTimeType::MultiFrame == m_LifeType_) ? m_bufferPtr_->m_cpuAddress_
                                                : m_ringBufferDestAddress_;
}

size_t UniformBufferBlockDx12::GetBufferSize() const {
  return (LifeTimeType::MultiFrame == m_LifeType_) ? m_bufferPtr_->m_size_
                                                : m_ringBufferAllocatedSize_;
}

const DescriptorDx12& UniformBufferBlockDx12::GetCBV() const {
  if (LifeTimeType::MultiFrame == m_LifeType_) {
    return m_bufferPtr_->m_cbv_;
  }

  return m_ringBuffer_->m_cbv_;
}

uint64_t UniformBufferBlockDx12::GetGPUAddress() const {
  return (LifeTimeType::MultiFrame == m_LifeType_)
           ? m_bufferPtr_->GetGPUAddress()
           : (m_ringBuffer_->GetGPUAddress() + m_ringBufferOffset_);
}

}  // namespace game_engine