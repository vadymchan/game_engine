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
    ScopedLock s(&Lock);

    RingBufferOffset = 0;
  }

  virtual uint64_t Alloc(uint64_t allocSize) {
    ScopedLock s(&Lock);

    const uint64_t allocOffset = Align<uint64_t>(RingBufferOffset, Alignment);
    if (allocOffset + allocSize <= RingBufferSize) {
      RingBufferOffset = allocOffset + allocSize;
      return allocOffset;
    }

    assert(0);

    return 0;
  }

  virtual void Release() override { CBV.Free(); }

  virtual void* GetMappedPointer() const override { return MappedPointer; }

  virtual void* Map(uint64_t offset, uint64_t size) override {
    assert(size);
    assert(offset + size <= RingBufferSize);
    assert(!MappedPointer);

    const D3D12_RANGE readRange = {.Begin = offset, .End = offset + size};

    HRESULT hr = m_buffer->Resource.get()->Get()->Map(
        0, &readRange, reinterpret_cast<void**>(&MappedPointer));
    assert(SUCCEEDED(hr));

    if (FAILED(hr)) {
      return nullptr;
    }
    return MappedPointer;
  }


  virtual void* Map() override {
    assert(RingBufferSize);
    assert(!MappedPointer);
    D3D12_RANGE readRange = {};

    HRESULT hr = m_buffer->Resource.get()->Get()->Map(
        0, &readRange, reinterpret_cast<void**>(&MappedPointer));
    assert(SUCCEEDED(hr));

    if (FAILED(hr)) {
      return nullptr;
    }
    return MappedPointer;
  }


  virtual void Unmap() override {
    assert(MappedPointer);
    m_buffer->Resource.get()->Get()->Unmap(0, nullptr);
    MappedPointer = nullptr;
  }

  virtual void UpdateBuffer(const void* data, uint64_t size) override {
    assert(size <= RingBufferSize);

    bool isMapped = Map(0, size);
    assert(isMapped);

    if (isMapped) {
      memcpy(MappedPointer, data, size);
      Unmap();
    }
  }

  virtual void* GetHandle() const override { return m_buffer->Get(); }

  virtual uint64_t GetAllocatedSize() const override { return RingBufferSize; }

  virtual uint64_t GetBufferSize() const override { return RingBufferSize; }

  virtual uint64_t GetOffset() const override { return RingBufferOffset; }

  inline uint64_t GetGPUAddress() const {
    return m_buffer->GetGPUVirtualAddress();
  }

  uint64_t                          RingBufferOffset = 0;
  uint32_t                          Alignment        = 16;
  std::shared_ptr<CreatedResource> m_buffer;
  uint64_t                          RingBufferSize = 0;
  void*                             MappedPointer  = nullptr;

  DescriptorDx12 CBV;

  MutexLock Lock;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RING_BUFFER_DX12_H