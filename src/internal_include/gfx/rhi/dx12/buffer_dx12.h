
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

struct BufferDx12 : public IBuffer {
  BufferDx12() = default;

  BufferDx12(std::shared_ptr<CreatedResource> buffer,
             uint64_t                         size,
             uint64_t                         alignment,
             EBufferCreateFlag bufferCreateFlag = EBufferCreateFlag::NONE)
      : m_buffer(buffer)
      , m_size_(size)
      , m_alignment_(alignment)
      , m_bufferCreateFlag_(bufferCreateFlag) {}

  virtual ~BufferDx12() { release(); }

  virtual void release() override;

  virtual void* getMappedPointer() const override { return m_cpuAddress_; }

  virtual void* map(uint64_t offset, uint64_t size) override {
    assert(m_buffer && m_buffer->isValid());

    if (!(m_bufferCreateFlag_ & EBufferCreateFlag::CPUAccess
          | EBufferCreateFlag::Readback)) {
      return nullptr;
    }

    if (m_cpuAddress_) {
      unmap();
    }

    D3D12_RANGE range = {};
    range.Begin       = offset;
    range.End         = offset + size;
    m_buffer->get()->Map(0, &range, reinterpret_cast<void**>(&m_cpuAddress_));

    return m_cpuAddress_;
  }

  virtual void* map() override {
    assert(m_buffer && m_buffer->isValid());

    if (!(m_bufferCreateFlag_
          & (EBufferCreateFlag::CPUAccess | EBufferCreateFlag::Readback))) {
      return nullptr;
    }

    if (m_cpuAddress_) {
      unmap();
    }

    D3D12_RANGE range = {};
    m_buffer->get()->Map(0, &range, reinterpret_cast<void**>(&m_cpuAddress_));

    return m_cpuAddress_;
  }

  virtual void unmap() override {
    assert(m_buffer && m_buffer->isValid());

    if (!(m_bufferCreateFlag_ & EBufferCreateFlag::CPUAccess
          | EBufferCreateFlag::Readback)) {
      return;
    }

    m_buffer->get()->Unmap(0, nullptr);
    m_cpuAddress_ = nullptr;
  }

  virtual void updateBuffer(const void* data, uint64_t size) override {
    assert(m_cpuAddress_);
    if (m_cpuAddress_) {
      assert(m_size_ >= size);
      memcpy(m_cpuAddress_, data, size);
    }
  }

  virtual void* getHandle() const override { return m_buffer->get(); }

  virtual uint64_t getAllocatedSize() const override {
    return (uint32_t)m_size_;
  }

  virtual uint64_t getBufferSize() const { return (uint32_t)m_size_; }

  virtual uint64_t getOffset() const { return m_offset_; }

  inline uint64_t getGPUAddress() const {
    return m_buffer->getGPUVirtualAddress() + m_offset_;
  }

  virtual EResourceLayout getLayout() const { return m_layout_; }

  EBufferCreateFlag m_bufferCreateFlag_ = EBufferCreateFlag::NONE;
  uint64_t          m_size_             = 0;
  uint64_t          m_alignment_        = 0;
  uint64_t          m_offset_           = 0;
  uint8_t*          m_cpuAddress_       = nullptr;
  // TODO: consider renaming to CreatedRedource
  std::shared_ptr<CreatedResource> m_buffer;
  DescriptorDx12                   m_cbv_;
  DescriptorDx12                   m_srv_;
  DescriptorDx12                   m_uav_;
  EResourceLayout                  m_layout_ = EResourceLayout::UNDEFINED;
};

struct VertexStreamDx12 {
  template <typename T>
  T* getBuffer() const {
    return (T*)m_bufferPtr_.get();
  }

  Name        m_name_;
  // uint32_t Count;
  EBufferType m_bufferType_ = EBufferType::Static;
  // EBufferElementType ElementType = EBufferElementType::BYTE;
  // TODO: seems not used
  bool        m_normalized_ = false;
  int32_t     m_stride_     = 0;
  size_t      m_offset_     = 0;
  // TODO: not used
  // int32_t     m_instanceDivisor_ = 0;

  std::shared_ptr<BufferDx12> m_bufferPtr_;
};

struct VertexBufferDx12 : public VertexBuffer {
  struct BindInfo {
    void reset() {
      m_inputElementDescs_.clear();
      // m_buffers.clear();
      // Offsets.clear();
      m_startBindingIndex_ = 0;
    }

    D3D12_INPUT_LAYOUT_DESC createVertexInputLayoutDesc() const {
      D3D12_INPUT_LAYOUT_DESC Desc;
      Desc.pInputElementDescs = m_inputElementDescs_.data();
      Desc.NumElements        = (uint32_t)m_inputElementDescs_.size();
      return Desc;
    }

    inline size_t getHash() const {
      size_t result = 0;
      for (int32_t i = 0; i < (int32_t)m_inputElementDescs_.size(); ++i) {
        if (m_inputElementDescs_[i].SemanticName) {
          result = ::XXH64(m_inputElementDescs_[i].SemanticName,
                           strlen(m_inputElementDescs_[i].SemanticName),
                           result);
        }
        result = XXH64(m_inputElementDescs_[i].SemanticIndex, result);
        result = XXH64(m_inputElementDescs_[i].Format, result);
        result = XXH64(m_inputElementDescs_[i].InputSlot, result);
        result = XXH64(m_inputElementDescs_[i].AlignedByteOffset, result);
        result = XXH64(m_inputElementDescs_[i].InputSlotClass, result);
        result = XXH64(m_inputElementDescs_[i].InstanceDataStepRate, result);
      }
      return result;
    }

    std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputElementDescs_;

    // m_buffer references VertexStreamDx12, so there's no need for separate
    // destruction
    std::vector<ID3D12Resource*> m_buffers_;
    std::vector<size_t>          m_offsets_;

    int32_t m_startBindingIndex_ = 0;
  };

  virtual size_t getHash() const override {
    if (m_hash_) {
      return m_hash_;
    }

    m_hash_ = getVertexInputStateHash()
            ^ (size_t)m_vertexStreamData_->m_primitiveType_;
    return m_hash_;
  }

  inline size_t getVertexInputStateHash() const {
    return m_bindInfos_.getHash();
  }

  inline D3D12_INPUT_LAYOUT_DESC createVertexInputLayoutDesc() const {
    return m_bindInfos_.createVertexInputLayoutDesc();
  }

  inline D3D12_PRIMITIVE_TOPOLOGY_TYPE getTopologyTypeOnly() const {
    return g_getDX12PrimitiveTopologyTypeOnly(
        m_vertexStreamData_->m_primitiveType_);
  }

  inline D3D12_PRIMITIVE_TOPOLOGY getTopology() const {
    return g_getDX12PrimitiveTopology(m_vertexStreamData_->m_primitiveType_);
  }

  static void s_createVertexInputState(
      std::vector<D3D12_INPUT_ELEMENT_DESC>& inputElementDescs,
      const VertexBufferArray&               vertexBufferArray) {
    for (int32_t i = 0; i < vertexBufferArray.m_numOfData_; ++i) {
      assert(vertexBufferArray[i]);
      const BindInfo& bindInfo
          = ((const VertexBufferDx12*)vertexBufferArray[i])->m_bindInfos_;
      inputElementDescs.insert(inputElementDescs.end(),
                               bindInfo.m_inputElementDescs_.begin(),
                               bindInfo.m_inputElementDescs_.end());
    }
  }

  virtual bool initialize(
      const std::shared_ptr<VertexStreamData>& streamData) override;
  virtual void    bind(const std::shared_ptr<RenderFrameContext>&
                           renderFrameContext) const override;
  virtual void    bind(CommandBufferDx12* commandList) const;
  virtual IBuffer* getBuffer(int32_t streamIndex) const override;

  BindInfo                              m_bindInfos_;
  std::vector<VertexStreamDx12>         m_streams_;
  // TODO: consider renaming
  std::vector<D3D12_VERTEX_BUFFER_VIEW> m_VBView_;
  mutable size_t                        m_hash_ = 0;
};

struct IndexBufferDx12 : public IndexBuffer {
  virtual void bind(const std::shared_ptr<RenderFrameContext>&
                        renderFrameContext) const override;
  virtual void bind(CommandBufferDx12* commandList) const;
  virtual bool initialize(
      const std::shared_ptr<IndexStreamData>& streamData) override;

  inline uint32_t getIndexCount() const {
    return m_indexStreamData_->m_elementCount_;
  }

  virtual BufferDx12* getBuffer() const override { return m_bufferPtr_.get(); }

  std::shared_ptr<BufferDx12> m_bufferPtr_;
  // TODO: consider renaming
  D3D12_INDEX_BUFFER_VIEW     m_IBView_;
};
}  // namespace game_engine
#endif  // GAME_ENGINE_BUFFER_DX12_H
