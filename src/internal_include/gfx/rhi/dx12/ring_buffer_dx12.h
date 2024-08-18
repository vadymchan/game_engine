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

  virtual ~RingBufferDx12() { Release(); }

  virtual void Create(uint64_t totalSize, uint32_t alignment = 16);

  virtual void Reset() {
    ScopedLock s(&m_lock_);

    m_ringBufferOffset_ = 0;
  }

  virtual uint64_t Alloc(uint64_t allocSize) {
    ScopedLock s(&m_lock_);

    const uint64_t allocOffset = Align<uint64_t>(m_ringBufferOffset_, m_alignment_);
    if (allocOffset + allocSize <= m_ringBufferSize_) {
      m_ringBufferOffset_ = allocOffset + allocSize;
      return allocOffset;
    }

    assert(0);

    return 0;
  }

  virtual void Release() override { m_cbv_.Free(); }

  virtual void* GetMappedPointer() const override { return m_mappedPointer_; }

  virtual void* Map(uint64_t offset, uint64_t size) override {
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

  virtual void* Map() override {
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

  virtual void Unmap() override {
    assert(m_mappedPointer_);
    m_buffer_->m_resource_.get()->Get()->Unmap(0, nullptr);
    m_mappedPointer_ = nullptr;
  }

  virtual void UpdateBuffer(const void* data, uint64_t size) override {
    assert(size <= m_ringBufferSize_);

    bool isMapped = Map(0, size);
    assert(isMapped);

    if (isMapped) {
      memcpy(m_mappedPointer_, data, size);
      Unmap();
    }
  }

  virtual void* GetHandle() const override { return m_buffer_->Get(); }

  virtual uint64_t GetAllocatedSize() const override { return m_ringBufferSize_; }

  virtual uint64_t GetBufferSize() const override { return m_ringBufferSize_; }

  virtual uint64_t GetOffset() const override { return m_ringBufferOffset_; }

  inline uint64_t GetGPUAddress() const {
    return m_buffer_->GetGPUVirtualAddress();
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