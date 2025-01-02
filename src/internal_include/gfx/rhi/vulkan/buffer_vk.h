
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

// TODO: separate m_buffer / VertexBuffer and IndexBuffer to different classes

namespace game_engine {

// ================================================================================

class BufferVk : public IBuffer {
  public:
  // ======= BEGIN: public constructors =======================================

  BufferVk()
      : m_buffer_(VK_NULL_HANDLE)
      , m_deviceMemory_(VK_NULL_HANDLE)
      , m_size_(0)
      , m_mappedPointer_(nullptr)
      , m_offset_(0)
      , m_allocatedSize_(0)
      , m_hasBufferOwnership_(true)
      , m_layout_(EResourceLayout::UNDEFINED) {}

  BufferVk(VkDeviceSize      size,
           EVulkanBufferBits usage,
           EVulkanMemoryBits properties,
           EResourceLayout   imageLayout);

  BufferVk(const Memory& memory) { initializeWithMemory(memory); }

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  ~BufferVk() { release(); }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void release() override;

  virtual void* map(uint64_t offset, uint64_t size) override;

  virtual void* map() override;

  virtual void unmap() override;

  virtual void updateBuffer(const void* data, uint64_t size) override;

  virtual void* getMappedPointer() const override { return m_mappedPointer_; }

  virtual void* getHandle() const override { return m_buffer_; }

  virtual uint64_t getAllocatedSize() const override {
    return m_allocatedSize_;
  }

  virtual uint64_t getOffset() const override { return m_offset_; }

  virtual uint64_t getBufferSize() const override { return m_realBufferSize_; }

  virtual EResourceLayout getLayout() const override { return m_layout_; }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public misc methods =======================================

  void initializeWithMemory(const Memory& memory);

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
  //  if (vkCreateBuffer(g_rhiVk->m_device_, &bufferInfo, nullptr, &m_buffer)
  //      != VK_SUCCESS) {
  //    // TODO: log error
  //  }

  //  VkMemoryRequirements memRequirements;
  //  vkGetBufferMemoryRequirements(
  //      g_rhiVk->m_device_, m_buffer, &memRequirements);

  //  VkMemoryAllocateInfo allocInfo{};
  //  allocInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  //  allocInfo.allocationSize = memRequirements.size;
  //  allocInfo.memoryTypeIndex
  //      = findMemoryType(memRequirements.memoryTypeBits, properties);

  //  if (vkAllocateMemory(g_rhiVk->m_device_, &allocInfo, nullptr,
  //  &m_deviceMemory)
  //      != VK_SUCCESS) {
  //    // TODO: log error
  //  }

  //  vkBindBufferMemory(g_rhiVk->m_device_, m_buffer, m_deviceMemory, 0);
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

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  // TODO: consider descriptive names for Memory and VkDeviceMemory
  Memory          m_memory_;
  VkBuffer        m_buffer_       = VK_NULL_HANDLE;
  VkDeviceMemory  m_deviceMemory_ = VK_NULL_HANDLE;
  VkDeviceSize    m_size_;  // TODO: consider about placing to VertexBufferVk
  void*           m_mappedPointer_      = nullptr;
  size_t          m_offset_             = 0;
  uint64_t        m_allocatedSize_      = 0;
  uint64_t        m_realBufferSize_     = 0;
  bool            m_hasBufferOwnership_ = true;
  EResourceLayout m_layout_
      = EResourceLayout::UNDEFINED;  // TODO: previously was m_imageLayout

  // ======= END: public misc fields   ========================================
};

// ============== Vertex m_buffer Vk =======================

// TODO: consider make general
struct VertexStreamVk {
  // ======= BEGIN: public misc fields ========================================

  Name        m_name_;
  EBufferType m_bufferType_ = EBufferType::Static;
  // TODO: not used (consider remove)
  bool        m_normalized_ = false;
  int32_t     m_stride_     = 0;
  size_t      m_offset_     = 0;
  // int32_t            m_instanceDivisor_ = 0;

  std::shared_ptr<BufferVk> m_bufferPtr_;

  // ======= END: public misc fields   ========================================
};

struct VertexBufferArrayVk;

struct VertexBufferVk : public VertexBuffer {
  // ======= BEGIN: public nested types =======================================

  struct BindInfo {
    void reset();

    VkPipelineVertexInputStateCreateInfo createVertexInputState() const;

    size_t getHash() const {
      size_t result = 0;
      for (int32_t i = 0; i < (int32_t)m_inputBindingDescriptions_.size();
           ++i) {
        result = XXH64(m_inputBindingDescriptions_[i].binding, result);
        result = XXH64(m_inputBindingDescriptions_[i].stride, result);
        result = XXH64(m_inputBindingDescriptions_[i].inputRate, result);
      }

      for (int32_t i = 0; i < (int32_t)m_attributeDescriptions_.size(); ++i) {
        result = XXH64(m_attributeDescriptions_[i].location, result);
        result = XXH64(m_attributeDescriptions_[i].binding, result);
        result = XXH64(m_attributeDescriptions_[i].format, result);
        result = XXH64(m_attributeDescriptions_[i].offset, result);
      }

      return result;
    }

    std::vector<VkVertexInputBindingDescription>   m_inputBindingDescriptions_;
    std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions_;

    // m_buffer references VertexStreamVk, so no destruction needed
    std::vector<VkBuffer>     m_buffers_;
    std::vector<VkDeviceSize> m_offsets_;

    int32_t m_startBindingIndex_ = 0;
  };

  // ======= END: public nested types   =======================================

  // ======= BEGIN: public static methods =====================================

  static void s_createVertexInputState(
      VkPipelineVertexInputStateCreateInfo&           vertexInputInfo,
      std::vector<VkVertexInputBindingDescription>&   bindingDescriptions,
      std::vector<VkVertexInputAttributeDescription>& attributeDescriptions,
      const VertexBufferArray&                        vertexBufferArray);

  // ======= END: public static methods   =====================================

  // ======= BEGIN: public overridden methods =================================

  virtual bool initialize(
      const std::shared_ptr<VertexStreamData>& streamData) override;

  virtual void bind(
      const std::shared_ptr<CommandBuffer>& commandBuffer) const override;

  virtual int32_t getElementCount() const {
    return m_vertexStreamData_ ? m_vertexStreamData_->m_elementCount_ : 0;
  }

  virtual size_t getHash() const override {
    if (m_hash_) {
      return m_hash_;
    }

    m_hash_ = getVertexInputStateHash() ^ getInputAssemblyStateHash();
    return m_hash_;
  }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public getters ============================================

  IBuffer* getBuffer(int32_t streamIndex) const override {
    assert(m_streams_.size() > streamIndex);
    return m_streams_[streamIndex].m_bufferPtr_.get();
  }

  size_t getVertexInputStateHash() const { return m_bindInfos_.getHash(); }

  size_t getInputAssemblyStateHash() const {
    VkPipelineInputAssemblyStateCreateInfo state = createInputAssemblyState();
    return GETHASH_FROM_INSTANT_STRUCT(state);
  }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  VkPipelineVertexInputStateCreateInfo createVertexInputState() const {
    return m_bindInfos_.createVertexInputState();
  }

  VkPipelineInputAssemblyStateCreateInfo createInputAssemblyState() const;

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  mutable size_t              m_hash_ = 0;
  BindInfo                    m_bindInfos_;
  std::vector<VertexStreamVk> m_streams_;

  // ======= END: public misc fields   ========================================
};

struct VertexBufferArrayVk : public ResourceContainer<const VertexBufferVk*> {
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

// ================ Index m_buffer Vk ===========================

struct IndexBufferVk : public IndexBuffer {
  // ======= BEGIN: public overridden methods =================================

  virtual bool initialize(
      const std::shared_ptr<IndexStreamData>& streamData) override;

  virtual void bind(
      const std::shared_ptr<CommandBuffer>& commandBuffer) const override;

  virtual BufferVk* getBuffer() const override { return m_bufferPtr_.get(); }

  virtual int32_t getElementCount() const {
    return m_indexStreamData_ ? m_indexStreamData_->m_elementCount_ : 0;
  }

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public getters ============================================

  uint32_t getIndexCount() const { return m_indexStreamData_->m_elementCount_; }

  VkIndexType getVulkanIndexFormat(EBufferElementType IndexType) const;
  uint32_t    getVulkanIndexStride(EBufferElementType IndexType) const;

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc fields ========================================

  std::shared_ptr<BufferVk> m_bufferPtr_;

  // ======= END: public misc fields   ========================================
};

}  // namespace game_engine
#endif  // GAME_ENGINE_BUFFER_VK_H
