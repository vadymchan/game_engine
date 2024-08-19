#ifndef GAME_ENGINE_BUFFER_H
#define GAME_ENGINE_BUFFER_H

#include "gfx/rhi/name.h"
#include "gfx/rhi/render_frame_context.h"
#include "gfx/rhi/resource_container.h"
#include "gfx/rhi/rhi_type.h"
#include "gfx/rhi/shader_bindable_resource.h"

#include <memory>

namespace game_engine {

class IBufferAttribute {
  public:
  struct Attribute {

    Attribute(EBufferElementType UnderlyingType = EBufferElementType::BYTE,
              int32_t            offset         = 0,
              int32_t            stride         = 0)
        : m_underlyingType_(UnderlyingType)
        , m_offset_(offset)
        , m_stride_(stride) {}

    Attribute(const Name&        name,
              EBufferElementType UnderlyingType = EBufferElementType::BYTE,
              int32_t            offset         = 0,
              int32_t            stride         = 0)
        : m_name_(name)
        , m_underlyingType_(UnderlyingType)
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

  virtual ~IBufferAttribute() {}

  virtual const void* GetBufferData() const  = 0;
  virtual size_t      GetBufferSize() const  = 0;
  virtual size_t      GetElementSize() const = 0;

  // private:
  Name                   m_name_;
  EBufferType            m_bufferType_ = EBufferType::Static;
  // TODO: Not needed
  // VkBufferUsageFlags     BufferUsage;
  int32_t                m_stride_ = 0;
  std::vector<Attribute> m_attributes_;
  // TODO: Not needed
  // VkVertexInputRate      InputRate;
};

// TODO: create interface for this class, use custom enums for abstraction
template <typename T>
class BufferAttributeStream : public IBufferAttribute {
  public:
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
      : IBufferAttribute(
          name, bufferType, /*bufferUsage,*/ stride, attributes /*, inputRate*/)
      , m_data_(data) {}

  virtual ~BufferAttributeStream() {}

  virtual const void* GetBufferData() const { return m_data_.data(); }

  virtual size_t GetBufferSize() const { return m_data_.size() * sizeof(T); }

  virtual size_t GetElementSize() const { return sizeof(T); }

  // private:
  std::vector<T> m_data_;
};

struct Buffer
    : public ShaderBindableResource
    , public std::enable_shared_from_this<Buffer> {
  virtual ~Buffer() {}

  virtual void Release() = 0;

  virtual void* GetMappedPointer() const                      = 0;
  virtual void* Map(uint64_t offset, uint64_t size)           = 0;
  virtual void* Map()                                         = 0;
  virtual void  Unmap()                                       = 0;
  virtual void  UpdateBuffer(const void* data, uint64_t size) = 0;

  virtual void*    GetHandle() const = 0;
  virtual uint64_t GetOffset() const = 0;
  virtual uint64_t GetAllocatedSize() const
      = 0;                                     // AllocatedSize from memory pool
  virtual uint64_t GetBufferSize() const = 0;  // RequstedSize

  virtual EResourceLayout GetLayout() const {
    return EResourceLayout::UNDEFINED;
  }
};

class VertexStreamData {
  public:
  virtual ~VertexStreamData() { m_streams_.clear(); }

  int32_t GetEndLocation() const {
    int32_t endLocation = m_startLocation_;
    for (const auto& stream : m_streams_) {
      endLocation += static_cast<int32_t>(stream->m_attributes_.size());
    }
    return endLocation;
  }

  // private:
  // TODO: consider renaming stream(s)
  std::vector<std::shared_ptr<IBufferAttribute>> m_streams_;
  EPrimitiveType   m_primitiveType_   = EPrimitiveType::TRIANGLES;
  EVertexInputRate m_vertexInputRate_ = EVertexInputRate::VERTEX;
  int32_t          m_elementCount_    = 0;
  int32_t          m_bindingIndex_    = 0;
  int32_t          m_startLocation_   = 0;
};

struct VertexBuffer {

  virtual ~VertexBuffer() {}

  virtual Name GetName() const { return Name::s_kInvalid; }

  virtual size_t GetHash() const { return 0; }

  virtual void Bind(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext) const {}

  virtual int32_t GetElementCount() const {
    return m_vertexStreamData_ ? m_vertexStreamData_->m_elementCount_ : 0;
  }

  virtual bool Initialize(
      const std::shared_ptr<VertexStreamData>& streamData) {
    return false;
  }

  //virtual bool IsSupportRaytracing() const { return false; }

  virtual Buffer* GetBuffer(int32_t streamIndex) const { return nullptr; }

  std::shared_ptr<VertexStreamData> m_vertexStreamData_;
};

class IndexStreamData {
  public:
  ~IndexStreamData() { delete m_stream_; }

  // private:
  IBufferAttribute* m_stream_       = nullptr;
  uint32_t          m_elementCount_ = 0;
};

struct IndexBuffer {
  std::shared_ptr<IndexStreamData> m_indexStreamData_;

  virtual ~IndexBuffer() {}

  virtual Name GetName() const { return Name::s_kInvalid; }

  virtual void Bind(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext) const {}

  virtual int32_t GetElementCount() const {
    return m_indexStreamData_ ? m_indexStreamData_->m_elementCount_ : 0;
  }

  virtual bool Initialize(
      const std::shared_ptr<IndexStreamData>& streamData) {
    return false;
  }

  virtual Buffer* GetBuffer() const { return nullptr; }
};

struct VertexBufferArray : public ResourceContainer<const VertexBuffer*> {
  size_t GetHash() const {
    if (m_hash_) {
      return m_hash_;
    }

    m_hash_ = 0;
    for (int32_t i = 0; i < m_numOfData_; ++i) {
      m_hash_ ^= (m_data_[i]->GetHash() << i);
    }
    return m_hash_;
  }

  private:
  mutable size_t m_hash_ = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_BUFFER_H