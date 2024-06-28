
#ifndef GAME_ENGINE_BUFFER_VK_H
#define GAME_ENGINE_BUFFER_VK_H

#include "gfx/rhi/buffer.h"
#include "gfx/rhi/memory_pool.h"
#include "gfx/rhi/name.h"
#include "gfx/rhi/resource_container.h"
#include "utils/third_party/xxhash_util.h"

#include <vulkan/vulkan.h>

#include <cassert>
#include <list>
#include <memory>
#include <vector>

// TODO: separate Buffer / VertexBuffer and IndexBuffer to different classes

namespace game_engine {

// ================================================================================

class BufferVk : public jBuffer {
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
           EResourceLayout   imageLayout);

  BufferVk(const jMemory& InMemory) { InitializeWithMemory(InMemory); }

  ~BufferVk() { Release(); }

  void InitializeWithMemory(const jMemory& InMemory);

  virtual void Release() override;

  virtual void* Map(uint64_t offset, uint64_t size) override;

  virtual void* Map() override;

  virtual void Unmap() override;

  virtual void* GetMappedPointer() const override { return MappedPointer; }

  virtual void UpdateBuffer(const void* data, uint64_t size) override;

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

  virtual void* GetHandle() const override { return m_buffer; }

  virtual uint64_t GetAllocatedSize() const override { return AllocatedSize; }

  virtual uint64_t GetOffset() const override { return Offset; }

  virtual uint64_t GetBufferSize() const override { return RealBufferSize; }

  virtual EResourceLayout GetLayout() const override { return Layout; }

  jMemory         Memory;
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

// TODO: consider make general
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

struct VertexBufferVk : public jVertexBuffer {
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
      const jVertexBufferArray&                       InVertexBufferArray);

  virtual int32_t GetElementCount() const {
    return vertexStreamData ? vertexStreamData->elementCount : 0;
  }

  virtual size_t GetHash() const override {
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

  virtual void Bind(const std::shared_ptr<jRenderFrameContext>&
                        InRenderFrameContext) const override;

  virtual bool Initialize(
      const std::shared_ptr<VertexStreamData>& InStreamData) override;

  jBuffer* GetBuffer(int32_t InStreamIndex) const override {
    assert(Streams.size() > InStreamIndex);
    return Streams[InStreamIndex].BufferPtr.get();
  }

  mutable size_t              Hash = 0;
  BindInfo                    BindInfos;
  std::vector<VertexStreamVk> Streams;
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

struct IndexBufferVk : public jIndexBuffer {
  std::shared_ptr<BufferVk> BufferPtr;

  virtual int32_t GetElementCount() const {
    return indexStreamData ? indexStreamData->elementCount : 0;
  }

  virtual void Bind(const std::shared_ptr<jRenderFrameContext>&
                        InRenderFrameContext) const override;
  virtual bool Initialize(
      const std::shared_ptr<IndexStreamData>& InStreamData) override;

  virtual BufferVk* GetBuffer() const override { return BufferPtr.get(); }

  uint32_t GetIndexCount() const { return indexStreamData->elementCount; }

  VkIndexType GetVulkanIndexFormat(EBufferElementType IndexType) const;
  uint32_t    GetVulkanIndexStride(EBufferElementType IndexType) const;
};

}  // namespace game_engine
#endif  // GAME_ENGINE_BUFFER_VK_H
