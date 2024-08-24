

#include "gfx/rhi/vulkan/buffer_vk.h"

#include "gfx/rhi/rhi.h"
#include "gfx/rhi/vulkan/render_frame_context_vk.h"  // TODO: redundant (consider either remove or remain for clarification what files is used)
#include "gfx/rhi/vulkan/rhi_vk.h"
#include "gfx/rhi/vulkan/utils_vk.h"

namespace game_engine {

// BufferVk
// ================================================================================

BufferVk::BufferVk(VkDeviceSize      size,
                   EVulkanBufferBits usage,
                   EVulkanMemoryBits properties,
                   EResourceLayout   imageLayout)
    : m_size_(size)
    , m_layout_(imageLayout) {
  g_createBuffer(usage, properties, size, imageLayout);
}

void BufferVk::initializeWithMemory(const Memory& memory) {
  assert(memory.isValid());
  m_hasBufferOwnership_ = false;
  m_buffer_             = (VkBuffer)memory.getBuffer();
  m_mappedPointer_      = memory.getMappedPointer();
  m_deviceMemory_       = (VkDeviceMemory)memory.getMemory();
  m_offset_             = memory.m_range.m_offset_;
  m_allocatedSize_      = memory.m_range.m_dataSize_;
  m_memory_             = memory;
}

void BufferVk::release() {
  // TODO: currently not deleted correctly

  if (!m_hasBufferOwnership_) {
    // Return an allocated memory to vulkan memory pool
    if (m_memory_.isValid()) {
      m_memory_.free();
    }

    return;
  }

  if (m_buffer_ != VK_NULL_HANDLE) {
    vkDestroyBuffer(g_rhiVk->m_device_, m_buffer_, nullptr);
  }
  if (m_deviceMemory_ != VK_NULL_HANDLE) {
    vkFreeMemory(g_rhiVk->m_device_, m_deviceMemory_, nullptr);
  }

  m_deviceMemory_ = nullptr;

  m_allocatedSize_ = 0;
  m_mappedPointer_ = nullptr;
}

void* BufferVk::map(uint64_t offset, uint64_t size) {
  if (!m_hasBufferOwnership_ || m_mappedPointer_) {
    return nullptr;
  }
  if (size > m_allocatedSize_) {
    GlobalLogger::Log(
        LogLevel::Error,
        "Failed to map buffer: Invalid buffer ownership or already mapped");
    return nullptr;
  }
  vkMapMemory(
      g_rhiVk->m_device_, m_deviceMemory_, offset, size, 0, &m_mappedPointer_);
  return m_mappedPointer_;
}

void* BufferVk::map() {
  return map(0, VK_WHOLE_SIZE);
}

void BufferVk::unmap() {
  if (!m_hasBufferOwnership_ || !m_mappedPointer_) {
    return;
  }
  vkUnmapMemory(g_rhiVk->m_device_, m_deviceMemory_);
  m_mappedPointer_ = nullptr;
}

void BufferVk::updateBuffer(const void* data, uint64_t size) {
  if (size > m_allocatedSize_) {
    GlobalLogger::Log(
        LogLevel::Error,
        "Failed to map buffer: Requested size is larger than allocated size");
    return;
  }

  // Check if the buffer is part of a larger memory management system
  // (e.g., pooled memory). This example assumes that BufferVk owns its
  // memory.
  if (m_hasBufferOwnership_) {
    void* ptr = map(0, size);
    if (ptr) {
      memcpy(ptr, data, size);
      unmap();
    } else {
      GlobalLogger::Log(LogLevel::Error, "Failed to map buffer for updating");
    }
  } else {
    // For buffers that don't own their memory, write directly to the mapped
    // memory. This assumes that void* is valid and points to the
    // correct memory region.
    if (m_mappedPointer_) {
      memcpy(static_cast<uint8_t*>(m_mappedPointer_) + m_offset_, data, size);
    } else {
      GlobalLogger::Log(LogLevel::Error,
                        "Buffer is not mapped or does not own its memory");
    }
  }
}

// VertexBufferVk
// ================================================================================

void VertexBufferVk::BindInfo::reset() {
  m_inputBindingDescriptions_.clear();
  m_attributeDescriptions_.clear();
  m_buffers_.clear();
  m_offsets_.clear();
  m_startBindingIndex_ = 0;
}

VkPipelineVertexInputStateCreateInfo
    VertexBufferVk::BindInfo::createVertexInputState() const {
  // Vertex Input
  // 1). Bindings : Spacing between data and whether it's per-virtue or
  // per-instance (when using instancing). 2). Attribute descriptions: Type
  // of attributes passed to the vertex shader. The bindings and offsets to
  // load them from
  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.sType
      = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  vertexInputInfo.vertexBindingDescriptionCount
      = (uint32_t)m_inputBindingDescriptions_.size();
  vertexInputInfo.pVertexBindingDescriptions = &m_inputBindingDescriptions_[0];
  vertexInputInfo.vertexAttributeDescriptionCount
      = (uint32_t)m_attributeDescriptions_.size();
  ;
  vertexInputInfo.pVertexAttributeDescriptions = &m_attributeDescriptions_[0];

  return vertexInputInfo;
}

void VertexBufferVk::bind(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext) const {
  assert(renderFrameContext);
  assert(renderFrameContext->getActiveCommandBuffer());
  vkCmdBindVertexBuffers(
      (VkCommandBuffer)renderFrameContext->getActiveCommandBuffer()
          ->getNativeHandle(),
      m_bindInfos_.m_startBindingIndex_,
      (uint32_t)m_bindInfos_.m_buffers_.size(),
      &m_bindInfos_.m_buffers_[0],
      &m_bindInfos_.m_offsets_[0]);
}

bool VertexBufferVk::initialize(
    const std::shared_ptr<VertexStreamData>& streamData) {
  if (!streamData) {
    return false;
  }

  m_vertexStreamData_ = streamData;
  m_bindInfos_.reset();
  m_bindInfos_.m_startBindingIndex_ = streamData->m_bindingIndex_;

  std::list<uint32_t> buffers;
  int32_t             locationIndex = streamData->m_startLocation_;
  int32_t             bindingIndex  = streamData->m_bindingIndex_;
  for (const auto& iter : streamData->m_streams_) {
    if (iter->m_stride_ <= 0) {
      continue;
    }

    VertexStreamVk stream;
    // TODO: consider use buffer type from iter
    stream.m_bufferType_ = EBufferType::Static;
    stream.m_name_       = iter->m_name_;
    stream.m_stride_     = iter->m_stride_;
    stream.m_offset_     = 0;

    if (iter->getBufferSize() > 0) {
      // TODO: old code (remove)
      // VkDeviceSize bufferSize = iter->getBufferSize();
      // BufferVk     stagingBuffer;

      //// VK_BUFFER_USAGE_TRANSFER_SRC_BIT: This buffer can be the source of
      //// memory transfer operations.
      // AllocateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      //                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
      //                    | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      //                bufferSize,
      //                stagingBuffer);

      ////// The last parameter 0 is the offset of the memory area.
      ////// If this value is not zero, it must be divided by
      ///// memRequirements.alignment. (which means it is aligned)
      //// vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);

      // stagingBuffer.updateBuffer(iter->getBufferData(), bufferSize);

      //// Map -> Unmap does not immediately reflect data in memory
      //// There are 2 ways to use it immediately
      //// 1. use VK_MEMORY_PROPERTY_HOST_COHERENT_BIT (always reflected, may be
      //// slightly slower)
      //// 2. call vkFlushMappedMemoryRanges after writes and
      //// vkInvalidateMappedMemoryRanges after reads Using the above 2 methods
      //// does not guarantee that this data will be immediately visible to the
      //// GPU, but it does guarantee that it will be completed before the next
      //// vkQueueSubmit call.

      //// VK_BUFFER_USAGE_TRANSFER_DST_BIT: This buffer can be the destination
      //// of memory transfer operations. We created a VertexBuffer in DEVICE
      //// LOCAL memory, so we can't do anything like vkMapMemory now.
      // stream.BufferPtr = std::make_shared<BufferVk>();
      // AllocateBuffer(
      //     VK_BUFFER_USAGE_TRANSFER_DST_BIT |
      //     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      //     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      //     bufferSize,
      //     *stream.BufferPtr.get());

      // g_copyBuffer(stagingBuffer, *stream.BufferPtr.get(), bufferSize);

      // stagingBuffer.release();

      // m_bindInfos.m_buffers.push_back(stream.BufferPtr->m_buffer);
      // m_bindInfos.Offsets.push_back(stream.Offset +
      // stream.BufferPtr->Offset);

      VkDeviceSize bufferSize = iter->getBufferSize();
      stream.m_bufferPtr_     = g_rhi->createStructuredBuffer<BufferVk>(
          bufferSize,
          stream.m_stride_,
          stream.m_stride_,
          EBufferCreateFlag::VertexBuffer
              | EBufferCreateFlag::AccelerationStructureBuildInput
              | EBufferCreateFlag::UAV,
          EResourceLayout::TRANSFER_DST,
          iter->getBufferData(),
          bufferSize);

      m_bindInfos_.m_buffers_.push_back(stream.m_bufferPtr_->m_buffer_);
      m_bindInfos_.m_offsets_.push_back(stream.m_offset_
                                        + stream.m_bufferPtr_->m_offset_);
    }

    /////////////////////////////////////////////////////////////
    VkVertexInputBindingDescription bindingDescription = {};
    // All data is in one array, so the binding index is 0
    bindingDescription.binding = bindingIndex;
    bindingDescription.stride  = iter->m_stride_;

    // VK_VERTEX_INPUT_RATE_VERTEX : go to next data for each vertex
    // VK_VERTEX_INPUT_RATE_INSTANCE : go to next data for each instance
    bindingDescription.inputRate
        = g_getVulkanVertexInputRate(streamData->m_vertexInputRate_);
    m_bindInfos_.m_inputBindingDescriptions_.push_back(bindingDescription);
    /////////////////////////////////////////////////////////////

    // for (const IBufferAttribute::Attribute& element : iter->Attributes) {
    //   VkVertexInputAttributeDescription attributeDescription = {};
    //   attributeDescription.binding                           = bindingIndex;
    //   attributeDescription.location                          = locationIndex;
    //   attributeDescription.format                            =
    //   element.format; attributeDescription.offset = element.offset;
    //   m_bindInfos.AttributeDescriptions.push_back(attributeDescription);

    //  ++locationIndex;
    //}

    for (IBufferAttribute::Attribute& element : iter->m_attributes_) {
      VkVertexInputAttributeDescription attributeDescription = {};
      attributeDescription.binding                           = bindingIndex;
      attributeDescription.location                          = locationIndex;

      VkFormat AttrFormat = VK_FORMAT_UNDEFINED;
      switch (element.m_underlyingType_) {
        case EBufferElementType::BYTE: {
          const int32_t elementCount = element.m_stride_ / sizeof(char);
          switch (elementCount) {
            case 1:
              AttrFormat = VK_FORMAT_R8_SINT;
              break;
            case 2:
              AttrFormat = VK_FORMAT_R8G8_SINT;
              break;
            case 3:
              AttrFormat = VK_FORMAT_R8G8B8_SINT;
              break;
            case 4:
              AttrFormat = VK_FORMAT_R8G8B8A8_SINT;
              break;
            default:
              assert(0);
              break;
          }
          break;
        }
        case EBufferElementType::BYTE_UNORM: {
          const int32_t elementCount = element.m_stride_ / sizeof(char);
          switch (elementCount) {
            case 1:
              AttrFormat = VK_FORMAT_R8_UNORM;
              break;
            case 2:
              AttrFormat = VK_FORMAT_R8G8_UNORM;
              break;
            case 3:
              AttrFormat = VK_FORMAT_R8G8B8_UNORM;
              break;
            case 4:
              AttrFormat = VK_FORMAT_R8G8B8A8_UNORM;
              break;
            default:
              assert(0);
              break;
          }
          break;
        }
        case EBufferElementType::UINT16: {
          const int32_t elementCount = element.m_stride_ / sizeof(uint16_t);
          switch (elementCount) {
            case 1:
              AttrFormat = VK_FORMAT_R16_UINT;
              break;
            case 2:
              AttrFormat = VK_FORMAT_R16G16_UINT;
              break;
            case 3:
              AttrFormat = VK_FORMAT_R16G16B16_UINT;
              break;
            case 4:
              AttrFormat = VK_FORMAT_R16G16B16A16_UINT;
              break;
            default:
              assert(0);
              break;
          }
          break;
        }
        case EBufferElementType::UINT32: {
          const int32_t elementCount = element.m_stride_ / sizeof(uint32_t);
          switch (elementCount) {
            case 1:
              AttrFormat = VK_FORMAT_R32_UINT;
              break;
            case 2:
              AttrFormat = VK_FORMAT_R32G32_UINT;
              break;
            case 3:
              AttrFormat = VK_FORMAT_R32G32B32_UINT;
              break;
            case 4:
              AttrFormat = VK_FORMAT_R32G32B32A32_UINT;
              break;
            default:
              assert(0);
              break;
          }
          break;
        }
        case EBufferElementType::FLOAT: {
          const int32_t elementCount = element.m_stride_ / sizeof(float);
          switch (elementCount) {
            case 1:
              AttrFormat = VK_FORMAT_R32_SFLOAT;
              break;
            case 2:
              AttrFormat = VK_FORMAT_R32G32_SFLOAT;
              break;
            case 3:
              AttrFormat = VK_FORMAT_R32G32B32_SFLOAT;
              break;
            case 4:
              AttrFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
              break;
            default:
              assert(0);
              break;
          }
          break;
        }
        default:
          assert(0);
          break;
      }
      assert(AttrFormat != VK_FORMAT_UNDEFINED);
      // float: VK_FORMAT_R32_SFLOAT
      // vec2 : VK_FORMAT_R32G32_SFLOAT
      // vec3 : VK_FORMAT_R32G32B32_SFLOAT
      // vec4 : VK_FORMAT_R32G32B32A32_SFLOAT
      // ivec2: VK_FORMAT_R32G32_SINT, a 2-component vector of 32-bit signed
      // integers uvec4: VK_FORMAT_R32G32B32A32_UINT, a 4-component vector of
      // 32-bit unsigned integers double: VK_FORMAT_R64_SFLOAT, a
      // double-precision (64-bit) float
      attributeDescription.format = AttrFormat;
      attributeDescription.offset = element.m_offset_;
      m_bindInfos_.m_attributeDescriptions_.push_back(attributeDescription);

      ++locationIndex;
    }

    /////////////////////////////////////////////////////////////
    m_streams_.emplace_back(stream);

    ++bindingIndex;
  }
  return true;
}

VkPipelineInputAssemblyStateCreateInfo
    VertexBufferVk::createInputAssemblyState() const {
  assert(m_vertexStreamData_ != nullptr);  // TODO: consider remove

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
  inputAssembly.sType
      = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology
      = g_getVulkanPrimitiveTopology(m_vertexStreamData_->m_primitiveType_);

  // If the primitiveRestartEnable option is VK_TRUE, line and triangle
  // topology modes can be used by using a special index 0xFFFF or 0xFFFFFFFF
  // in the index buffer.
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  return inputAssembly;
}

void VertexBufferVk::s_createVertexInputState(
    VkPipelineVertexInputStateCreateInfo&           vertexInputInfo,
    std::vector<VkVertexInputBindingDescription>&   bindingDescriptions,
    std::vector<VkVertexInputAttributeDescription>& attributeDescriptions,
    const VertexBufferArray&                        vertexBufferArray) {
  for (int32_t i = 0; i < vertexBufferArray.m_numOfData_; ++i) {
    // TODO: consider replace assertion
    assert(vertexBufferArray[i] != nullptr);
    const auto& bindInfo
        = ((const VertexBufferVk*)vertexBufferArray[i])->m_bindInfos_;
    bindingDescriptions.insert(bindingDescriptions.end(),
                                  bindInfo.m_inputBindingDescriptions_.begin(),
                                  bindInfo.m_inputBindingDescriptions_.end());
    attributeDescriptions.insert(attributeDescriptions.end(),
                                    bindInfo.m_attributeDescriptions_.begin(),
                                    bindInfo.m_attributeDescriptions_.end());
  }

  vertexInputInfo.sType
      = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount
      = (uint32_t)bindingDescriptions.size();
  vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
  vertexInputInfo.vertexAttributeDescriptionCount
      = (uint32_t)attributeDescriptions.size();
  vertexInputInfo.pVertexAttributeDescriptions
      = attributeDescriptions.data();
}

// IndexBufferVk
// ================================================================================
void IndexBufferVk::bind(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext) const {
  // TODO: old code (remove)
  // assert(indexStreamData->stream->Attributes.size() != 0);
  // VkIndexType IndexType
  //     = VK_INDEX_TYPE_UINT16;  // TODO: consider remove default enum constant
  // switch (indexStreamData->stream->Attributes[0].format) {
  //   case VK_FORMAT_R8_UINT:
  //     IndexType = VK_INDEX_TYPE_UINT8_EXT;  // TODO: check that
  //                                           // VK_EXT_index_type_uint8
  //                                           // extension is available
  //     break;
  //   case VK_FORMAT_R16_UINT:
  //     IndexType = VK_INDEX_TYPE_UINT16;
  //     break;
  //   case VK_FORMAT_R32_UINT:
  //     IndexType = VK_INDEX_TYPE_UINT32;
  //     break;
  //   default:
  //     // TODO: log error
  //     break;
  // }
  // assert(renderFrameContext);
  // assert(renderFrameContext->getActiveCommandBuffer());
  // vkCmdBindIndexBuffer(
  //     (VkCommandBuffer)renderFrameContext->getActiveCommandBuffer()
  //         ->getNativeHandle(),
  //     BufferPtr->m_buffer,
  //     BufferPtr->Offset,
  //     IndexType);

  assert(m_indexStreamData_->m_stream_->m_attributes_.size() != 0);

  assert(renderFrameContext);
  assert(renderFrameContext->getActiveCommandBuffer());
  const VkIndexType IndexType = getVulkanIndexFormat(
      m_indexStreamData_->m_stream_->m_attributes_[0].m_underlyingType_);
  vkCmdBindIndexBuffer(
      (VkCommandBuffer)renderFrameContext->getActiveCommandBuffer()
          ->getNativeHandle(),
      m_bufferPtr_->m_buffer_,
      m_bufferPtr_->m_offset_,
      IndexType);
}

VkIndexType IndexBufferVk::getVulkanIndexFormat(EBufferElementType type) const {
  VkIndexType IndexType = VK_INDEX_TYPE_UINT32;
  switch (type) {
    case EBufferElementType::BYTE:
      IndexType = VK_INDEX_TYPE_UINT8_EXT;
      break;
    case EBufferElementType::UINT16:
      IndexType = VK_INDEX_TYPE_UINT16;
      break;
    case EBufferElementType::UINT32:
      IndexType = VK_INDEX_TYPE_UINT32;
      break;
    case EBufferElementType::FLOAT:
      assert(0);
      break;
    default:
      break;
  }
  return IndexType;
}

uint32_t IndexBufferVk::getVulkanIndexStride(EBufferElementType type) const {
  switch (type) {
    case EBufferElementType::BYTE:
      return 1;
    case EBufferElementType::UINT16:
      return 2;
    case EBufferElementType::UINT32:
      return 4;
    case EBufferElementType::FLOAT:
      assert(0);
      break;
    default:
      break;
  }
  assert(0);
  return 4;
}

bool IndexBufferVk::initialize(
    const std::shared_ptr<IndexStreamData>& streamData) {
  if (!streamData) {
    return false;
  }

  m_indexStreamData_      = streamData;
  VkDeviceSize bufferSize = streamData->m_stream_->getBufferSize();

  // TODO: remove (old code)
  // BufferVk stagingBuffer;
  // AllocateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
  //               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
  //                   | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  //               bufferSize,
  //               stagingBuffer);

  // stagingBuffer.updateBuffer(streamData->stream->getBufferData(),
  // bufferSize);

  // BufferPtr = std::make_shared<BufferVk>();
  // AllocateBuffer(
  //     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
  //     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
  //     bufferSize,
  //     *BufferPtr.get());
  // g_copyBuffer(stagingBuffer, *BufferPtr.get(), bufferSize);

  // stagingBuffer.release();

  const EBufferCreateFlag BufferCreateFlag
      = EBufferCreateFlag::IndexBuffer | EBufferCreateFlag::UAV
      | EBufferCreateFlag::AccelerationStructureBuildInput;
  m_bufferPtr_ = g_rhi->createStructuredBuffer<BufferVk>(
      bufferSize,
      0,
      getVulkanIndexStride(
          m_indexStreamData_->m_stream_->m_attributes_[0].m_underlyingType_),
      BufferCreateFlag,
      EResourceLayout::TRANSFER_DST,
      streamData->m_stream_->getBufferData(),
      bufferSize
      /*, Text("IndexBuffer")*/);

  return true;
}

}  // namespace game_engine
