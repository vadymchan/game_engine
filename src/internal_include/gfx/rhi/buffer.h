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
    Name               name;
    // TODO: remove VkFormat (instead EBufferElementType and stride is used)
    // VkFormat format;  // TODO: consider that index buffer use VkIndexType
    // (currently in IndexBufferVk converts VkFormat to
    // VkIndexType). Either use custom enums or union
    EBufferElementType UnderlyingType;
    int32_t            offset;
    // The stride specifies the byte offset between consecutive elements of this
    // attribute in the buffer.
    int32_t            stride;

    Attribute(EBufferElementType UnderlyingType = EBufferElementType::BYTE,
              int32_t            offset         = 0,
              int32_t            stride         = 0)
        : UnderlyingType(UnderlyingType)
        , offset(offset)
        , stride(stride) {}

    Attribute(const Name&        name,
              EBufferElementType UnderlyingType = EBufferElementType::BYTE,
              int32_t            offset         = 0,
              int32_t            stride         = 0)
        : name(name)
        , UnderlyingType(UnderlyingType)
        , offset(offset)
        , stride(stride) {}
  };

  IBufferAttribute(const Name&                   name,
                   EBufferType                   bufferType,
                   // VkBufferUsageFlags            bufferUsage,
                   int32_t                       stride,
                   const std::vector<Attribute>& attributes
                   //, VkVertexInputRate             inputRate
                   )
      : name(name)
      , BufferType(bufferType)
      //, BufferUsage(bufferUsage)
      , Stride(stride)
      , Attributes(attributes)
  //, InputRate(inputRate)
  {}

  virtual ~IBufferAttribute() {}

  virtual const void* GetBufferData() const  = 0;
  virtual size_t      GetBufferSize() const  = 0;
  virtual size_t      GetElementSize() const = 0;

  // private:
  Name                   name;
  EBufferType            BufferType = EBufferType::Static;
  // TODO: Not needed
  // VkBufferUsageFlags     BufferUsage;
  int32_t                Stride = 0;
  std::vector<Attribute> Attributes;
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
      , Data(data) {}

  virtual ~BufferAttributeStream() {}

  virtual const void* GetBufferData() const { return Data.data(); }

  virtual size_t GetBufferSize() const { return Data.size() * sizeof(T); }

  virtual size_t GetElementSize() const { return sizeof(T); }

  // private:
  std::vector<T> Data;
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
  virtual ~VertexStreamData() { streams.clear(); }

  int32_t GetEndLocation() const {
    int32_t endLocation = startLocation;
    for (const auto& stream : streams) {
      endLocation += static_cast<int32_t>(stream->Attributes.size());
    }
    return endLocation;
  }

  // private:
  // TODO: consider renaming stream(s)
  std::vector<std::shared_ptr<IBufferAttribute>> streams;
  EPrimitiveType   PrimitiveType   = EPrimitiveType::TRIANGLES;
  EVertexInputRate VertexInputRate = EVertexInputRate::VERTEX;
  int32_t          elementCount    = 0;
  int32_t          bindingIndex    = 0;
  int32_t          startLocation   = 0;
};

struct VertexBuffer {
  std::shared_ptr<VertexStreamData> vertexStreamData;

  virtual ~VertexBuffer() {}

  virtual Name GetName() const { return Name::Invalid; }

  virtual size_t GetHash() const { return 0; }

  virtual void Bind(
      const std::shared_ptr<RenderFrameContext>& InRenderFrameContext) const {}

  virtual int32_t GetElementCount() const {
    return vertexStreamData ? vertexStreamData->elementCount : 0;
  }

  virtual bool Initialize(
      const std::shared_ptr<VertexStreamData>& InStreamData) {
    return false;
  }

  //virtual bool IsSupportRaytracing() const { return false; }

  virtual Buffer* GetBuffer(int32_t InStreamIndex) const { return nullptr; }
};

class IndexStreamData {
  public:
  ~IndexStreamData() { delete stream; }

  // private:
  IBufferAttribute* stream       = nullptr;
  uint32_t          elementCount = 0;
};

struct IndexBuffer {
  std::shared_ptr<IndexStreamData> indexStreamData;

  virtual ~IndexBuffer() {}

  virtual Name GetName() const { return Name::Invalid; }

  virtual void Bind(
      const std::shared_ptr<RenderFrameContext>& InRenderFrameContext) const {}

  virtual int32_t GetElementCount() const {
    return indexStreamData ? indexStreamData->elementCount : 0;
  }

  virtual bool Initialize(
      const std::shared_ptr<IndexStreamData>& InStreamData) {
    return false;
  }

  virtual Buffer* GetBuffer() const { return nullptr; }
};

struct VertexBufferArray : public ResourceContainer<const VertexBuffer*> {
  size_t GetHash() const {
    if (Hash) {
      return Hash;
    }

    Hash = 0;
    for (int32_t i = 0; i < NumOfData; ++i) {
      Hash ^= (Data[i]->GetHash() << i);
    }
    return Hash;
  }

  private:
  mutable size_t Hash = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_BUFFER_H