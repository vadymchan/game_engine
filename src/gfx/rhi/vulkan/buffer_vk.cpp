

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
    : m_size(size)
    , Layout(imageLayout) {
  CreateBuffer(usage, properties, size, imageLayout);
}

void BufferVk::InitializeWithMemory(const Memory& InMemory) {
  assert(InMemory.IsValid());
  HasBufferOwnership = false;
  m_buffer           = (VkBuffer)InMemory.GetBuffer();
  MappedPointer      = InMemory.GetMappedPointer();
  m_deviceMemory           = (VkDeviceMemory)InMemory.GetMemory();
  Offset             = InMemory.m_range.Offset;
  AllocatedSize      = InMemory.m_range.DataSize;
  m_memory          = InMemory;
}

void BufferVk::Release() {
  // TODO: currently not deleted correctly

  if (!HasBufferOwnership) {
    // Return an allocated memory to vulkan memory pool
    if (m_memory.IsValid()) {
      m_memory.Free();
    }

    return;
  }

  if (m_buffer != VK_NULL_HANDLE) {
    vkDestroyBuffer(g_rhi_vk->m_device_, m_buffer, nullptr);
  }
  if (m_deviceMemory != VK_NULL_HANDLE) {
    vkFreeMemory(g_rhi_vk->m_device_, m_deviceMemory, nullptr);
  }

  m_deviceMemory = nullptr;

  AllocatedSize = 0;
  MappedPointer = nullptr;
}

void* BufferVk::Map(uint64_t offset, uint64_t size) {
  if (!HasBufferOwnership || MappedPointer) {
    return nullptr;
  }
  if (size > AllocatedSize) {
    GlobalLogger::Log(
        LogLevel::Error,
        "Failed to map buffer: Invalid buffer ownership or already mapped");
    return nullptr;
  }
  vkMapMemory(g_rhi_vk->m_device_, m_deviceMemory, offset, size, 0, &MappedPointer);
  return MappedPointer;
}

void* BufferVk::Map() {
  return Map(0, VK_WHOLE_SIZE);
}

void BufferVk::Unmap() {
  if (!HasBufferOwnership || !MappedPointer) {
    return;
  }
  vkUnmapMemory(g_rhi_vk->m_device_, m_deviceMemory);
  MappedPointer = nullptr;
}

void BufferVk::UpdateBuffer(const void* data, uint64_t size) {
  if (size > AllocatedSize) {
    GlobalLogger::Log(
        LogLevel::Error,
        "Failed to map buffer: Requested size is larger than allocated size");
    return;
  }

  // Check if the buffer is part of a larger memory management system
  // (e.g., pooled memory). This example assumes that BufferVk owns its
  // memory.
  if (HasBufferOwnership) {
    void* ptr = Map(0, size);
    if (ptr) {
      memcpy(ptr, data, size);
      Unmap();
    } else {
      GlobalLogger::Log(LogLevel::Error, "Failed to map buffer for updating");
    }
  } else {
    // For buffers that don't own their memory, write directly to the mapped
    // memory. This assumes that void* is valid and points to the
    // correct memory region.
    if (MappedPointer) {
      memcpy(static_cast<uint8_t*>(MappedPointer) + Offset, data, size);
    } else {
      GlobalLogger::Log(LogLevel::Error,
                        "Buffer is not mapped or does not own its memory");
    }
  }
}

// VertexBufferVk
// ================================================================================

void VertexBufferVk::BindInfo::Reset() {
  InputBindingDescriptions.clear();
  AttributeDescriptions.clear();
  Buffers.clear();
  Offsets.clear();
  StartBindingIndex = 0;
}

VkPipelineVertexInputStateCreateInfo
    VertexBufferVk::BindInfo::CreateVertexInputState() const {
  // Vertex Input
  // 1). Bindings : Spacing between data and whether it's per-virtue or
  // per-instance (when using instancing). 2). Attribute descriptions: Type
  // of attributes passed to the vertex shader. The bindings and offsets to
  // load them from
  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.sType
      = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  vertexInputInfo.vertexBindingDescriptionCount
      = (uint32_t)InputBindingDescriptions.size();
  vertexInputInfo.pVertexBindingDescriptions = &InputBindingDescriptions[0];
  vertexInputInfo.vertexAttributeDescriptionCount
      = (uint32_t)AttributeDescriptions.size();
  ;
  vertexInputInfo.pVertexAttributeDescriptions = &AttributeDescriptions[0];

  return vertexInputInfo;
}

void VertexBufferVk::Bind(
    const std::shared_ptr<RenderFrameContext>& InRenderFrameContext) const {
  assert(InRenderFrameContext);
  assert(InRenderFrameContext->GetActiveCommandBuffer());
  vkCmdBindVertexBuffers(
      (VkCommandBuffer)InRenderFrameContext->GetActiveCommandBuffer()
          ->GetNativeHandle(),
      BindInfos.StartBindingIndex,
      (uint32_t)BindInfos.Buffers.size(),
      &BindInfos.Buffers[0],
      &BindInfos.Offsets[0]);
}

bool VertexBufferVk::Initialize(
    const std::shared_ptr<VertexStreamData>& InStreamData) {
  if (!InStreamData) {
    return false;
  }

  vertexStreamData = InStreamData;
  BindInfos.Reset();
  BindInfos.StartBindingIndex = InStreamData->bindingIndex;

  std::list<uint32_t> buffers;
  int32_t             locationIndex = InStreamData->startLocation;
  int32_t             bindingIndex  = InStreamData->bindingIndex;
  for (const auto& iter : InStreamData->streams) {
    if (iter->Stride <= 0) {
      continue;
    }

    VertexStreamVk stream;
    // TODO: consider use buffer type from iter
    stream.BufferType = EBufferType::Static;
    stream.name       = iter->name;
    stream.Stride     = iter->Stride;
    stream.Offset     = 0;

    if (iter->GetBufferSize() > 0) {
      // TODO: old code (remove)
      // VkDeviceSize bufferSize = iter->GetBufferSize();
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

      // stagingBuffer.UpdateBuffer(iter->GetBufferData(), bufferSize);

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

      // CopyBuffer(stagingBuffer, *stream.BufferPtr.get(), bufferSize);

      // stagingBuffer.Release();

      // BindInfos.Buffers.push_back(stream.BufferPtr->m_buffer);
      // BindInfos.Offsets.push_back(stream.Offset + stream.BufferPtr->Offset);

      VkDeviceSize bufferSize = iter->GetBufferSize();
      stream.BufferPtr        = g_rhi->CreateStructuredBuffer<BufferVk>(
          bufferSize,
          stream.Stride,
          stream.Stride,
          EBufferCreateFlag::VertexBuffer
              | EBufferCreateFlag::AccelerationStructureBuildInput
              | EBufferCreateFlag::UAV,
          EResourceLayout::TRANSFER_DST,
          iter->GetBufferData(),
          bufferSize);

      BindInfos.Buffers.push_back(stream.BufferPtr->m_buffer);
      BindInfos.Offsets.push_back(stream.Offset + stream.BufferPtr->Offset);
    }

    /////////////////////////////////////////////////////////////
    VkVertexInputBindingDescription bindingDescription = {};
    // All data is in one array, so the binding index is 0
    bindingDescription.binding = bindingIndex;
    bindingDescription.stride  = iter->Stride;

    // VK_VERTEX_INPUT_RATE_VERTEX : go to next data for each vertex
    // VK_VERTEX_INPUT_RATE_INSTANCE : go to next data for each instance
    bindingDescription.inputRate
        = GetVulkanVertexInputRate(InStreamData->VertexInputRate);
    BindInfos.InputBindingDescriptions.push_back(bindingDescription);
    /////////////////////////////////////////////////////////////

    // for (const IBufferAttribute::Attribute& element : iter->Attributes) {
    //   VkVertexInputAttributeDescription attributeDescription = {};
    //   attributeDescription.binding                           = bindingIndex;
    //   attributeDescription.location                          = locationIndex;
    //   attributeDescription.format                            =
    //   element.format; attributeDescription.offset = element.offset;
    //   BindInfos.AttributeDescriptions.push_back(attributeDescription);

    //  ++locationIndex;
    //}

    for (IBufferAttribute::Attribute& element : iter->Attributes) {
      VkVertexInputAttributeDescription attributeDescription = {};
      attributeDescription.binding                           = bindingIndex;
      attributeDescription.location                          = locationIndex;

      VkFormat AttrFormat = VK_FORMAT_UNDEFINED;
      switch (element.UnderlyingType) {
        case EBufferElementType::BYTE: {
          const int32_t elementCount = element.stride / sizeof(char);
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
          const int32_t elementCount = element.stride / sizeof(char);
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
          const int32_t elementCount = element.stride / sizeof(uint16_t);
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
          const int32_t elementCount = element.stride / sizeof(uint32_t);
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
          const int32_t elementCount = element.stride / sizeof(float);
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
      attributeDescription.offset = element.offset;
      BindInfos.AttributeDescriptions.push_back(attributeDescription);

      ++locationIndex;
    }

    /////////////////////////////////////////////////////////////
    Streams.emplace_back(stream);

    ++bindingIndex;
  }
  return true;
}

VkPipelineInputAssemblyStateCreateInfo
    VertexBufferVk::CreateInputAssemblyState() const {
  assert(vertexStreamData != nullptr);  // TODO: consider remove

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
  inputAssembly.sType
      = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology
      = GetVulkanPrimitiveTopology(vertexStreamData->PrimitiveType);

  // If the primitiveRestartEnable option is VK_TRUE, line and triangle
  // topology modes can be used by using a special index 0xFFFF or 0xFFFFFFFF
  // in the index buffer.
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  return inputAssembly;
}

void VertexBufferVk::CreateVertexInputState(
    VkPipelineVertexInputStateCreateInfo&           OutVertexInputInfo,
    std::vector<VkVertexInputBindingDescription>&   OutBindingDescriptions,
    std::vector<VkVertexInputAttributeDescription>& OutAttributeDescriptions,
    const VertexBufferArray&                       InVertexBufferArray) {
  for (int32_t i = 0; i < InVertexBufferArray.NumOfData; ++i) {
    // TODO: consider replace assertion
    assert(InVertexBufferArray[i] != nullptr);
    const auto& bindInfo
        = ((const VertexBufferVk*)InVertexBufferArray[i])->BindInfos;
    OutBindingDescriptions.insert(OutBindingDescriptions.end(),
                                  bindInfo.InputBindingDescriptions.begin(),
                                  bindInfo.InputBindingDescriptions.end());
    OutAttributeDescriptions.insert(OutAttributeDescriptions.end(),
                                    bindInfo.AttributeDescriptions.begin(),
                                    bindInfo.AttributeDescriptions.end());
  }

  OutVertexInputInfo.sType
      = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  OutVertexInputInfo.vertexBindingDescriptionCount
      = (uint32_t)OutBindingDescriptions.size();
  OutVertexInputInfo.pVertexBindingDescriptions = OutBindingDescriptions.data();
  OutVertexInputInfo.vertexAttributeDescriptionCount
      = (uint32_t)OutAttributeDescriptions.size();
  OutVertexInputInfo.pVertexAttributeDescriptions
      = OutAttributeDescriptions.data();
}

// IndexBufferVk
// ================================================================================
void IndexBufferVk::Bind(
    const std::shared_ptr<RenderFrameContext>& InRenderFrameContext) const {
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
  // assert(InRenderFrameContext);
  // assert(InRenderFrameContext->GetActiveCommandBuffer());
  // vkCmdBindIndexBuffer(
  //     (VkCommandBuffer)InRenderFrameContext->GetActiveCommandBuffer()
  //         ->GetNativeHandle(),
  //     BufferPtr->m_buffer,
  //     BufferPtr->Offset,
  //     IndexType);

  assert(indexStreamData->stream->Attributes.size() != 0);

  assert(InRenderFrameContext);
  assert(InRenderFrameContext->GetActiveCommandBuffer());
  const VkIndexType IndexType = GetVulkanIndexFormat(
      indexStreamData->stream->Attributes[0].UnderlyingType);
  vkCmdBindIndexBuffer(
      (VkCommandBuffer)InRenderFrameContext->GetActiveCommandBuffer()
          ->GetNativeHandle(),
      BufferPtr->m_buffer,
      BufferPtr->Offset,
      IndexType);
}

VkIndexType IndexBufferVk::GetVulkanIndexFormat(
    EBufferElementType InType) const {
  VkIndexType IndexType = VK_INDEX_TYPE_UINT32;
  switch (InType) {
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

uint32_t IndexBufferVk::GetVulkanIndexStride(EBufferElementType InType) const {
  switch (InType) {
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

bool IndexBufferVk::Initialize(
    const std::shared_ptr<IndexStreamData>& InStreamData) {
  if (!InStreamData) {
    return false;
  }

  indexStreamData         = InStreamData;
  VkDeviceSize bufferSize = InStreamData->stream->GetBufferSize();

  // TODO: remove (old code)
  // BufferVk stagingBuffer;
  // AllocateBuffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
  //               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
  //                   | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  //               bufferSize,
  //               stagingBuffer);

  // stagingBuffer.UpdateBuffer(InStreamData->stream->GetBufferData(),
  // bufferSize);

  // BufferPtr = std::make_shared<BufferVk>();
  // AllocateBuffer(
  //     VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
  //     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
  //     bufferSize,
  //     *BufferPtr.get());
  // CopyBuffer(stagingBuffer, *BufferPtr.get(), bufferSize);

  // stagingBuffer.Release();

  const EBufferCreateFlag BufferCreateFlag
      = EBufferCreateFlag::IndexBuffer | EBufferCreateFlag::UAV
      | EBufferCreateFlag::AccelerationStructureBuildInput;
  BufferPtr = g_rhi->CreateStructuredBuffer<BufferVk>(
      bufferSize,
      0,
      GetVulkanIndexStride(
          indexStreamData->stream->Attributes[0].UnderlyingType),
      BufferCreateFlag,
      EResourceLayout::TRANSFER_DST,
      InStreamData->stream->GetBufferData(),
      bufferSize
      /*, TEXT("IndexBuffer")*/);

  return true;
}

}  // namespace game_engine
