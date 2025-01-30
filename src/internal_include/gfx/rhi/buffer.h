#ifndef GAME_ENGINE_BUFFER_H
#define GAME_ENGINE_BUFFER_H

#include "gfx/rhi/name.h"
#include "gfx/rhi/resource_container.h"
#include "gfx/rhi/rhi_type.h"
#include "gfx/rhi/shader_bindable_resource.h"

#include <memory>

namespace game_engine {

class CommandBuffer;

class IBufferAttribute {
  public:
  // ======= BEGIN: public nested types =======================================

  struct Attribute {
    Attribute(EBufferElementType underlyingType = EBufferElementType::BYTE,
              int32_t            offset         = 0,
              int32_t            stride         = 0)
        : m_underlyingType_(underlyingType)
        , m_offset_(offset)
        , m_stride_(stride) {}

    Attribute(const Name&        name,
              EBufferElementType underlyingType = EBufferElementType::BYTE,
              int32_t            offset         = 0,
              int32_t            stride         = 0)
        : m_name_(name)
        , m_underlyingType_(underlyingType)
        , m_offset_(offset)
        , m_stride_(stride) {}

    Name               m_name_;
    // TODO: remove VkFormat (instead EBufferElementType and stride is used)
    // VkFormat format;  // TODO: consider that index buffer use VkIndexType
    // (currently in IndexBufferVk converts VkFormat to
    // VkIndexType). Either use custom enums or union
    EBufferElementType m_underlyingType_;
    int32_t            m_offset_;
    // The stride specifies the byte offset between consecutive elements of this
    // attribute in the buffer.
    int32_t            m_stride_;
  };

  // ======= END: public nested types   =======================================

  // ======= BEGIN: public constructors =======================================

  IBufferAttribute(const Name&                   name,
                   EBufferType                   bufferType,
                   // VkBufferUsageFlags            bufferUsage,
                   int32_t                       stride,
                   const std::vector<Attribute>& attributes
                   //, VkVertexInputRate             inputRate
                   )
      : m_name_(name)
      , m_bufferType_(bufferType)
      //, BufferUsage(bufferUsage)
      , m_stride_(stride)
      , m_attributes_(attributes)
  //, InputRate(inputRate)
  {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~IBufferAttribute() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual const void* getBufferData() const  = 0;
  virtual size_t      getBufferSize() const  = 0;
  virtual size_t      getElementSize() const = 0;

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  Name                   m_name_;
  EBufferType            m_bufferType_ = EBufferType::Static;
  // TODO: Not needed
  // VkBufferUsageFlags     BufferUsage;
  int32_t                m_stride_ = 0;
  std::vector<Attribute> m_attributes_;
  // TODO: Not needed
  // VkVertexInputRate      InputRate;

  // ======= END: public misc fields   ========================================
};

// TODO: create interface for this class, use custom enums for abstraction
template <typename T>
class BufferAttributeStream : public IBufferAttribute {
  public:
  // ======= BEGIN: public constructors =======================================

  BufferAttributeStream() = default;

  BufferAttributeStream(const Name&                   name,
                        EBufferType                   bufferType,
                        // VkBufferUsageFlags            bufferUsage,
                        int32_t                       stride,
                        const std::vector<Attribute>& attributes,
                        const std::vector<T>&         data
                        //, VkVertexInputRate             inputRate
                        //  = VK_VERTEX_INPUT_RATE_VERTEX
                        )
      : IBufferAttribute(name,
                         bufferType,
                         /*bufferUsage,*/ stride,
                         attributes /*, inputRate*/)
      , m_data_(data) {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~BufferAttributeStream() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual const void* getBufferData() const { return m_data_.data(); }

  virtual size_t getBufferSize() const { return m_data_.size() * sizeof(T); }

  virtual size_t getElementSize() const { return sizeof(T); }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  std::vector<T> m_data_;

  // ======= END: public misc fields   ========================================
};

struct IBuffer
    : public ShaderBindableResource
    , public std::enable_shared_from_this<IBuffer> {
  // ======= BEGIN: public destructor =========================================

  virtual ~IBuffer() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void release() = 0;

  virtual void* map(uint64_t offset, uint64_t size)           = 0;
  virtual void* map()                                         = 0;
  virtual void  unmap()                                       = 0;
  virtual void  updateBuffer(const void* data, uint64_t size) = 0;

  virtual void*    getMappedPointer() const = 0;
  virtual void*    getHandle() const        = 0;
  virtual uint64_t getOffset() const        = 0;
  // AllocatedSize from memory pool
  virtual uint64_t getAllocatedSize() const = 0;
  virtual uint64_t getBufferSize() const    = 0;  // RequstedSize

  virtual EResourceLayout getLayout() const {
    return EResourceLayout::UNDEFINED;
  }

  // ======= END: public overridden methods   =================================
};

class VertexStreamData {
  public:
  // ======= BEGIN: public destructor =========================================

  virtual ~VertexStreamData() { m_streams_.clear(); }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public misc fields ========================================

  // TODO: consider renaming stream(s)
  std::vector<std::shared_ptr<IBufferAttribute>> m_streams_;
  EPrimitiveType   m_primitiveType_   = EPrimitiveType::TRIANGLES;
  EVertexInputRate m_vertexInputRate_ = EVertexInputRate::VERTEX;
  int32_t          m_elementCount_    = 0;
  int32_t          m_bindingIndex_    = 0;
  int32_t          m_startLocation_   = 0;

  // ======= END: public misc fields   ========================================

  // ======= BEGIN: public getters ============================================

  int32_t getEndLocation() const {
    int32_t endLocation = m_startLocation_;
    for (const auto& stream : m_streams_) {
      endLocation += static_cast<int32_t>(stream->m_attributes_.size());
    }
    return endLocation;
  }

  // ======= END: public getters   ============================================
};

struct VertexBuffer {
  // ======= BEGIN: public destructor =========================================

  virtual ~VertexBuffer() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual bool initialize(const std::shared_ptr<VertexStreamData>& streamData) {
    return false;
  }

  virtual void bind(const std::shared_ptr<CommandBuffer>& commandList) const {}

  // virtual bool IsSupportRaytracing() const { return false; }

  virtual Name getName() const { return Name::s_kInvalid; }

  virtual size_t getHash() const { return 0; }

  virtual int32_t getElementCount() const {
    return m_vertexStreamData_ ? m_vertexStreamData_->m_elementCount_ : 0;
  }

  virtual IBuffer* getBuffer(int32_t streamIndex) const { return nullptr; }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  std::shared_ptr<VertexStreamData> m_vertexStreamData_;

  // ======= END: public misc fields   ========================================
};

class IndexStreamData {
  public:
  // ======= BEGIN: public destructor =========================================

  ~IndexStreamData() { delete m_stream_; }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public misc fields ========================================

  IBufferAttribute* m_stream_       = nullptr;
  uint32_t          m_elementCount_ = 0;

  // ======= END: public misc fields   ========================================
};

struct IndexBuffer {
  // ======= BEGIN: public destructor =========================================

  virtual ~IndexBuffer() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual bool initialize(const std::shared_ptr<IndexStreamData>& streamData) {
    return false;
  }

  virtual void bind(const std::shared_ptr<CommandBuffer>& commandList) const {}

  virtual Name getName() const { return Name::s_kInvalid; }

  virtual IBuffer* getBuffer() const { return nullptr; }

  virtual int32_t getElementCount() const {
    return m_indexStreamData_ ? m_indexStreamData_->m_elementCount_ : 0;
  }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc fields ========================================

  std::shared_ptr<IndexStreamData> m_indexStreamData_;

  // ======= END: public misc fields   ========================================
};

struct VertexBufferArray : public ResourceContainer<const VertexBuffer*> {
  // ======= BEGIN: public getters ============================================

  size_t getHash() const {
    if (m_hash_) {
      return m_hash_;
    }

    m_hash_ = 0;
    for (int32_t i = 0; i < m_numOfData_; ++i) {
      m_hash_ ^= (m_data_[i]->getHash() << i);
    }
    return m_hash_;
  }

  // ======= END: public getters   ============================================

  private:
  // ======= BEGIN: private misc fields =======================================

  mutable size_t m_hash_ = 0;

  // ======= END: private misc fields   =======================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_BUFFER_H
