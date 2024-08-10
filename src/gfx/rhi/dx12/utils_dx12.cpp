#include "gfx/rhi/dx12/utils_dx12.h"

#include "gfx/rhi/dx12/buffer_dx12.h"
#include "gfx/rhi/dx12/command_list_dx12.h"
#include "gfx/rhi/dx12/rhi_dx12.h"
#include "gfx/rhi/rhi.h"
#include "platform/windows/windows_platform_setup.h"

namespace game_engine {

size_t BitsPerPixel_(DXGI_FORMAT fmt) {
  switch (static_cast<int>(fmt)) {
    case DXGI_FORMAT_R32G32B32A32_TYPELESS:
    case DXGI_FORMAT_R32G32B32A32_FLOAT:
    case DXGI_FORMAT_R32G32B32A32_UINT:
    case DXGI_FORMAT_R32G32B32A32_SINT:
      return 128;

    case DXGI_FORMAT_R32G32B32_TYPELESS:
    case DXGI_FORMAT_R32G32B32_FLOAT:
    case DXGI_FORMAT_R32G32B32_UINT:
    case DXGI_FORMAT_R32G32B32_SINT:
      return 96;

    case DXGI_FORMAT_R16G16B16A16_TYPELESS:
    case DXGI_FORMAT_R16G16B16A16_FLOAT:
    case DXGI_FORMAT_R16G16B16A16_UNORM:
    case DXGI_FORMAT_R16G16B16A16_UINT:
    case DXGI_FORMAT_R16G16B16A16_SNORM:
    case DXGI_FORMAT_R16G16B16A16_SINT:
    case DXGI_FORMAT_R32G32_TYPELESS:
    case DXGI_FORMAT_R32G32_FLOAT:
    case DXGI_FORMAT_R32G32_UINT:
    case DXGI_FORMAT_R32G32_SINT:
    case DXGI_FORMAT_R32G8X24_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
    case DXGI_FORMAT_Y416:
    case DXGI_FORMAT_Y210:
    case DXGI_FORMAT_Y216:
      return 64;

    case DXGI_FORMAT_R10G10B10A2_TYPELESS:
    case DXGI_FORMAT_R10G10B10A2_UNORM:
    case DXGI_FORMAT_R10G10B10A2_UINT:
    case DXGI_FORMAT_R11G11B10_FLOAT:
    case DXGI_FORMAT_R8G8B8A8_TYPELESS:
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
    case DXGI_FORMAT_R8G8B8A8_UINT:
    case DXGI_FORMAT_R8G8B8A8_SNORM:
    case DXGI_FORMAT_R8G8B8A8_SINT:
    case DXGI_FORMAT_R16G16_TYPELESS:
    case DXGI_FORMAT_R16G16_FLOAT:
    case DXGI_FORMAT_R16G16_UNORM:
    case DXGI_FORMAT_R16G16_UINT:
    case DXGI_FORMAT_R16G16_SNORM:
    case DXGI_FORMAT_R16G16_SINT:
    case DXGI_FORMAT_R32_TYPELESS:
    case DXGI_FORMAT_D32_FLOAT:
    case DXGI_FORMAT_R32_FLOAT:
    case DXGI_FORMAT_R32_UINT:
    case DXGI_FORMAT_R32_SINT:
    case DXGI_FORMAT_R24G8_TYPELESS:
    case DXGI_FORMAT_D24_UNORM_S8_UINT:
    case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
    case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
    case DXGI_FORMAT_R8G8_B8G8_UNORM:
    case DXGI_FORMAT_G8R8_G8B8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8X8_UNORM:
    case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
    case DXGI_FORMAT_B8G8R8A8_TYPELESS:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case DXGI_FORMAT_B8G8R8X8_TYPELESS:
    case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
    case DXGI_FORMAT_AYUV:
    case DXGI_FORMAT_Y410:
    case DXGI_FORMAT_YUY2:
      // case XBOX_DXGI_FORMAT_R10G10B10_7E3_A2_FLOAT:
      // case XBOX_DXGI_FORMAT_R10G10B10_6E4_A2_FLOAT:
      // case XBOX_DXGI_FORMAT_R10G10B10_SNORM_A2_UNORM:
      return 32;

    case DXGI_FORMAT_P010:
    case DXGI_FORMAT_P016:
      // case XBOX_DXGI_FORMAT_D16_UNORM_S8_UINT:
      // case XBOX_DXGI_FORMAT_R16_UNORM_X8_TYPELESS:
      // case XBOX_DXGI_FORMAT_X16_TYPELESS_G8_UINT:
      // case WIN10_DXGI_FORMAT_V408:
      return 24;

    case DXGI_FORMAT_R8G8_TYPELESS:
    case DXGI_FORMAT_R8G8_UNORM:
    case DXGI_FORMAT_R8G8_UINT:
    case DXGI_FORMAT_R8G8_SNORM:
    case DXGI_FORMAT_R8G8_SINT:
    case DXGI_FORMAT_R16_TYPELESS:
    case DXGI_FORMAT_R16_FLOAT:
    case DXGI_FORMAT_D16_UNORM:
    case DXGI_FORMAT_R16_UNORM:
    case DXGI_FORMAT_R16_UINT:
    case DXGI_FORMAT_R16_SNORM:
    case DXGI_FORMAT_R16_SINT:
    case DXGI_FORMAT_B5G6R5_UNORM:
    case DXGI_FORMAT_B5G5R5A1_UNORM:
    case DXGI_FORMAT_A8P8:
    case DXGI_FORMAT_B4G4R4A4_UNORM:
      // case WIN10_DXGI_FORMAT_P208:
      // case WIN10_DXGI_FORMAT_V208:
      return 16;

    case DXGI_FORMAT_NV12:
    case DXGI_FORMAT_420_OPAQUE:
    case DXGI_FORMAT_NV11:
      return 12;

    case DXGI_FORMAT_R8_TYPELESS:
    case DXGI_FORMAT_R8_UNORM:
    case DXGI_FORMAT_R8_UINT:
    case DXGI_FORMAT_R8_SNORM:
    case DXGI_FORMAT_R8_SINT:
    case DXGI_FORMAT_A8_UNORM:
    case DXGI_FORMAT_AI44:
    case DXGI_FORMAT_IA44:
    case DXGI_FORMAT_P8:
      // case XBOX_DXGI_FORMAT_R4G4_UNORM:
      return 8;

    case DXGI_FORMAT_R1_UNORM:
      return 1;

    case DXGI_FORMAT_BC1_TYPELESS:
    case DXGI_FORMAT_BC1_UNORM:
    case DXGI_FORMAT_BC1_UNORM_SRGB:
    case DXGI_FORMAT_BC4_TYPELESS:
    case DXGI_FORMAT_BC4_UNORM:
    case DXGI_FORMAT_BC4_SNORM:
      return 4;

    case DXGI_FORMAT_BC2_TYPELESS:
    case DXGI_FORMAT_BC2_UNORM:
    case DXGI_FORMAT_BC2_UNORM_SRGB:
    case DXGI_FORMAT_BC3_TYPELESS:
    case DXGI_FORMAT_BC3_UNORM:
    case DXGI_FORMAT_BC3_UNORM_SRGB:
    case DXGI_FORMAT_BC5_TYPELESS:
    case DXGI_FORMAT_BC5_UNORM:
    case DXGI_FORMAT_BC5_SNORM:
    case DXGI_FORMAT_BC6H_TYPELESS:
    case DXGI_FORMAT_BC6H_UF16:
    case DXGI_FORMAT_BC6H_SF16:
    case DXGI_FORMAT_BC7_TYPELESS:
    case DXGI_FORMAT_BC7_UNORM:
    case DXGI_FORMAT_BC7_UNORM_SRGB:
      return 8;

    default:
      return 0;
  }
}

size_t BytesPerPixel_(DXGI_FORMAT fmt) {
  return BitsPerPixel_(fmt) / 8;
}

const D3D12_HEAP_PROPERTIES& GetUploadHeap() {
  static D3D12_HEAP_PROPERTIES HeapProp{
    .Type                 = D3D12_HEAP_TYPE_UPLOAD,
    .CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
    .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
    .CreationNodeMask     = 0,
    .VisibleNodeMask      = 0};
  return HeapProp;
}

const D3D12_HEAP_PROPERTIES& GetDefaultHeap() {
  static D3D12_HEAP_PROPERTIES HeapProp{
    .Type                 = D3D12_HEAP_TYPE_DEFAULT,
    .CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
    .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
    .CreationNodeMask     = 0,
    .VisibleNodeMask      = 0};
  return HeapProp;
}

const D3D12_RESOURCE_DESC& GetUploadResourceDesc(uint64_t InSize) {
  static D3D12_RESOURCE_DESC Desc{
    .Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER,
    .Alignment        = 0,
    .Width            = InSize,
    .Height           = 1,
    .DepthOrArraySize = 1,
    .MipLevels        = 1,
    .Format           = DXGI_FORMAT_UNKNOWN,
    .SampleDesc       = {.Count = 1, .Quality = 0},
    .Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
    .Flags            = D3D12_RESOURCE_FLAG_NONE,
  };

  return Desc;
}

ComPtr<ID3D12Resource> CreateStagingBuffer(const void* InInitData,
                                           int64_t     InSize,
                                           uint64_t    InAlignment) {
  const uint64_t AlignedSize = Align(InSize, InAlignment);

  ComPtr<ID3D12Resource> UploadResourceRHI;
  g_rhi_dx12->Device->CreateCommittedResource(
      &GetUploadHeap(),
      D3D12_HEAP_FLAG_NONE,
      &GetUploadResourceDesc(AlignedSize),
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&UploadResourceRHI));

  void*       MappedPointer = nullptr;
  D3D12_RANGE Range         = {};
  UploadResourceRHI->Map(0, &Range, reinterpret_cast<void**>(&MappedPointer));
  memcpy(MappedPointer, InInitData, InSize);
  UploadResourceRHI->Unmap(0, &Range);
  return UploadResourceRHI;
}



void UploadByUsingStagingBuffer(ComPtr<ID3D12Resource>& DestBuffer,
                                const void*             InInitData,
                                uint64_t                InSize,
                                uint64_t                InAlignment) {
  assert(DestBuffer);

  const uint64_t AlignedSize = Align(InSize, InAlignment);
  assert(DestBuffer->GetDesc().Width >= AlignedSize);

  ComPtr<ID3D12Resource> StagingBuffer
      = CreateStagingBuffer(InInitData, InSize, InAlignment);
  jCommandBuffer_DX12* commandBuffer
      = g_rhi_dx12->BeginSingleTimeCopyCommands();
  assert(commandBuffer->IsValid());
  commandBuffer->Get()->CopyBufferRegion(
      DestBuffer.Get(), 0, StagingBuffer.Get(), 0, AlignedSize);
  g_rhi_dx12->EndSingleTimeCopyCommands(commandBuffer);
}

D3D12_RESOURCE_DESC GetDefaultResourceDesc(uint64_t InAlignedSize,
                                           bool     InIsAllowUAV) {
  static D3D12_RESOURCE_DESC Desc{
    .Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER,
    .Alignment        = 0,
    .Width            = InAlignedSize,
    .Height           = 1,
    .DepthOrArraySize = 1,
    .MipLevels        = 1,
    .Format           = DXGI_FORMAT_UNKNOWN,
    .SampleDesc       = {.Count = 1, .Quality = 0},
    .Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
    .Flags = InIsAllowUAV ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
                          : D3D12_RESOURCE_FLAG_NONE,
  };

  return Desc;
}

ComPtr<ID3D12Resource> CreateDefaultResource(
    uint64_t              InAlignedSize,
    D3D12_RESOURCE_STATES InInitialState,
    bool                  InIsAllowUAV,
    bool                  InIsCPUAccessible,
    const wchar_t*        InName) {
  const D3D12_RESOURCE_DESC Desc = GetDefaultResourceDesc(InAlignedSize, false);
  const D3D12_HEAP_PROPERTIES HeapProperties
      = InIsCPUAccessible ? GetUploadHeap() : GetDefaultHeap();

  D3D12_RESOURCE_STATES ResourceState = InInitialState;
  if (InIsCPUAccessible) {
    ResourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
  }

  ComPtr<ID3D12Resource> NewResourceRHI;
  if (SUCCEEDED(g_rhi_dx12->Device->CreateCommittedResource(
          &HeapProperties,
          D3D12_HEAP_FLAG_NONE,
          &Desc,
          ResourceState,
          nullptr,
          IID_PPV_ARGS(&NewResourceRHI)))) {
    if (InName) {
      NewResourceRHI->SetName(InName);
    }
  }

  return NewResourceRHI;
}

void* CopyInitialData(ComPtr<ID3D12Resource>& InDest,
                      const void*             InInitData,
                      uint64_t                InSize,
                      uint64_t                InAlignment,
                      bool                    InIsCPUAccessible) {
  if (InDest) {
    if (InIsCPUAccessible) {
      void*       MappedPointer = nullptr;
      D3D12_RANGE Range         = {};
      InDest->Map(0, &Range, reinterpret_cast<void**>(&MappedPointer));
      assert(MappedPointer);

      if (InInitData && MappedPointer) {
        memcpy(MappedPointer, InInitData, InSize);
      }
      return MappedPointer;
    } else {
      if (InInitData) {
        UploadByUsingStagingBuffer(InDest, InInitData, InSize, InAlignment);
      }
    }
  }
  return nullptr;
}

std::shared_ptr<jCreatedResource> CreateBufferInternal(
    uint64_t              InSize,
    uint64_t              InAlignment,
    EBufferCreateFlag     InBufferCreateFlag,
    D3D12_RESOURCE_STATES InInitialResourceState,
    const wchar_t*        InResourceName) {
  if (!!(InBufferCreateFlag & EBufferCreateFlag::AccelerationStructure)) {
    assert(InInitialResourceState
           == D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
  } else if (!!(InBufferCreateFlag & EBufferCreateFlag::Readback)) {
    assert(InInitialResourceState == D3D12_RESOURCE_STATE_COPY_DEST);
  } else if (!!(InBufferCreateFlag & EBufferCreateFlag::CPUAccess)) {
    assert(InInitialResourceState == D3D12_RESOURCE_STATE_GENERIC_READ);
  } else {
    assert(InInitialResourceState == D3D12_RESOURCE_STATE_COMMON);
  }

  InSize = (InAlignment > 0) ? Align(InSize, InAlignment) : InSize;
  if (jRHI_DX12::GIsUsePlacedResource) {
    InSize = Align(InSize, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
  }

  D3D12_RESOURCE_DESC resourceDesc = {};
  resourceDesc.Dimension           = D3D12_RESOURCE_DIMENSION_BUFFER;
  resourceDesc.Width               = InSize;
  resourceDesc.Height              = 1;
  resourceDesc.DepthOrArraySize    = 1;
  resourceDesc.MipLevels           = 1;
  resourceDesc.Format              = DXGI_FORMAT_UNKNOWN;
  resourceDesc.Flags = !!(InBufferCreateFlag & EBufferCreateFlag::UAV)
                         ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
                         : D3D12_RESOURCE_FLAG_NONE;
  resourceDesc.SampleDesc.Count   = 1;
  resourceDesc.SampleDesc.Quality = 0;
  resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  resourceDesc.Alignment          = 0;

  assert(g_rhi_dx12);

  std::shared_ptr<jCreatedResource> CreatedResource;
  if (!!(InBufferCreateFlag & EBufferCreateFlag::Readback)) {
    assert(EBufferCreateFlag::NONE
           == (InBufferCreateFlag
               & EBufferCreateFlag::UAV));  // Not allowed Readback with UAV

    ComPtr<ID3D12Resource>         NewResource;
    const CD3DX12_HEAP_PROPERTIES& HeapProperties
        = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
    HRESULT hr = g_rhi_dx12->Device->CreateCommittedResource(
        &HeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        InInitialResourceState,
        nullptr,
        IID_PPV_ARGS(&NewResource));

    assert(SUCCEEDED(hr));

    CreatedResource = jCreatedResource::CreatedFromStandalone(NewResource);
  } else if (!!(InBufferCreateFlag & EBufferCreateFlag::CPUAccess)) {
    assert(EBufferCreateFlag::NONE
           == (InBufferCreateFlag
               & EBufferCreateFlag::UAV));  // Not allowed Readback with UAV
    CreatedResource = g_rhi_dx12->CreateUploadResource(&resourceDesc,
                                                       InInitialResourceState);
  } else {
    CreatedResource
        = g_rhi_dx12->CreateResource(&resourceDesc, InInitialResourceState);
  }

  assert(CreatedResource->Resource);

  if (InResourceName && CreatedResource->Resource) {
    CreatedResource->Resource.get()->Get()->SetName(InResourceName);
  }

  return CreatedResource;
}

std::shared_ptr<jBuffer_DX12> CreateBuffer(uint64_t          InSize,
                                           uint64_t          InAlignment,
                                           EBufferCreateFlag InBufferCreateFlag,
                                           EResourceLayout   InLayout,
                                           const void*       InData,
                                           uint64_t          InDataSize,
                                           const wchar_t*    InResourceName) {
  // If the resource needed to be created with
  // EBufferCreateFlag::AccelerationStructure, you must initialize the buffer
  // resource state as ACCELERATION_STRUCTURE state.
  assert(InLayout != EResourceLayout::ACCELERATION_STRUCTURE
         || (InLayout == EResourceLayout::ACCELERATION_STRUCTURE
             && !!(InBufferCreateFlag
                   & EBufferCreateFlag::AccelerationStructure)));

  EResourceLayout       InitialLayout = EResourceLayout::UNDEFINED;
  D3D12_RESOURCE_STATES InitialLayout_DX12
      = GetDX12ResourceLayout(InitialLayout);
  if (!!(InBufferCreateFlag & EBufferCreateFlag::AccelerationStructure)) {
    assert(InLayout == EResourceLayout::ACCELERATION_STRUCTURE);
    InitialLayout_DX12 = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
    InitialLayout      = EResourceLayout::ACCELERATION_STRUCTURE;
  } else if (!!(InBufferCreateFlag & EBufferCreateFlag::Readback)) {
    InitialLayout_DX12 = D3D12_RESOURCE_STATE_COPY_DEST;
    InitialLayout      = EResourceLayout::TRANSFER_DST;
  } else if (!!(InBufferCreateFlag & EBufferCreateFlag::CPUAccess)) {
    InitialLayout_DX12 = D3D12_RESOURCE_STATE_GENERIC_READ;
    InitialLayout      = EResourceLayout::READ_ONLY;
  }

  std::shared_ptr<jCreatedResource> BufferInternal
      = CreateBufferInternal(InSize,
                             InAlignment,
                             InBufferCreateFlag,
                             InitialLayout_DX12,
                             InResourceName);
  if (!BufferInternal->Resource) {
    return nullptr;
  }

  auto BufferPtr = std::make_shared<jBuffer_DX12>(
      BufferInternal, InSize, InAlignment, InBufferCreateFlag);
  BufferPtr->Layout = InitialLayout;
  if (InResourceName) {
    // https://learn.microsoft.com/ko-kr/cpp/text/how-to-convert-between-various-string-types?view=msvc-170#example-convert-from-char-
    char         szResourceName[1024];
    size_t       OutLength = 0;
    size_t       origsize  = wcslen(InResourceName) + 1;
    const size_t newsize   = origsize * 2;
    wcstombs_s(&OutLength, szResourceName, newsize, InResourceName, _TRUNCATE);
    BufferPtr->ResourceName = Name(szResourceName);
  }

  const bool HasInitialData = InData && (InDataSize > 0);
  if (HasInitialData) {
    if (!!(InBufferCreateFlag & EBufferCreateFlag::Readback)) {
      // nothing todo
    } else if (!!(InBufferCreateFlag & EBufferCreateFlag::CPUAccess)) {
      void* CPUAddress = BufferPtr->Map();
      assert(CPUAddress);
      memcpy(CPUAddress, InData, InDataSize);
    } else {
      std::shared_ptr<jCreatedResource> StagingBuffer = CreateBufferInternal(
          InSize,
          InAlignment,
          EBufferCreateFlag::CPUAccess,
          D3D12_RESOURCE_STATE_GENERIC_READ,
          InResourceName);  // CPU Access should be created with
                            // 'D3D12_RESOURCE_STATE_GENERIC_READ'.
      assert(StagingBuffer->IsValid());

      void*       MappedPointer = nullptr;
      D3D12_RANGE Range         = {};
      HRESULT     hr            = StagingBuffer->Resource.get()->Get()->Map(
          0, &Range, &MappedPointer);
      assert(SUCCEEDED(hr));

      if (SUCCEEDED(hr)) {
        assert(MappedPointer);
        memcpy(MappedPointer, InData, InDataSize);
        StagingBuffer->Resource.get()->Get()->Unmap(0, nullptr);
      }

      jCommandBuffer_DX12* commandBuffer
          = g_rhi_dx12->BeginSingleTimeCopyCommands();
      assert(commandBuffer->IsValid());

      commandBuffer->Get()->CopyBufferRegion(
          (ID3D12Resource*)BufferPtr->GetHandle(),
          0,
          StagingBuffer->Get(),
          0,
          InSize);
      g_rhi_dx12->EndSingleTimeCopyCommands(commandBuffer);
    }
  }

  if (BufferPtr->Layout != InLayout) {
    g_rhi->TransitionLayoutImmediate(BufferPtr.get(), InLayout);
  }

  return BufferPtr;
}

std::shared_ptr<jCreatedResource> CreateTexturenternal(
    uint32_t                 InWidth,
    uint32_t                 InHeight,
    uint32_t                 InArrayLayers,
    uint32_t                 InMipLevels,
    uint32_t                 InNumOfSample,
    D3D12_RESOURCE_DIMENSION InType,
    DXGI_FORMAT              InFormat,
    ETextureCreateFlag       InTextureCreateFlag,
    EResourceLayout          InImageLayout,
    D3D12_CLEAR_VALUE*       InClearValue,
    const wchar_t*           InResourceName) {
  assert(g_rhi_dx12);
  assert(g_rhi_dx12->Device);

  D3D12_RESOURCE_DESC TexDesc = {};
  TexDesc.MipLevels           = InMipLevels;
  if (IsDepthFormat(GetDX12TextureFormat(InFormat))) {
    DXGI_FORMAT TexFormat, SrvFormat;
    GetDepthFormatForSRV(TexFormat, SrvFormat, InFormat);
    TexDesc.Format = TexFormat;
  } else {
    TexDesc.Format = InFormat;
  }
  TexDesc.Width  = InWidth;
  TexDesc.Height = InHeight;
  TexDesc.Flags  = D3D12_RESOURCE_FLAG_NONE;

  if (!!(InTextureCreateFlag & ETextureCreateFlag::RTV)) {
    TexDesc.Flags = IsDepthFormat(GetDX12TextureFormat(InFormat))
                      ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
                      : D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
  }
  if (!!(InTextureCreateFlag & ETextureCreateFlag::UAV)) {
    TexDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
  }
  TexDesc.DepthOrArraySize = InArrayLayers;

  const uint32_t StandardMSAAPattern = 0xFF'FF'FF'FF;
  TexDesc.SampleDesc.Count           = InNumOfSample;
  TexDesc.SampleDesc.Quality = InNumOfSample > 1 ? StandardMSAAPattern : 0;

  TexDesc.Dimension = InType;
  TexDesc.Layout    = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  TexDesc.Alignment = 0;

  std::shared_ptr<jCreatedResource> ImageResource = g_rhi_dx12->CreateResource(
      &TexDesc, GetDX12ResourceLayout(InImageLayout), InClearValue);
  assert(ImageResource->Resource);

  if (InResourceName && ImageResource->Resource) {
    ImageResource->Resource.get()->Get()->SetName(InResourceName);
  }

  return ImageResource;
}

std::shared_ptr<jTexture_DX12> CreateTexture(
    uint32_t             InWidth,
    uint32_t             InHeight,
    uint32_t             InArrayLayers,
    uint32_t             InMipLevels,
    uint32_t             InNumOfSample,
    ETextureType         InType,
    ETextureFormat       InFormat,
    ETextureCreateFlag   InTextureCreateFlag,
    EResourceLayout      InImageLayout,
    const jRTClearValue& InClearValue,
    const wchar_t*       InResourceName) {
  bool              HasClearValue = false;
  D3D12_CLEAR_VALUE ClearValue{};
  if (!!(InTextureCreateFlag & ETextureCreateFlag::RTV)) {
    if (InClearValue.GetType() == ERTClearType::Color) {
      ClearValue.Color[0] = InClearValue.GetCleraColor()[0];
      ClearValue.Color[1] = InClearValue.GetCleraColor()[1];
      ClearValue.Color[2] = InClearValue.GetCleraColor()[2];
      ClearValue.Color[3] = InClearValue.GetCleraColor()[3];
      ClearValue.Format   = GetDX12TextureFormat(InFormat);
    } else if (InClearValue.GetType() == ERTClearType::DepthStencil) {
      ClearValue.DepthStencil.Depth   = InClearValue.GetCleraDepth();
      ClearValue.DepthStencil.Stencil = InClearValue.GetCleraStencil();
      ClearValue.Format               = GetDX12TextureFormat(InFormat);
    }
    HasClearValue = InClearValue.GetType() != ERTClearType::None;
  }

  std::shared_ptr<jCreatedResource> TextureInternal
      = CreateTexturenternal(InWidth,
                             InHeight,
                             InArrayLayers,
                             InMipLevels,
                             InNumOfSample,
                             GetDX12TextureDemension(InType),
                             GetDX12TextureFormat(InFormat),
                             InTextureCreateFlag,
                             InImageLayout,
                             (HasClearValue ? &ClearValue : nullptr),
                             InResourceName);
  assert(TextureInternal->IsValid());

  auto TexturePtr = std::make_shared<jTexture_DX12>(
      InType,
      InFormat,
      // TODO: remove casting
      math::Dimension2Di{static_cast<int>(InWidth), static_cast<int>(InHeight)},
      InArrayLayers,
      EMSAASamples::COUNT_1,
      false,
      InClearValue,
      TextureInternal);
  assert(TexturePtr);
  // TODO: hotfix (remove mip level assigning here)
  TexturePtr->mipLevels = InMipLevels;
  TexturePtr->Layout = InImageLayout;

  // TODO: general - why do we need ResourceName?
  if (InResourceName) {
    // https://learn.microsoft.com/ko-kr/cpp/text/how-to-convert-between-various-string-types?view=msvc-170#example-convert-from-char-
    char         szResourceName[1024];
    size_t       OutLength = 0;
    size_t       origsize  = wcslen(InResourceName) + 1;
    const size_t newsize   = origsize * 2;
    wcstombs_s(&OutLength, szResourceName, newsize, InResourceName, _TRUNCATE);

    TexturePtr->ResourceName = Name(szResourceName);
  }

  if (IsDepthFormat(InFormat)) {
    CreateShaderResourceView(TexturePtr.get());
    CreateDepthStencilView(TexturePtr.get());
  } else {
    CreateShaderResourceView(TexturePtr.get());
    if (!!(InTextureCreateFlag & ETextureCreateFlag::RTV)) {
      CreateRenderTargetView(TexturePtr.get());
    }
    if (!!(InTextureCreateFlag & ETextureCreateFlag::UAV)) {
      CreateUnorderedAccessView(TexturePtr.get());
    }
  }

  return TexturePtr;
}

std::shared_ptr<jTexture_DX12> CreateTexture(
    const std::shared_ptr<jCreatedResource>& InTexture,
    ETextureCreateFlag                       InTextureCreateFlag,
    EResourceLayout                          InImageLayout,
    const jRTClearValue&                     InClearValue,
    const wchar_t*                           InResourceName) {
  const auto desc       = InTexture->Resource.get()->Get()->GetDesc();
  auto       TexturePtr = std::make_shared<jTexture_DX12>(
      GetDX12TextureDemension(desc.Dimension, desc.DepthOrArraySize > 1),
      GetDX12TextureFormat(desc.Format),
      // TODO: remove casting
      math::Dimension2Di{static_cast<int>(desc.Width),
                         static_cast<int>(desc.Height)},
      (int32_t)desc.DepthOrArraySize,
      EMSAASamples::COUNT_1,
      false,
      InClearValue,
      InTexture);

  assert(TexturePtr);
  TexturePtr->Layout = InImageLayout;

  if (InResourceName) {
    // https://learn.microsoft.com/ko-kr/cpp/text/how-to-convert-between-various-string-types?view=msvc-170#example-convert-from-char-
    char         szResourceName[1024];
    size_t       OutLength = 0;
    size_t       origsize  = wcslen(InResourceName) + 1;
    const size_t newsize   = origsize * 2;
    wcstombs_s(&OutLength, szResourceName, newsize, InResourceName, _TRUNCATE);

    TexturePtr->ResourceName = Name(szResourceName);
  }

  if (IsDepthFormat(GetDX12TextureFormat(desc.Format))) {
    CreateShaderResourceView(TexturePtr.get());
    CreateDepthStencilView(TexturePtr.get());
  } else {
    CreateShaderResourceView(TexturePtr.get());
    if (!!(InTextureCreateFlag & ETextureCreateFlag::RTV)) {
      CreateRenderTargetView(TexturePtr.get());
    }
    if (!!(InTextureCreateFlag & ETextureCreateFlag::UAV)) {
      CreateUnorderedAccessView(TexturePtr.get());
    }
  }

  return TexturePtr;
}

uint64_t CopyBufferToTexture(ID3D12GraphicsCommandList4* InCommandBuffer,
                             ID3D12Resource*             InBuffer,
                             uint64_t                    InBufferOffset,
                             ID3D12Resource*             InImage,
                             int32_t InImageSubresourceIndex) {
  assert(InCommandBuffer);

  const auto                         imageDesc         = InImage->GetDesc();
  D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout            = {};
  uint32_t                           numRow            = 0;
  uint64_t                           rowSize           = 0;
  uint64_t                           textureMemorySize = 0;
  assert(g_rhi_dx12);
  assert(g_rhi_dx12->Device);
  g_rhi_dx12->Device->GetCopyableFootprints(
      &imageDesc, 0, 1, 0, &layout, &numRow, &rowSize, &textureMemorySize);

  D3D12_TEXTURE_COPY_LOCATION dst = {};
  dst.pResource                   = InImage;
  dst.Type                        = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  dst.SubresourceIndex            = InImageSubresourceIndex;

  D3D12_TEXTURE_COPY_LOCATION src = {};
  src.pResource                   = InBuffer;
  src.Type                        = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
  src.PlacedFootprint             = layout;
  src.PlacedFootprint.Offset      = InBufferOffset;
  InCommandBuffer->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

  return textureMemorySize;
}

uint64_t CopyBufferToTexture(ID3D12GraphicsCommandList4* InCommandBuffer,
                             ID3D12Resource*             InBuffer,
                             uint64_t                    InBufferOffset,
                             ID3D12Resource*             InImage,
                             int32_t InNumOfImageSubresource,
                             int32_t InStartImageSubresource) {
  for (int32_t i = 0; i < InNumOfImageSubresource; ++i) {
    InBufferOffset += CopyBufferToTexture(
        InCommandBuffer, InBuffer, InBufferOffset, InImage, i);
  }
  return InBufferOffset;  // total size of copy data
}

void CopyBufferToTexture(
    ID3D12GraphicsCommandList4*              InCommandBuffer,
    ID3D12Resource*                          InBuffer,
    ID3D12Resource*                          InImage,
    const std::vector<ImageSubResourceData>& InSubresourceData) {
  for (uint64_t i = 0; i < InSubresourceData.size(); ++i) {
    D3D12_TEXTURE_COPY_LOCATION dst = {};
    dst.pResource                   = InImage;
    dst.Type                        = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.SubresourceIndex            = uint32_t(i);
    D3D12_TEXTURE_COPY_LOCATION src = {};
    src.pResource                   = InBuffer;
    src.Type                        = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    src.PlacedFootprint.Footprint.Format
        = (DXGI_FORMAT)InSubresourceData[i].Format;
    src.PlacedFootprint.Footprint.Width    = InSubresourceData[i].Width;
    src.PlacedFootprint.Footprint.Height   = InSubresourceData[i].Height;
    src.PlacedFootprint.Footprint.Depth    = InSubresourceData[i].Depth;
    src.PlacedFootprint.Footprint.RowPitch = InSubresourceData[i].RowPitch;
    src.PlacedFootprint.Offset             = InSubresourceData[i].Offset;
    InCommandBuffer->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
  }
}

void CopyBuffer(ID3D12GraphicsCommandList4* InCommandBuffer,
                ID3D12Resource*             InSrcBuffer,
                ID3D12Resource*             InDstBuffer,
                uint64_t                    InSize,
                uint64_t                    InSrcOffset,
                uint64_t                    InDstOffset) {
  assert(InCommandBuffer);
  assert(InSrcBuffer);
  assert(InDstBuffer);

  InCommandBuffer->CopyBufferRegion(
      InDstBuffer, InDstOffset, InSrcBuffer, InSrcOffset, InSize);
}

void CopyBuffer(ID3D12Resource* InSrcBuffer,
                ID3D12Resource* InDstBuffer,
                uint64_t        InSize,
                uint64_t        InSrcOffset,
                uint64_t        InDstOffset) {
  jCommandBuffer_DX12* commandBuffer
      = g_rhi_dx12->BeginSingleTimeCopyCommands();
  CopyBuffer(commandBuffer->Get(),
             InSrcBuffer,
             InDstBuffer,
             InSize,
             InSrcOffset,
             InDstOffset);
  g_rhi_dx12->EndSingleTimeCopyCommands(commandBuffer);
}

void CreateConstantBufferView(jBuffer_DX12* InBuffer) {
  assert(g_rhi_dx12);
  assert(g_rhi_dx12->Device);

  assert(InBuffer);
  if (!InBuffer) {
    return;
  }

  assert(!InBuffer->CBV.IsValid());
  InBuffer->CBV = g_rhi_dx12->DescriptorHeaps.Alloc();

  D3D12_CONSTANT_BUFFER_VIEW_DESC Desc{};
  Desc.BufferLocation = InBuffer->GetGPUAddress();
  Desc.SizeInBytes    = (uint32_t)InBuffer->GetAllocatedSize();

  g_rhi_dx12->Device->CreateConstantBufferView(&Desc, InBuffer->CBV.CPUHandle);
}

void CreateShaderResourceView_StructuredBuffer(jBuffer_DX12* InBuffer,
                                               uint32_t      InStride,
                                               uint32_t      InCount) {
  assert(g_rhi_dx12);
  assert(g_rhi_dx12->Device);

  assert(InBuffer);
  if (!InBuffer) {
    return;
  }

  assert(!InBuffer->SRV.IsValid());
  InBuffer->SRV = g_rhi_dx12->DescriptorHeaps.Alloc();

  D3D12_SHADER_RESOURCE_VIEW_DESC Desc{};
  Desc.Format                     = DXGI_FORMAT_UNKNOWN;
  Desc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
  Desc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  Desc.Buffer.FirstElement        = 0;
  Desc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;
  Desc.Buffer.NumElements         = InCount;
  Desc.Buffer.StructureByteStride = InStride;
  g_rhi_dx12->Device->CreateShaderResourceView(
      InBuffer->Buffer->Get(), &Desc, InBuffer->SRV.CPUHandle);
}

void CreateShaderResourceView_Raw(jBuffer_DX12* InBuffer,
                                  uint32_t      InBufferSize) {
  assert(g_rhi_dx12);
  assert(g_rhi_dx12->Device);

  assert(InBuffer);
  if (!InBuffer) {
    return;
  }

  assert(!InBuffer->SRV.IsValid());
  InBuffer->SRV = g_rhi_dx12->DescriptorHeaps.Alloc();

  D3D12_SHADER_RESOURCE_VIEW_DESC Desc{};
  Desc.Format                  = DXGI_FORMAT_R32_TYPELESS;
  Desc.ViewDimension           = D3D12_SRV_DIMENSION_BUFFER;
  Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  Desc.Buffer.FirstElement     = 0;
  Desc.Buffer.Flags            = D3D12_BUFFER_SRV_FLAG_RAW;
  Desc.Buffer.NumElements
      = InBufferSize / 4;  // DXGI_FORMAT_R32_TYPELESS size is 4
  g_rhi_dx12->Device->CreateShaderResourceView(
      InBuffer->Buffer->Get(), &Desc, InBuffer->SRV.CPUHandle);
}

void CreateShaderResourceView_Formatted(jBuffer_DX12*  InBuffer,
                                        ETextureFormat InFormat,
                                        uint32_t       InBufferSize) {
  assert(g_rhi_dx12);
  assert(g_rhi_dx12->Device);

  assert(InBuffer);
  if (!InBuffer) {
    return;
  }

  assert(!InBuffer->SRV.IsValid());
  InBuffer->SRV = g_rhi_dx12->DescriptorHeaps.Alloc();

  const uint32_t Stride = GetDX12TextureComponentCount(InFormat)
                        * GetDX12TexturePixelSize(InFormat);

  D3D12_SHADER_RESOURCE_VIEW_DESC Desc{};
  Desc.Format                  = GetDX12TextureFormat(InFormat);
  Desc.ViewDimension           = D3D12_SRV_DIMENSION_BUFFER;
  Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  Desc.Buffer.FirstElement     = 0;
  Desc.Buffer.Flags            = D3D12_BUFFER_SRV_FLAG_NONE;
  Desc.Buffer.NumElements      = InBufferSize / Stride;
  g_rhi_dx12->Device->CreateShaderResourceView(
      InBuffer->Buffer->Get(), &Desc, InBuffer->SRV.CPUHandle);
}

void CreateUnorderedAccessView_StructuredBuffer(jBuffer_DX12* InBuffer,
                                                uint32_t      InStride,
                                                uint32_t      InCount) {
  assert(g_rhi_dx12);
  assert(g_rhi_dx12->Device);

  assert(InBuffer);
  if (!InBuffer) {
    return;
  }

  assert(!InBuffer->UAV.IsValid());
  InBuffer->UAV = g_rhi_dx12->DescriptorHeaps.Alloc();

  D3D12_UNORDERED_ACCESS_VIEW_DESC Desc{};
  Desc.Format                     = DXGI_FORMAT_UNKNOWN;
  Desc.ViewDimension              = D3D12_UAV_DIMENSION_BUFFER;
  Desc.Buffer.FirstElement        = 0;
  Desc.Buffer.Flags               = D3D12_BUFFER_UAV_FLAG_NONE;
  Desc.Buffer.NumElements         = InCount;
  Desc.Buffer.StructureByteStride = InStride;
  g_rhi_dx12->Device->CreateUnorderedAccessView(
      InBuffer->Buffer->Get(), nullptr, &Desc, InBuffer->UAV.CPUHandle);
}

void CreateUnorderedAccessView_Raw(jBuffer_DX12* InBuffer,
                                   uint32_t      InBufferSize) {
  assert(g_rhi_dx12);
  assert(g_rhi_dx12->Device);

  assert(InBuffer);
  if (!InBuffer) {
    return;
  }

  assert(!InBuffer->UAV.IsValid());
  InBuffer->UAV = g_rhi_dx12->DescriptorHeaps.Alloc();

  D3D12_UNORDERED_ACCESS_VIEW_DESC Desc{};
  Desc.Format              = DXGI_FORMAT_R32_TYPELESS;
  Desc.ViewDimension       = D3D12_UAV_DIMENSION_BUFFER;
  Desc.Buffer.FirstElement = 0;
  Desc.Buffer.Flags        = D3D12_BUFFER_UAV_FLAG_RAW;
  Desc.Buffer.NumElements
      = InBufferSize / 4;  // DXGI_FORMAT_R32_TYPELESS size is 4
  g_rhi_dx12->Device->CreateUnorderedAccessView(
      InBuffer->Buffer->Get(), nullptr, &Desc, InBuffer->UAV.CPUHandle);
}

void CreateUnorderedAccessView_Formatted(jBuffer_DX12*  InBuffer,
                                         ETextureFormat InFormat,
                                         uint32_t       InBufferSize) {
  assert(g_rhi_dx12);
  assert(g_rhi_dx12->Device);

  assert(InBuffer);
  if (!InBuffer) {
    return;
  }

  assert(!InBuffer->UAV.IsValid());
  InBuffer->UAV = g_rhi_dx12->DescriptorHeaps.Alloc();

  const uint32_t Stride = GetDX12TextureComponentCount(InFormat)
                        * GetDX12TexturePixelSize(InFormat);

  D3D12_UNORDERED_ACCESS_VIEW_DESC Desc{};
  Desc.Format              = GetDX12TextureFormat(InFormat);
  Desc.ViewDimension       = D3D12_UAV_DIMENSION_BUFFER;
  Desc.Buffer.FirstElement = 0;
  Desc.Buffer.Flags        = D3D12_BUFFER_UAV_FLAG_NONE;
  Desc.Buffer.NumElements  = InBufferSize / Stride;
  g_rhi_dx12->Device->CreateUnorderedAccessView(
      InBuffer->Buffer->Get(), nullptr, &Desc, InBuffer->UAV.CPUHandle);
}

void CreateShaderResourceView(jTexture_DX12* InTexture) {
  assert(g_rhi_dx12);
  assert(g_rhi_dx12->Device);

  assert(InTexture);
  if (!InTexture) {
    return;
  }

  assert(!InTexture->SRV.IsValid());
  InTexture->SRV = g_rhi_dx12->DescriptorHeaps.Alloc();

  D3D12_SHADER_RESOURCE_VIEW_DESC Desc = {};
  Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  if (IsDepthFormat(InTexture->format)) {
    DXGI_FORMAT TexFormat, SrvFormat;
    GetDepthFormatForSRV(
        TexFormat, SrvFormat, GetDX12TextureFormat(InTexture->format));
    Desc.Format = SrvFormat;
  } else {
    Desc.Format = GetDX12TextureFormat(InTexture->format);
  }

  switch (InTexture->type) {
    case ETextureType::TEXTURE_2D:
      Desc.ViewDimension                 = ((int32_t)InTexture->sampleCount > 1)
                                             ? D3D12_SRV_DIMENSION_TEXTURE2DMS
                                             : D3D12_SRV_DIMENSION_TEXTURE2D;
      Desc.Texture2D.MipLevels           = InTexture->mipLevels;
      Desc.Texture2D.MostDetailedMip     = 0;
      Desc.Texture2D.PlaneSlice          = 0;
      Desc.Texture2D.ResourceMinLODClamp = 0.0f;
      break;
    case ETextureType::TEXTURE_2D_ARRAY:
      Desc.ViewDimension            = ((int32_t)InTexture->sampleCount > 1)
                                        ? D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY
                                        : D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
      Desc.Texture2DArray.MipLevels = InTexture->mipLevels;
      Desc.Texture2DArray.MostDetailedMip     = 0;
      Desc.Texture2DArray.PlaneSlice          = 0;
      Desc.Texture2DArray.ResourceMinLODClamp = 0.0f;
      Desc.Texture2DArray.ArraySize           = InTexture->layerCount;
      Desc.Texture2DArray.FirstArraySlice     = 0;
      break;
    case ETextureType::TEXTURE_CUBE:
      Desc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURECUBE;
      Desc.TextureCube.MipLevels           = InTexture->mipLevels;
      Desc.TextureCube.MostDetailedMip     = 0;
      Desc.TextureCube.ResourceMinLODClamp = 0.0f;
      break;
    default:
      assert(0);
      break;
  }

  g_rhi_dx12->Device->CreateShaderResourceView(
      InTexture->Texture->Get(), &Desc, InTexture->SRV.CPUHandle);
}

void CreateDepthStencilView(jTexture_DX12* InTexture) {
  assert(g_rhi_dx12);
  assert(g_rhi_dx12->Device);

  assert(InTexture);
  if (!InTexture) {
    return;
  }

  assert(!InTexture->DSV.IsValid());
  InTexture->DSV                     = g_rhi_dx12->DSVDescriptorHeaps.Alloc();
  D3D12_DEPTH_STENCIL_VIEW_DESC Desc = {};

  Desc.Format               = GetDX12TextureFormat(InTexture->format);
  const bool IsMultisampled = ((int32_t)InTexture->sampleCount > 1);
  switch (InTexture->type) {
    case ETextureType::TEXTURE_2D:
      Desc.ViewDimension      = IsMultisampled ? D3D12_DSV_DIMENSION_TEXTURE2DMS
                                               : D3D12_DSV_DIMENSION_TEXTURE2D;
      Desc.Texture2D.MipSlice = 0;
      break;
    case ETextureType::TEXTURE_2D_ARRAY:
    case ETextureType::TEXTURE_CUBE:
      if (IsMultisampled) {
        Desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
        Desc.Texture2DMSArray.FirstArraySlice = 0;
        Desc.Texture2DMSArray.ArraySize       = InTexture->layerCount;
      } else {
        Desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
        Desc.Texture2DArray.FirstArraySlice = 0;
        Desc.Texture2DArray.ArraySize       = InTexture->layerCount;
        Desc.Texture2DArray.MipSlice        = 0;
      }
      break;
    default:
      assert(0);
      break;
  }

  // const bool HasStencil = !IsDepthOnlyFormat(InTexture->format);
  // Desc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
  // if (HasStencil)
  //     Desc.Flags |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;
  Desc.Flags = D3D12_DSV_FLAG_NONE;

  g_rhi_dx12->Device->CreateDepthStencilView(
      InTexture->Texture->Get(), &Desc, InTexture->DSV.CPUHandle);
}

void CreateUnorderedAccessView(jTexture_DX12* InTexture) {
  assert(g_rhi_dx12);
  assert(g_rhi_dx12->Device);

  assert(InTexture);
  if (!InTexture) {
    return;
  }

  assert(!InTexture->UAV.IsValid());
  assert(InTexture->mipLevels > 0);
  for (int32_t i = 0; i < InTexture->mipLevels; ++i) {
    jDescriptor_DX12 UAV = g_rhi_dx12->DescriptorHeaps.Alloc();

    D3D12_UNORDERED_ACCESS_VIEW_DESC Desc = {};
    Desc.Format = GetDX12TextureFormat(InTexture->format);
    switch (InTexture->type) {
      case ETextureType::TEXTURE_2D:
        Desc.ViewDimension        = D3D12_UAV_DIMENSION_TEXTURE2D;
        Desc.Texture2D.MipSlice   = i;
        Desc.Texture2D.PlaneSlice = 0;
        break;
      case ETextureType::TEXTURE_2D_ARRAY:
        Desc.ViewDimension            = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        Desc.Texture2DArray.ArraySize = InTexture->layerCount;
        Desc.Texture2DArray.FirstArraySlice = 0;
        Desc.Texture2DArray.MipSlice        = i;
        Desc.Texture2DArray.PlaneSlice      = 0;
        break;
      case ETextureType::TEXTURE_CUBE:
        Desc.ViewDimension            = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        Desc.Texture2DArray.ArraySize = InTexture->layerCount;
        Desc.Texture2DArray.FirstArraySlice = 0;
        Desc.Texture2DArray.MipSlice        = i;
        Desc.Texture2DArray.PlaneSlice      = 0;
        break;
      default:
        assert(0);
        break;
    }

    g_rhi_dx12->Device->CreateUnorderedAccessView(
        InTexture->Texture->Get(), nullptr, &Desc, UAV.CPUHandle);

    if (i == 0) {
      InTexture->UAV = UAV;
    }

    InTexture->UAVMipMap[i] = UAV;
  }
}

void CreateRenderTargetView(jTexture_DX12* InTexture) {
  assert(g_rhi_dx12);
  assert(g_rhi_dx12->Device);

  assert(InTexture);
  if (!InTexture) {
    return;
  }

  assert(!InTexture->RTV.IsValid());
  InTexture->RTV = g_rhi_dx12->RTVDescriptorHeaps.Alloc();

  D3D12_RENDER_TARGET_VIEW_DESC Desc = {};
  Desc.Format                        = GetDX12TextureFormat(InTexture->format);
  switch (InTexture->type) {
    case ETextureType::TEXTURE_2D:
      Desc.ViewDimension        = D3D12_RTV_DIMENSION_TEXTURE2D;
      Desc.Texture2D.MipSlice   = 0;
      Desc.Texture2D.PlaneSlice = 0;
      break;
    case ETextureType::TEXTURE_2D_ARRAY:
      Desc.ViewDimension                  = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
      Desc.Texture2DArray.ArraySize       = InTexture->layerCount;
      Desc.Texture2DArray.FirstArraySlice = 0;
      Desc.Texture2DArray.MipSlice        = 0;
      Desc.Texture2DArray.PlaneSlice      = 0;
      break;
    case ETextureType::TEXTURE_CUBE:
      assert(InTexture->layerCount == 6);
      Desc.ViewDimension                  = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
      Desc.Texture2DArray.ArraySize       = InTexture->layerCount;
      Desc.Texture2DArray.FirstArraySlice = 0;
      Desc.Texture2DArray.MipSlice        = 0;
      Desc.Texture2DArray.PlaneSlice      = 0;
      break;
    default:
      assert(0);
      break;
  }

  g_rhi_dx12->Device->CreateRenderTargetView(
      InTexture->Texture->Get(), &Desc, InTexture->RTV.CPUHandle);
}

}  // namespace game_engine