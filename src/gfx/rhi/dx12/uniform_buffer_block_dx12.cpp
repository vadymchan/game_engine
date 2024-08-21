#include "gfx/rhi/dx12/uniform_buffer_block_dx12.h"

#include "gfx/rhi/dx12/rhi_dx12.h"
#include "gfx/rhi/dx12/utils_dx12.h"
#include "utils/memory/align.h"

namespace game_engine {

UniformBufferBlockDx12::~UniformBufferBlockDx12() {
  release();
}

void UniformBufferBlockDx12::init(size_t size) {
  assert(size);

  size = g_align<uint64_t>(size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);

  if (LifeTimeType::MultiFrame == m_LifeType_) {
    m_bufferPtr_ = g_createBuffer(size,
                             D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT,
                             EBufferCreateFlag::CPUAccess,
                             EResourceLayout::GENERAL,
                             nullptr,
                             0,
                             TEXT(L"UniformBufferBlock"));
    g_createConstantBufferView(m_bufferPtr_.get());
  }
}

void UniformBufferBlockDx12::release() {
}

void UniformBufferBlockDx12::updateBufferData(const void* data,
                                                size_t      size) {
  if (LifeTimeType::MultiFrame == m_LifeType_) {
    assert(m_bufferPtr_);
    assert(m_bufferPtr_->getAllocatedSize() >= size);

    bool isMapped = m_bufferPtr_->map();
    assert(isMapped);

    if (isMapped) {
      uint8_t* startAddr = ((uint8_t*)m_bufferPtr_->getMappedPointer());
      if (data) {
        memcpy(startAddr, data, size);
      } else {
        memset(startAddr, 0, size);
      }
      m_bufferPtr_->unmap();
    }
  } else {
    m_ringBuffer_ = g_rhi_dx12->getOneFrameUniformRingBuffer();
    m_ringBufferAllocatedSize_
        = g_align(size, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
    m_ringBufferOffset_ = m_ringBuffer_->alloc(m_ringBufferAllocatedSize_);
    m_ringBufferDestAddress_
        = ((uint8_t*)m_ringBuffer_->getMappedPointer()) + m_ringBufferOffset_;
    if (data) {
      memcpy(m_ringBufferDestAddress_, data, size);
    } else {
      memset(m_ringBufferDestAddress_, 0, size);
    }
  }
}

void UniformBufferBlockDx12::clearBuffer(int32_t clearValue) {
  assert(m_bufferPtr_);
  updateBufferData(nullptr, m_bufferPtr_->getAllocatedSize());
}

void* UniformBufferBlockDx12::getLowLevelResource() const {
  return (LifeTimeType::MultiFrame == m_LifeType_) ? m_bufferPtr_->getHandle()
                                                : m_ringBuffer_->getHandle();
}

void* UniformBufferBlockDx12::getLowLevelMemory() const {
  return (LifeTimeType::MultiFrame == m_LifeType_) ? m_bufferPtr_->m_cpuAddress_
                                                : m_ringBufferDestAddress_;
}

size_t UniformBufferBlockDx12::getBufferSize() const {
  return (LifeTimeType::MultiFrame == m_LifeType_) ? m_bufferPtr_->m_size_
                                                : m_ringBufferAllocatedSize_;
}

const DescriptorDx12& UniformBufferBlockDx12::getCBV() const {
  if (LifeTimeType::MultiFrame == m_LifeType_) {
    return m_bufferPtr_->m_cbv_;
  }

  return m_ringBuffer_->m_cbv_;
}

uint64_t UniformBufferBlockDx12::getGPUAddress() const {
  return (LifeTimeType::MultiFrame == m_LifeType_)
           ? m_bufferPtr_->getGPUAddress()
           : (m_ringBuffer_->getGPUAddress() + m_ringBufferOffset_);
}

}  // namespace game_engine