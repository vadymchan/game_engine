
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

struct jCommandBuffer_DX12;

struct jBuffer_DX12 : public jBuffer {
  jBuffer_DX12() = default;

  jBuffer_DX12(std::shared_ptr<jCreatedResource> InBuffer,
               uint64_t                          InSize,
               uint64_t                          InAlignment,
               EBufferCreateFlag InBufferCreateFlag = EBufferCreateFlag::NONE)
      : Buffer(InBuffer)
      , Size(InSize)
      , Alignment(InAlignment)
      , BufferCreateFlag(InBufferCreateFlag) {}

  virtual ~jBuffer_DX12() { Release(); }

  virtual void Release() override;

  virtual void* GetMappedPointer() const override { return CPUAddress; }

  virtual void* Map(uint64_t offset, uint64_t size) override {
    assert(Buffer && Buffer->IsValid());

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
    Buffer->Get()->Map(0, &range, reinterpret_cast<void**>(&CPUAddress));

    return CPUAddress;
  }

  virtual void* Map() override {
    assert(Buffer && Buffer->IsValid());

    if (!(BufferCreateFlag
          & (EBufferCreateFlag::CPUAccess | EBufferCreateFlag::Readback))) {
      return nullptr;
    }

    if (CPUAddress) {
      Unmap();
    }

    D3D12_RANGE range = {};
    Buffer->Get()->Map(0, &range, reinterpret_cast<void**>(&CPUAddress));

    return CPUAddress;
  }

  virtual void Unmap() override {
    assert(Buffer && Buffer->IsValid());

    if (!(BufferCreateFlag & EBufferCreateFlag::CPUAccess
          | EBufferCreateFlag::Readback)) {
      return;
    }

    Buffer->Get()->Unmap(0, nullptr);
    CPUAddress = nullptr;
  }

  virtual void UpdateBuffer(const void* data, uint64_t size) override {
    assert(CPUAddress);
    if (CPUAddress) {
      assert(Size >= size);
      memcpy(CPUAddress, data, size);
    }
  }

  virtual void* GetHandle() const override { return Buffer->Get(); }

  virtual uint64_t GetAllocatedSize() const override { return (uint32_t)Size; }

  virtual uint64_t GetBufferSize() const { return (uint32_t)Size; }

  virtual uint64_t GetOffset() const { return Offset; }

  inline uint64_t GetGPUAddress() const {
    return Buffer->GetGPUVirtualAddress() + Offset;
  }

  virtual EResourceLayout GetLayout() const { return Layout; }

  EBufferCreateFlag                 BufferCreateFlag = EBufferCreateFlag::NONE;
  uint64_t                          Size             = 0;
  uint64_t                          Alignment        = 0;
  uint64_t                          Offset           = 0;
  uint8_t*                          CPUAddress       = nullptr;
  std::shared_ptr<jCreatedResource> Buffer;
  jDescriptor_DX12                  CBV;
  jDescriptor_DX12                  SRV;
  jDescriptor_DX12                  UAV;
  EResourceLayout                   Layout = EResourceLayout::UNDEFINED;
};

struct jVertexStream_DX12 {
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

  std::shared_ptr<jBuffer_DX12> BufferPtr;
};

struct jVertexBuffer_DX12 : public jVertexBuffer {
  struct jBindInfo {
    void Reset() {
      InputElementDescs.clear();
      // Buffers.clear();
      // Offsets.clear();
      StartBindingIndex = 0;
    }

    std::vector<D3D12_INPUT_ELEMENT_DESC> InputElementDescs;

    // Buffer references jVertexStream_DX12, so there's no need for separate
    // destruction
    std::vector<ID3D12Resource*> Buffers;
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

  inline size_t GetVertexInputStateHash() const { return BindInfos.GetHash(); }

  inline D3D12_INPUT_LAYOUT_DESC CreateVertexInputLayoutDesc() const {
    return BindInfos.CreateVertexInputLayoutDesc();
  }

  inline D3D12_PRIMITIVE_TOPOLOGY_TYPE GetTopologyTypeOnly() const {
    return GetDX12PrimitiveTopologyTypeOnly(vertexStreamData->PrimitiveType);
  }

  inline D3D12_PRIMITIVE_TOPOLOGY GetTopology() const {
    return GetDX12PrimitiveTopology(vertexStreamData->PrimitiveType);
  }

  static void CreateVertexInputState(
      std::vector<D3D12_INPUT_ELEMENT_DESC>& OutInputElementDescs,
      const jVertexBufferArray&              InVertexBufferArray) {
    for (int32_t i = 0; i < InVertexBufferArray.NumOfData; ++i) {
      assert(InVertexBufferArray[i]);
      const jBindInfo& bindInfo
          = ((const jVertexBuffer_DX12*)InVertexBufferArray[i])->BindInfos;
      OutInputElementDescs.insert(OutInputElementDescs.end(),
                                  bindInfo.InputElementDescs.begin(),
                                  bindInfo.InputElementDescs.end());
    }
  }

  jBindInfo                             BindInfos;
  std::vector<jVertexStream_DX12>       Streams;
  std::vector<D3D12_VERTEX_BUFFER_VIEW> VBView;
  mutable size_t                        Hash = 0;

  virtual bool Initialize(
      const std::shared_ptr<VertexStreamData>& InStreamData) override;
  virtual void     Bind(const std::shared_ptr<jRenderFrameContext>&
                            InRenderFrameContext) const override;
  virtual void     Bind(jCommandBuffer_DX12* InCommandList) const;
  virtual jBuffer* GetBuffer(int32_t InStreamIndex) const override;
};

struct jIndexBuffer_DX12 : public jIndexBuffer {
  std::shared_ptr<jBuffer_DX12> BufferPtr;
  virtual void Bind(const std::shared_ptr<jRenderFrameContext>&
                        InRenderFrameContext) const override;
  virtual void Bind(jCommandBuffer_DX12* InCommandList) const;
  virtual bool Initialize(
      const std::shared_ptr<IndexStreamData>& InStreamData) override;

  D3D12_INDEX_BUFFER_VIEW IBView;

  inline uint32_t GetIndexCount() const {
    return indexStreamData->elementCount;
  }

  virtual jBuffer_DX12* GetBuffer() const override { return BufferPtr.get(); }
};
}  // namespace game_engine
#endif  // GAME_ENGINE_BUFFER_DX12_H
