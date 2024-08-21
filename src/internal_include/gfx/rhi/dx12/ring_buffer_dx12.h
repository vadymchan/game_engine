#ifndef GAME_ENGINE_RING_BUFFER_DX12_H
#define GAME_ENGINE_RING_BUFFER_DX12_H

#include "gfx/rhi/buffer.h"
#include "gfx/rhi/dx12/rhi_type_dx12.h"
// TODO: consider using forward declaration insted of indlude file
#include "gfx/rhi/dx12/descriptor_heap_dx12.h"
#include "gfx/rhi/lock.h"
#include "utils/memory/align.h"

#include <cassert>

namespace game_engine {

struct RingBufferDx12 : public Buffer {
  RingBufferDx12() = default;

  virtual ~RingBufferDx12() { release(); }

  virtual void create(uint64_t totalSize, uint32_t alignment = 16);

  virtual void reset() {
    ScopedLock s(&m_lock_);

    m_ringBufferOffset_ = 0;
  }

  virtual uint64_t alloc(uint64_t allocSize) {
    ScopedLock s(&m_lock_);

    const uint64_t allocOffset = g_align<uint64_t>(m_ringBufferOffset_, m_alignment_);
    if (allocOffset + allocSize <= m_ringBufferSize_) {
      m_ringBufferOffset_ = allocOffset + allocSize;
      return allocOffset;
    }

    assert(0);

    return 0;
  }

  virtual void release() override { m_cbv_.free(); }

  virtual void* getMappedPointer() const override { return m_mappedPointer_; }

  virtual void* map(uint64_t offset, uint64_t size) override {
    assert(size);
    assert(offset + size <= m_ringBufferSize_);
    assert(!m_mappedPointer_);

    const D3D12_RANGE readRange = {.Begin = offset, .End = offset + size};

    HRESULT hr = m_buffer_->m_resource_.get()->Get()->Map(
        0, &readRange, reinterpret_cast<void**>(&m_mappedPointer_));
    assert(SUCCEEDED(hr));

    if (FAILED(hr)) {
      return nullptr;
    }
    return m_mappedPointer_;
  }

  virtual void* map() override {
    assert(m_ringBufferSize_);
    assert(!m_mappedPointer_);
    D3D12_RANGE readRange = {};

    HRESULT hr = m_buffer_->m_resource_.get()->Get()->Map(
        0, &readRange, reinterpret_cast<void**>(&m_mappedPointer_));
    assert(SUCCEEDED(hr));

    if (FAILED(hr)) {
      return nullptr;
    }
    return m_mappedPointer_;
  }

  virtual void unmap() override {
    assert(m_mappedPointer_);
    m_buffer_->m_resource_.get()->Get()->Unmap(0, nullptr);
    m_mappedPointer_ = nullptr;
  }

  virtual void updateBuffer(const void* data, uint64_t size) override {
    assert(size <= m_ringBufferSize_);

    bool isMapped = map(0, size);
    assert(isMapped);

    if (isMapped) {
      memcpy(m_mappedPointer_, data, size);
      unmap();
    }
  }

  virtual void* getHandle() const override { return m_buffer_->get(); }

  virtual uint64_t getAllocatedSize() const override { return m_ringBufferSize_; }

  virtual uint64_t getBufferSize() const override { return m_ringBufferSize_; }

  virtual uint64_t getOffset() const override { return m_ringBufferOffset_; }

  inline uint64_t getGPUAddress() const {
    return m_buffer_->getGPUVirtualAddress();
  }

  uint64_t                         m_ringBufferOffset_ = 0;
  uint32_t                         m_alignment_        = 16;
  std::shared_ptr<CreatedResource> m_buffer_;
  uint64_t                         m_ringBufferSize_ = 0;
  void*                            m_mappedPointer_  = nullptr;

  // TODO: consider whether this is correct naming convention
  DescriptorDx12 m_cbv_;

  MutexLock m_lock_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RING_BUFFER_DX12_H