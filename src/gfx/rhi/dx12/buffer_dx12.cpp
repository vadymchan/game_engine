#include "gfx/rhi/dx12/buffer_dx12.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/dx12/command_list_dx12.h"
#include "gfx/rhi/rhi.h"

namespace game_engine {

void BufferDx12::release() {
  m_cbv_.free();
  m_srv_.free();
  m_uav_.free();
}

bool VertexBufferDx12::initialize(
    const std::shared_ptr<VertexStreamData>& streamData) {
  if (!streamData) {
    return false;
  }
m_vertexStreamData_ = streamData;
  m_bindInfos_.reset();
  m_bindInfos_.m_startBindingIndex_ = streamData->m_bindingIndex_;

  m_streams_.clear();
  m_streams_.reserve(streamData->m_streams_.size());

  m_VBView_.clear();
  m_VBView_.reserve(streamData->m_streams_.size());

  std::list<uint32_t> buffers;
  int32_t             locationIndex = streamData->m_startLocation_;
  int32_t             bindingIndex  = streamData->m_bindingIndex_;
  for (const auto& iter : streamData->m_streams_) {
    if (iter->m_stride_ <= 0) {
      continue;
    }

    VertexStreamDx12 stream;
    stream.m_bufferType_ = EBufferType::Static;
    stream.m_name_       = iter->m_name_;
    stream.m_stride_     = iter->m_stride_;
    stream.m_offset_     = 0;

    // Create vertex buffer
    stream.m_bufferPtr_
        = g_rhi->createRawBuffer<BufferDx12>(iter->getBufferSize(),
                                             0,
                                             EBufferCreateFlag::NONE,
                                             EResourceLayout::GENERAL,
                                             iter->getBufferData(),
                                             iter->getBufferSize()
                                             /*, Text("VertexBuffer")*/);

    BufferDx12* bufferDx12 = stream.getBuffer<BufferDx12>();

    for (IBufferAttribute::Attribute& element : iter->m_attributes_) {
      DXGI_FORMAT AttrFormat = DXGI_FORMAT_UNKNOWN;
      switch (element.m_underlyingType_) {
        case EBufferElementType::BYTE: {
          const int32_t elementCount = element.m_stride_ / sizeof(char);
          switch (elementCount) {
            case 1:
              AttrFormat = DXGI_FORMAT_R8_SINT;
              break;
            case 2:
              AttrFormat = DXGI_FORMAT_R8G8_SINT;
              break;
            case 3:
              assert(0);
              // TODO: consider how to handle this (There's no type)
              // AttrFormat = DXGI_FORMAT_R8G8B8A8_SINT;
              break;
            case 4:
              AttrFormat = DXGI_FORMAT_R8G8B8A8_SINT;
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
              AttrFormat = DXGI_FORMAT_R8_UNORM;
              break;
            case 2:
              AttrFormat = DXGI_FORMAT_R8G8_UNORM;
              break;
            case 3:
              // TODO: consider how to handle this (There's no type)
              // AttrFormat = DXGI_FORMAT_R8G8B8_UNORM;
              break;
            case 4:
              AttrFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
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
              AttrFormat = DXGI_FORMAT_R16_UINT;
              break;
            case 2:
              AttrFormat = DXGI_FORMAT_R16G16_UINT;
              break;
            case 3:
              // TODO: consider how to handle this (There's no type)
              // AttrFormat = DXGI_FORMAT_R16G16B16_UINT;
              break;
            case 4:
              AttrFormat = DXGI_FORMAT_R16G16B16A16_UINT;
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
              AttrFormat = DXGI_FORMAT_R32_UINT;
              break;
            case 2:
              AttrFormat = DXGI_FORMAT_R32G32_UINT;
              break;
            case 3:
              AttrFormat = DXGI_FORMAT_R32G32B32_UINT;
              break;
            case 4:
              AttrFormat = DXGI_FORMAT_R32G32B32A32_UINT;
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
              AttrFormat = DXGI_FORMAT_R32_FLOAT;
              break;
            case 2:
              AttrFormat = DXGI_FORMAT_R32G32_FLOAT;
              break;
            case 3:
              AttrFormat = DXGI_FORMAT_R32G32B32_FLOAT;
              break;
            case 4:
              AttrFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
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
      // TODO: use DX 12 enum const
      assert(AttrFormat != VK_FORMAT_UNDEFINED);

      if (iter->getBufferSize() > 0) {
        m_bindInfos_.m_buffers_.push_back(bufferDx12->m_buffer->get());
        m_bindInfos_.m_offsets_.push_back(stream.m_offset_
                                          + bufferDx12->m_offset_);
      }

      D3D12_INPUT_ELEMENT_DESC elem;
      elem.SemanticName
          = element.m_name_.getStringLength()
              ? element.m_name_.toStr()
              : iter->m_name_.toStr();  // TODO: remove iter-Name.toStr();
      elem.SemanticIndex     = 0;       // TODO
      elem.Format            = AttrFormat;
      elem.InputSlot         = bindingIndex;
      elem.AlignedByteOffset = element.m_offset_;
      elem.InputSlotClass
          = g_getDX12VertexInputRate(streamData->m_vertexInputRate_);
      elem.InstanceDataStepRate
          = (elem.InputSlotClass
             == D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA)
              ? 1
              : 0;
      m_bindInfos_.m_inputElementDescs_.emplace_back(elem);

      ++locationIndex;
    }

    m_streams_.emplace_back(stream);

    D3D12_VERTEX_BUFFER_VIEW view;
    view.BufferLocation = bufferDx12->getGPUAddress();
    view.SizeInBytes    = (uint32_t)bufferDx12->m_size_;
    view.StrideInBytes  = stream.m_stride_;
    m_VBView_.emplace_back(view);

    ++bindingIndex;
  }

  getHash();  // GenerateHash

  return true;
}

void VertexBufferDx12::bind(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext) const {
  auto commandBufferDx12
      = (CommandBufferDx12*)renderFrameContext->getActiveCommandBuffer();
  assert(commandBufferDx12);

  bind(commandBufferDx12);
}

void VertexBufferDx12::bind(CommandBufferDx12* commandList) const {
  assert(commandList->m_commandList_);
  commandList->m_commandList_->IASetPrimitiveTopology(getTopology());
  commandList->m_commandList_->IASetVertexBuffers(
      m_bindInfos_.m_startBindingIndex_,
      (uint32_t)m_VBView_.size(),
      &m_VBView_[0]);
}

IBuffer* VertexBufferDx12::getBuffer(int32_t streamIndex) const {
  assert(m_streams_.size() > streamIndex);
  return m_streams_[streamIndex].m_bufferPtr_.get();
}

void IndexBufferDx12::bind(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext) const {
  auto commandBufferDx12
      = (CommandBufferDx12*)renderFrameContext->getActiveCommandBuffer();
  assert(commandBufferDx12);

  bind(commandBufferDx12);
}

void IndexBufferDx12::bind(CommandBufferDx12* commandList) const {
  assert(commandList->m_commandList_);
  commandList->m_commandList_->IASetIndexBuffer(&m_IBView_);
}

bool IndexBufferDx12::initialize(
    const std::shared_ptr<IndexStreamData>& streamData) {
  if (!streamData) {
    return false;
  }

  assert(streamData);
  assert(streamData->m_stream_);
  m_indexStreamData_ = streamData;

  const size_t bufferSize = streamData->m_stream_->getBufferSize();

  DXGI_FORMAT IndexType = DXGI_FORMAT_R16_UINT;
  switch (m_indexStreamData_->m_stream_->m_attributes_[0].m_underlyingType_) {
    case EBufferElementType::BYTE:
      IndexType = DXGI_FORMAT_R8_UINT;
      break;
    case EBufferElementType::UINT16:
      IndexType = DXGI_FORMAT_R16_UINT;
      break;
    case EBufferElementType::UINT32:
      IndexType = DXGI_FORMAT_R32_UINT;
      break;
    case EBufferElementType::FLOAT:
      assert(0);
      break;
    default:
      break;
  }

  // Create index buffer
  m_bufferPtr_ = g_rhi->createFormattedBuffer<BufferDx12>(
      bufferSize,
      0,
      g_getDX12TextureFormat(IndexType),
      EBufferCreateFlag::NONE,
      EResourceLayout::GENERAL,
      streamData->m_stream_->getBufferData(),
      bufferSize
      /*,Text("IndexBuffer")*/);

  // Create Index buffer View
  m_IBView_.BufferLocation = m_bufferPtr_->getGPUAddress();
  m_IBView_.SizeInBytes    = (uint32_t)m_bufferPtr_->getAllocatedSize();
  m_IBView_.Format         = IndexType;

  return true;
}

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12