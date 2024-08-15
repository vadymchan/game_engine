
#ifndef GAME_ENGINE_BUFFER_DX12_H
#define GAME_ENGINE_BUFFER_DX12_H

#include "gfx/rhi/buffer.h"
#include "gfx/rhi/dx12/descriptor_heap_dx12.h"
#include "gfx/rhi/dx12/rhi_type_dx12.h"
#include "gfx/rhi/shader.h"
#include "utils/third_party/xxhash_util.h"

#include <cassert>
#include <cstdint>

namespace game_engine {

struct CommandBufferDx12;

struct BufferDx12 : public Buffer {
  BufferDx12() = default;

  BufferDx12(std::shared_ptr<CreatedResource> InBuffer,
               uint64_t                          InSize,
               uint64_t                          InAlignment,
               EBufferCreateFlag InBufferCreateFlag = EBufferCreateFlag::NONE)
      : m_buffer(InBuffer)
      , Size(InSize)
      , Alignment(InAlignment)
      , BufferCreateFlag(InBufferCreateFlag) {}

  virtual ~BufferDx12() { Release(); }

  virtual void Release() override;

  virtual void* GetMappedPointer() const override { return CPUAddress; }

  virtual void* Map(uint64_t offset, uint64_t size) override {
    assert(m_buffer && m_buffer->IsValid());

    if (!(BufferCreateFlag & EBufferCreateFlag::CPUAccess
          | EBufferCreateFlag::Readback)) {
      return nullptr;
    }

    if (CPUAddress) {
      Unmap();
    }

    D3D12_RANGE range = {};
    range.Begin       = offset;
    range.End         = offset + size;
    m_buffer->Get()->Map(0, &range, reinterpret_cast<void**>(&CPUAddress));

    return CPUAddress;
  }

  virtual void* Map() override {
    assert(m_buffer && m_buffer->IsValid());

    if (!(BufferCreateFlag
          & (EBufferCreateFlag::CPUAccess | EBufferCreateFlag::Readback))) {
      return nullptr;
    }

    if (CPUAddress) {
      Unmap();
    }

    D3D12_RANGE range = {};
    m_buffer->Get()->Map(0, &range, reinterpret_cast<void**>(&CPUAddress));

    return CPUAddress;
  }

  virtual void Unmap() override {
    assert(m_buffer && m_buffer->IsValid());

    if (!(BufferCreateFlag & EBufferCreateFlag::CPUAccess
          | EBufferCreateFlag::Readback)) {
      return;
    }

    m_buffer->Get()->Unmap(0, nullptr);
    CPUAddress = nullptr;
  }

  virtual void UpdateBuffer(const void* data, uint64_t size) override {
    assert(CPUAddress);
    if (CPUAddress) {
      assert(Size >= size);
      memcpy(CPUAddress, data, size);
    }
  }

  virtual void* GetHandle() const override { return m_buffer->Get(); }

  virtual uint64_t GetAllocatedSize() const override { return (uint32_t)Size; }

  virtual uint64_t GetBufferSize() const { return (uint32_t)Size; }

  virtual uint64_t GetOffset() const { return Offset; }

  inline uint64_t GetGPUAddress() const {
    return m_buffer->GetGPUVirtualAddress() + Offset;
  }

  virtual EResourceLayout GetLayout() const { return Layout; }

  EBufferCreateFlag                 BufferCreateFlag = EBufferCreateFlag::NONE;
  uint64_t                          Size             = 0;
  uint64_t                          Alignment        = 0;
  uint64_t                          Offset           = 0;
  uint8_t*                          CPUAddress       = nullptr;
  // TODO: consider renaming to CreatedRedource
  std::shared_ptr<CreatedResource> m_buffer;
  DescriptorDx12                  CBV;
  DescriptorDx12                  SRV;
  DescriptorDx12                  UAV;
  EResourceLayout                   Layout = EResourceLayout::UNDEFINED;
};

struct VertexStreamDx12 {
  Name       name;
  // uint32_t Count;
  EBufferType BufferType = EBufferType::Static;
  // EBufferElementType ElementType = EBufferElementType::BYTE;
  bool        Normalized      = false;
  int32_t     Stride          = 0;
  size_t      Offset          = 0;
  int32_t     InstanceDivisor = 0;

  template <typename T>
  T* GetBuffer() const {
    return (T*)BufferPtr.get();
  }

  std::shared_ptr<BufferDx12> BufferPtr;
};

struct VertexBufferDx12 : public VertexBuffer {
  struct BindInfo {
    void Reset() {
      InputElementDescs.clear();
      // m_buffers.clear();
      // Offsets.clear();
      StartBindingIndex = 0;
    }

    std::vector<D3D12_INPUT_ELEMENT_DESC> InputElementDescs;

    // m_buffer references VertexStreamDx12, so there's no need for separate
    // destruction
    std::vector<ID3D12Resource*> m_buffers;
    std::vector<size_t>          Offsets;

    int32_t StartBindingIndex = 0;

    D3D12_INPUT_LAYOUT_DESC CreateVertexInputLayoutDesc() const {
      D3D12_INPUT_LAYOUT_DESC Desc;
      Desc.pInputElementDescs = InputElementDescs.data();
      Desc.NumElements        = (uint32_t)InputElementDescs.size();
      return Desc;
    }

    inline size_t GetHash() const {
      size_t result = 0;
      for (int32_t i = 0; i < (int32_t)InputElementDescs.size(); ++i) {
        if (InputElementDescs[i].SemanticName) {
          result = ::XXH64(InputElementDescs[i].SemanticName,
                           strlen(InputElementDescs[i].SemanticName),
                           result);
        }
        result = XXH64(InputElementDescs[i].SemanticIndex, result);
        result = XXH64(InputElementDescs[i].Format, result);
        result = XXH64(InputElementDescs[i].InputSlot, result);
        result = XXH64(InputElementDescs[i].AlignedByteOffset, result);
        result = XXH64(InputElementDescs[i].InputSlotClass, result);
        result = XXH64(InputElementDescs[i].InstanceDataStepRate, result);
      }
      return result;
    }
  };

  virtual size_t GetHash() const override {
    if (Hash) {
      return Hash;
    }

    Hash = GetVertexInputStateHash() ^ (size_t)vertexStreamData->PrimitiveType;
    return Hash;
  }

  inline size_t GetVertexInputStateHash() const { return m_bindInfos.GetHash(); }

  inline D3D12_INPUT_LAYOUT_DESC CreateVertexInputLayoutDesc() const {
    return m_bindInfos.CreateVertexInputLayoutDesc();
  }

  inline D3D12_PRIMITIVE_TOPOLOGY_TYPE GetTopologyTypeOnly() const {
    return GetDX12PrimitiveTopologyTypeOnly(vertexStreamData->PrimitiveType);
  }

  inline D3D12_PRIMITIVE_TOPOLOGY GetTopology() const {
    return GetDX12PrimitiveTopology(vertexStreamData->PrimitiveType);
  }

  static void CreateVertexInputState(
      std::vector<D3D12_INPUT_ELEMENT_DESC>& OutInputElementDescs,
      const VertexBufferArray&              InVertexBufferArray) {
    for (int32_t i = 0; i < InVertexBufferArray.NumOfData; ++i) {
      assert(InVertexBufferArray[i]);
      const BindInfo& bindInfo
          = ((const VertexBufferDx12*)InVertexBufferArray[i])->m_bindInfos;
      OutInputElementDescs.insert(OutInputElementDescs.end(),
                                  bindInfo.InputElementDescs.begin(),
                                  bindInfo.InputElementDescs.end());
    }
  }

  BindInfo                             m_bindInfos;
  std::vector<VertexStreamDx12>       Streams;
  std::vector<D3D12_VERTEX_BUFFER_VIEW> VBView;
  mutable size_t                        Hash = 0;

  virtual bool Initialize(
      const std::shared_ptr<VertexStreamData>& InStreamData) override;
  virtual void     Bind(const std::shared_ptr<RenderFrameContext>&
                            InRenderFrameContext) const override;
  virtual void     Bind(CommandBufferDx12* InCommandList) const;
  virtual Buffer* GetBuffer(int32_t InStreamIndex) const override;
};

struct IndexBufferDx12 : public IndexBuffer {
  std::shared_ptr<BufferDx12> BufferPtr;
  virtual void Bind(const std::shared_ptr<RenderFrameContext>&
                        InRenderFrameContext) const override;
  virtual void Bind(CommandBufferDx12* InCommandList) const;
  virtual bool Initialize(
      const std::shared_ptr<IndexStreamData>& InStreamData) override;

  D3D12_INDEX_BUFFER_VIEW IBView;

  inline uint32_t GetIndexCount() const {
    return indexStreamData->elementCount;
  }

  virtual BufferDx12* GetBuffer() const override { return BufferPtr.get(); }
};
}  // namespace game_engine
#endif  // GAME_ENGINE_BUFFER_DX12_H
