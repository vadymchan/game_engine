
#ifndef GAME_ENGINE_BUFFER_VK_H
#define GAME_ENGINE_BUFFER_VK_H

#include "gfx/rhi/name.h"
#include "gfx/rhi/resource_container.h"
#include "gfx/rhi/vulkan/command_buffer_vk.h"
#include "gfx/rhi/vulkan/memory_pool_vk.h"
#include "gfx/rhi/vulkan/render_frame_context_vk.h"
#include "gfx/rhi/vulkan/render_target_vk.h"
#include "gfx/rhi/vulkan/shader_vk.h"
#include "gfx/rhi/vulkan/swapchain_vk.h"
#include "utils/third_party/xxhash_util.h"

#include <vulkan/vulkan.h>

#include <cassert>
#include <list>
#include <memory>
#include <vector>

namespace game_engine {

// enum class EBufferType {
//   Static,
//   Dynamic,
//   Max
// };

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

class VertexStreamData {
  public:
  int32_t GetEndLocation() const;

  // private:
  // TODO: consider renaming stream(s)
  std::vector<std::shared_ptr<IBufferAttribute>> streams;
  EPrimitiveType   PrimitiveType   = EPrimitiveType::TRIANGLES;
  EVertexInputRate VertexInputRate = EVertexInputRate::VERTEX;
  int32_t          elementCount    = 0;
  int32_t          bindingIndex    = 0;
  int32_t          startLocation   = 0;
};

class IndexStreamData {
  public:
  // private:
  IBufferAttribute* stream       = nullptr;
  uint32_t          elementCount = 0;
};

// ================================================================================

class BufferVk {
  public:
  BufferVk()
      : m_buffer(VK_NULL_HANDLE)
      , m_memory(VK_NULL_HANDLE)
      , m_size(0)
      , MappedPointer(nullptr)
      , Offset(0)
      , AllocatedSize(0)
      , HasBufferOwnership(true)
      , Layout(EResourceLayout::UNDEFINED) {}

  BufferVk(VkDeviceSize      size,
           EVulkanBufferBits usage,
           EVulkanMemoryBits properties,
           EResourceLayout   imageLayout)
      : m_size(size)
      , Layout(imageLayout) {
    CreateBuffer(usage, properties, size, imageLayout);
  }

  BufferVk(const MemoryVk& InMemory) { InitializeWithMemory(InMemory); }

  ~BufferVk() { Release(); }

  void InitializeWithMemory(const MemoryVk& InMemory);

  void Release();

  void* Map(uint64_t offset = 0, uint64_t size = VK_WHOLE_SIZE);

  void Unmap();

  void* GetMappedPointer() const { return MappedPointer; }

  void UpdateBuffer(const void* data, uint64_t size);

  // private:
  // TODO: consider remove it
  // void createBuffer(VkDeviceSize          size,
  //                  VkBufferUsageFlags    usage,
  //                  VkMemoryPropertyFlags properties) {
  //  VkBufferCreateInfo bufferInfo{};
  //  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  //  bufferInfo.size  = size;
  //  bufferInfo.usage = usage;
  //  // TODO: consider remove hard code for Multi-Queue Families support
  //  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  //  if (vkCreateBuffer(g_rhi_vk->m_device_, &bufferInfo, nullptr, &m_buffer)
  //      != VK_SUCCESS) {
  //    // TODO: log error
  //  }

  //  VkMemoryRequirements memRequirements;
  //  vkGetBufferMemoryRequirements(
  //      g_rhi_vk->m_device_, m_buffer, &memRequirements);

  //  VkMemoryAllocateInfo allocInfo{};
  //  allocInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  //  allocInfo.allocationSize = memRequirements.size;
  //  allocInfo.memoryTypeIndex
  //      = findMemoryType(memRequirements.memoryTypeBits, properties);

  //  if (vkAllocateMemory(g_rhi_vk->m_device_, &allocInfo, nullptr, &m_memory)
  //      != VK_SUCCESS) {
  //    // TODO: log error
  //  }

  //  vkBindBufferMemory(g_rhi_vk->m_device_, m_buffer, m_memory, 0);
  //}

  // TODO: consider remove it
  // uint32_t findMemoryType(uint32_t              typeFilter,
  //                        VkMemoryPropertyFlags properties) {
  //  VkPhysicalDeviceMemoryProperties memProperties;
  //  vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

  //  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
  //    if ((typeFilter & (1 << i))
  //        && (memProperties.memoryTypes[i].propertyFlags & properties)
  //               == properties) {
  //      return i;
  //    }
  //  }

  // TODO: log error
  //}

  virtual void* GetHandle() const { return m_buffer; }

  virtual uint64_t GetAllocatedSize() const { return AllocatedSize; }

  virtual uint64_t GetOffset() const { return Offset; }

  // TODO:

  virtual uint64_t GetBufferSize() const { return RealBufferSize; }

  // virtual EResourceLayout GetLayout() const { return Layout; }

  MemoryVk        Memory;
  VkBuffer        m_buffer = VK_NULL_HANDLE;
  VkDeviceMemory  m_memory = VK_NULL_HANDLE;
  VkDeviceSize    m_size;  // TODO: consider about placing to VertexBufferVk
  void*           MappedPointer      = nullptr;
  size_t          Offset             = 0;
  uint64_t        AllocatedSize      = 0;
  uint64_t        RealBufferSize     = 0;
  bool            HasBufferOwnership = true;
  EResourceLayout Layout
      = EResourceLayout::UNDEFINED;  // TODO: previously was m_imageLayout
};

// ============== Vertex Buffer Vk =======================

struct VertexStreamVk {
  Name        name;
  EBufferType BufferType = EBufferType::Static;
  bool        Normalized = false;
  int32_t     Stride     = 0;
  size_t      Offset     = 0;
  // int32_t            InstanceDivisor = 0;

  std::shared_ptr<BufferVk> BufferPtr;
};

struct VertexBufferArrayVk;

struct VertexBufferVk {
  struct BindInfo {
    void Reset();

    std::vector<VkVertexInputBindingDescription>   InputBindingDescriptions;
    std::vector<VkVertexInputAttributeDescription> AttributeDescriptions;

    // Buffer references VertexStreamVk, so no destruction needed
    std::vector<VkBuffer>     Buffers;
    std::vector<VkDeviceSize> Offsets;

    int32_t StartBindingIndex = 0;

    VkPipelineVertexInputStateCreateInfo CreateVertexInputState() const;

    size_t GetHash() const {
      size_t result = 0;
      for (int32_t i = 0; i < (int32_t)InputBindingDescriptions.size(); ++i) {
        result = XXH64(InputBindingDescriptions[i].binding, result);
        result = XXH64(InputBindingDescriptions[i].stride, result);
        result = XXH64(InputBindingDescriptions[i].inputRate, result);
      }

      for (int32_t i = 0; i < (int32_t)AttributeDescriptions.size(); ++i) {
        result = XXH64(AttributeDescriptions[i].location, result);
        result = XXH64(AttributeDescriptions[i].binding, result);
        result = XXH64(AttributeDescriptions[i].format, result);
        result = XXH64(AttributeDescriptions[i].offset, result);
      }

      return result;
    }
  };

  VkPipelineVertexInputStateCreateInfo CreateVertexInputState() const {
    return BindInfos.CreateVertexInputState();
  }

  VkPipelineInputAssemblyStateCreateInfo CreateInputAssemblyState() const;

  static void CreateVertexInputState(
      VkPipelineVertexInputStateCreateInfo&           OutVertexInputInfo,
      std::vector<VkVertexInputBindingDescription>&   OutBindingDescriptions,
      std::vector<VkVertexInputAttributeDescription>& OutAttributeDescriptions,
      const VertexBufferArrayVk&                      InVertexBufferArray);

  virtual int32_t GetElementCount() const {
    return vertexStreamData ? vertexStreamData->elementCount : 0;
  }

  virtual size_t GetHash() const {
    if (Hash) {
      return Hash;
    }

    Hash = GetVertexInputStateHash() ^ GetInputAssemblyStateHash();
    return Hash;
  }

  size_t GetVertexInputStateHash() const { return BindInfos.GetHash(); }

  size_t GetInputAssemblyStateHash() const {
    VkPipelineInputAssemblyStateCreateInfo state = CreateInputAssemblyState();
    return GETHASH_FROM_INSTANT_STRUCT(state);
  }

  virtual void Bind(
      const std::shared_ptr<RenderFrameContextVk>& InRenderFrameContext) const;

  virtual bool Initialize(
      const std::shared_ptr<VertexStreamData>& InStreamData);

  mutable size_t                    Hash = 0;
  BindInfo                          BindInfos;
  std::vector<VertexStreamVk>       Streams;
  std::shared_ptr<VertexStreamData> vertexStreamData;
};

struct VertexBufferArrayVk : public ResourceContainer<const VertexBufferVk*> {
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

// ================ Index Buffer Vk ===========================

struct IndexBufferVk {
  std::shared_ptr<BufferVk>        BufferPtr;
  std::shared_ptr<IndexStreamData> indexStreamData;

  virtual int32_t GetElementCount() const {
    return indexStreamData ? indexStreamData->elementCount : 0;
  }

  virtual void Bind(const Shader* shader) const {}

  virtual void Bind(
      const std::shared_ptr<RenderFrameContextVk>& InRenderFrameContext) const;
  virtual bool Initialize(const std::shared_ptr<IndexStreamData>& InStreamData);

  uint32_t GetIndexCount() const { return indexStreamData->elementCount; }

  VkIndexType GetVulkanIndexFormat(EBufferElementType IndexType) const;
  uint32_t    GetVulkanIndexStride(EBufferElementType IndexType) const;
};



}  // namespace game_engine
#endif  // GAME_ENGINE_BUFFER_VK_H
