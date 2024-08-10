#include "gfx/rhi/dx12/buffer_dx12.h"
#include "gfx/rhi/dx12/command_list_dx12.h"

#include "gfx/rhi/rhi.h"

namespace game_engine {

void jBuffer_DX12::Release() {
  CBV.Free();
  SRV.Free();
  UAV.Free();
}

bool jVertexBuffer_DX12::Initialize(
    const std::shared_ptr<VertexStreamData>& InStreamData) {
  if (!InStreamData) {
    return false;
  }

  vertexStreamData = InStreamData;
  BindInfos.Reset();
  BindInfos.StartBindingIndex = InStreamData->bindingIndex;

  Streams.clear();
  Streams.reserve(InStreamData->streams.size());

  VBView.clear();
  VBView.reserve(InStreamData->streams.size());

  std::list<uint32_t> buffers;
  int32_t             locationIndex = InStreamData->startLocation;
  int32_t             bindingIndex  = InStreamData->bindingIndex;
  for (const auto& iter : InStreamData->streams) {
    if (iter->Stride <= 0) {
      continue;
    }

    jVertexStream_DX12 stream;
    stream.BufferType      = EBufferType::Static;
    stream.name            = iter->name;
    stream.Stride          = iter->Stride;
    stream.Offset          = 0;

    // Create vertex buffer
    stream.BufferPtr
        = g_rhi->CreateRawBuffer<jBuffer_DX12>(iter->GetBufferSize(),
                                               0,
                                               EBufferCreateFlag::NONE,
                                               EResourceLayout::GENERAL,
                                               iter->GetBufferData(),
                                               iter->GetBufferSize()
                                               /*, TEXT("VertexBuffer")*/);

    jBuffer_DX12* Buffer_DX12 = stream.GetBuffer<jBuffer_DX12>();

    for (IBufferAttribute::Attribute& element : iter->Attributes) {
      DXGI_FORMAT AttrFormat = DXGI_FORMAT_UNKNOWN;
      switch (element.UnderlyingType) {
        case EBufferElementType::BYTE: {
          const int32_t elementCount = element.stride / sizeof(char);
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
          const int32_t elementCount = element.stride / sizeof(char);
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
          const int32_t elementCount = element.stride / sizeof(uint16_t);
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
          const int32_t elementCount = element.stride / sizeof(uint32_t);
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
          const int32_t elementCount = element.stride / sizeof(float);
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

      if (iter->GetBufferSize() > 0) {
        BindInfos.Buffers.push_back(Buffer_DX12->Buffer->Get());
        BindInfos.Offsets.push_back(stream.Offset + Buffer_DX12->Offset);
      }

      D3D12_INPUT_ELEMENT_DESC elem;
      elem.SemanticName
          = element.name.GetStringLength()
              ? element.name.ToStr()
              : iter->name.ToStr();  // TODO: remove iter-Name.ToStr();
      elem.SemanticIndex     = 0;    // TODO
      elem.Format            = AttrFormat;
      elem.InputSlot         = bindingIndex;
      elem.AlignedByteOffset = element.offset;
      elem.InputSlotClass
          = GetDX12VertexInputRate(InStreamData->VertexInputRate);
      elem.InstanceDataStepRate
          = (elem.InputSlotClass
             == D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA)
              ? 1
              : 0;
      BindInfos.InputElementDescs.emplace_back(elem);

      ++locationIndex;
    }

    Streams.emplace_back(stream);

    D3D12_VERTEX_BUFFER_VIEW view;
    view.BufferLocation = Buffer_DX12->GetGPUAddress();
    view.SizeInBytes    = (uint32_t)Buffer_DX12->Size;
    view.StrideInBytes  = stream.Stride;
    VBView.emplace_back(view);

    ++bindingIndex;
  }

  GetHash();  // GenerateHash

  return true;
}

void jVertexBuffer_DX12::Bind(
    const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext) const {
  auto CommandBuffer_DX12
      = (jCommandBuffer_DX12*)InRenderFrameContext->GetActiveCommandBuffer();
  assert(CommandBuffer_DX12);

  Bind(CommandBuffer_DX12);
}

void jVertexBuffer_DX12::Bind(jCommandBuffer_DX12* InCommandList) const {
  assert(InCommandList->CommandList);
  InCommandList->CommandList->IASetPrimitiveTopology(GetTopology());
  InCommandList->CommandList->IASetVertexBuffers(
      BindInfos.StartBindingIndex, (uint32_t)VBView.size(), &VBView[0]);
}

jBuffer* jVertexBuffer_DX12::GetBuffer(int32_t InStreamIndex) const {
  assert(Streams.size() > InStreamIndex);
  return Streams[InStreamIndex].BufferPtr.get();
}

void jIndexBuffer_DX12::Bind(
    const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext) const {
  auto CommandBuffer_DX12
      = (jCommandBuffer_DX12*)InRenderFrameContext->GetActiveCommandBuffer();
  assert(CommandBuffer_DX12);

  Bind(CommandBuffer_DX12);
}

void jIndexBuffer_DX12::Bind(jCommandBuffer_DX12* InCommandList) const {
  assert(InCommandList->CommandList);
  InCommandList->CommandList->IASetIndexBuffer(&IBView);
}

bool jIndexBuffer_DX12::Initialize(
    const std::shared_ptr<IndexStreamData>& InStreamData) {
  if (!InStreamData) {
    return false;
  }

  assert(InStreamData);
  assert(InStreamData->stream);
  indexStreamData = InStreamData;

  const size_t bufferSize = InStreamData->stream->GetBufferSize();

  DXGI_FORMAT IndexType = DXGI_FORMAT_R16_UINT;
  switch (indexStreamData->stream->Attributes[0].UnderlyingType) {
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
  BufferPtr = g_rhi->CreateFormattedBuffer<jBuffer_DX12>(
      bufferSize,
      0,
      GetDX12TextureFormat(IndexType),
      EBufferCreateFlag::NONE,
      EResourceLayout::GENERAL,
      InStreamData->stream->GetBufferData(),
      bufferSize
      /*,TEXT("IndexBuffer")*/);

  // Create Index buffer View
  IBView.BufferLocation = BufferPtr->GetGPUAddress();
  IBView.SizeInBytes    = (uint32_t)BufferPtr->GetAllocatedSize();
  IBView.Format         = IndexType;

  return true;
}

}  // namespace game_engine