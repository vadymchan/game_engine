
#ifndef GAME_ENGINE_BUFFER_DX12_H
#define GAME_ENGINE_BUFFER_DX12_H

#include "platform/windows/windows_platform_setup.h"

#ifdef GAME_ENGINE_RHI_DX12

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
  // ======= BEGIN: public constructors =======================================

  BufferDx12() = default;

  BufferDx12(std::shared_ptr<CreatedResourceDx12> buffer,
             uint64_t                             size,
             uint64_t                             alignment,
             EBufferCreateFlag bufferCreateFlag = EBufferCreateFlag::NONE)
      : m_buffer(buffer)
      , m_size_(size)
      , m_alignment_(alignment)
      , m_bufferCreateFlag_(bufferCreateFlag) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~BufferDx12() { release(); }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void release() override;

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

  virtual void* getMappedPointer() const override { return m_cpuAddress_; }

  virtual void* getHandle() const override { return m_buffer->get(); }

  virtual uint64_t getAllocatedSize() const override {
    return (uint32_t)m_size_;
  }

  virtual uint64_t getBufferSize() const { return (uint32_t)m_size_; }

  virtual uint64_t getOffset() const { return m_offset_; }

  virtual EResourceLayout getLayout() const { return m_layout_; }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public getters ============================================

  inline uint64_t getGPUAddress() const {
    return m_buffer->getGPUVirtualAddress() + m_offset_;
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc fields ========================================

  EBufferCreateFlag m_bufferCreateFlag_ = EBufferCreateFlag::NONE;
  uint64_t          m_size_             = 0;
  uint64_t          m_alignment_        = 0;
  uint64_t          m_offset_           = 0;
  uint8_t*          m_cpuAddress_       = nullptr;
  // TODO: consider renaming to CreatedRedource
  std::shared_ptr<CreatedResourceDx12> m_buffer;
  DescriptorDx12                       m_cbv_;
  DescriptorDx12                       m_srv_;
  DescriptorDx12                       m_uav_;
  EResourceLayout                      m_layout_ = EResourceLayout::UNDEFINED;

  // ======= END: public misc fields   ========================================
};

struct VertexStreamDx12 {
  // ======= BEGIN: public getters ============================================

  template <typename T>
  T* getBuffer() const {
    return (T*)m_bufferPtr_.get();
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc fields ========================================

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

  // ======= END: public misc fields   ========================================
};

struct VertexBufferDx12 : public VertexBuffer {
  // ======= BEGIN: public nested types =======================================

  struct BindInfo {
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

    std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputElementDescs_;

    // m_buffer references VertexStreamDx12, so there's no need for separate
    // destruction
    std::vector<ID3D12Resource*> m_buffers_;
    std::vector<size_t>          m_offsets_;

    int32_t m_startBindingIndex_ = 0;
  };

  // ======= END: public nested types   =======================================

  // ======= BEGIN: public static methods =====================================

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

  // ======= END: public static methods   =====================================

  // ======= BEGIN: public overridden methods =================================

  virtual bool initialize(
      const std::shared_ptr<VertexStreamData>& streamData) override;
  virtual void bind(
      const std::shared_ptr<CommandBuffer>& commandList) const override;

  virtual IBuffer* getBuffer(int32_t streamIndex) const override;

  virtual size_t getHash() const override {
    if (m_hash_) {
      return m_hash_;
    }

    m_hash_ = getVertexInputStateHash()
            ^ (size_t)m_vertexStreamData_->m_primitiveType_;
    return m_hash_;
  }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public getters ============================================

  inline size_t getVertexInputStateHash() const {
    return m_bindInfos_.getHash();
  }

  inline D3D12_PRIMITIVE_TOPOLOGY_TYPE getTopologyTypeOnly() const {
    return g_getDX12PrimitiveTopologyTypeOnly(
        m_vertexStreamData_->m_primitiveType_);
  }

  inline D3D12_PRIMITIVE_TOPOLOGY getTopology() const {
    return g_getDX12PrimitiveTopology(m_vertexStreamData_->m_primitiveType_);
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  inline D3D12_INPUT_LAYOUT_DESC createVertexInputLayoutDesc() const {
    return m_bindInfos_.createVertexInputLayoutDesc();
  }

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  BindInfo                              m_bindInfos_;
  std::vector<VertexStreamDx12>         m_streams_;
  // TODO: consider renaming
  std::vector<D3D12_VERTEX_BUFFER_VIEW> m_VBView_;
  mutable size_t                        m_hash_ = 0;

  // ======= END: public misc fields   ========================================
};

struct IndexBufferDx12 : public IndexBuffer {
  // ======= BEGIN: public overridden methods =================================

  virtual void bind(
      const std::shared_ptr<CommandBuffer>& commandList) const override;

  virtual bool initialize(
      const std::shared_ptr<IndexStreamData>& streamData) override;

  virtual BufferDx12* getBuffer() const override { return m_bufferPtr_.get(); }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public getters ============================================

  inline uint32_t getIndexCount() const {
    return m_indexStreamData_->m_elementCount_;
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc fields ========================================

  std::shared_ptr<BufferDx12> m_bufferPtr_;
  // TODO: consider renaming
  D3D12_INDEX_BUFFER_VIEW     m_IBView_;

  // ======= END: public misc fields   ========================================
};
}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12

#endif  // GAME_ENGINE_BUFFER_DX12_H
