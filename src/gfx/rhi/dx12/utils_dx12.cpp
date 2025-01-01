#include "gfx/rhi/dx12/utils_dx12.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/dx12/buffer_dx12.h"
#include "gfx/rhi/dx12/command_list_dx12.h"
#include "gfx/rhi/dx12/rhi_dx12.h"
#include "gfx/rhi/rhi.h"
#include "platform/windows/windows_platform_setup.h"
#include "resources/image.h"

namespace game_engine {

size_t g_bitsPerPixel(DXGI_FORMAT fmt) {
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

size_t g_bytesPerPixel(DXGI_FORMAT fmt) {
  return g_bitsPerPixel(fmt) / 8;
}

const D3D12_HEAP_PROPERTIES& g_getUploadHeap() {
  static D3D12_HEAP_PROPERTIES HeapProp{
    .Type                 = D3D12_HEAP_TYPE_UPLOAD,
    .CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
    .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
    .CreationNodeMask     = 0,
    .VisibleNodeMask      = 0};
  return HeapProp;
}

const D3D12_HEAP_PROPERTIES& g_getDefaultHeap() {
  static D3D12_HEAP_PROPERTIES HeapProp{
    .Type                 = D3D12_HEAP_TYPE_DEFAULT,
    .CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
    .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
    .CreationNodeMask     = 0,
    .VisibleNodeMask      = 0};
  return HeapProp;
}

const D3D12_RESOURCE_DESC& g_getUploadResourceDesc(uint64_t size) {
  static D3D12_RESOURCE_DESC Desc{
    .Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER,
    .Alignment        = 0,
    .Width            = size,
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

ComPtr<ID3D12Resource> g_createStagingBuffer(const void* initData,
                                             int64_t     size,
                                             uint64_t    alignment) {
  const uint64_t AlignedSize = g_align(size, alignment);

  ComPtr<ID3D12Resource> UploadResourceRHI;
  g_rhiDx12->m_device_->CreateCommittedResource(
      &g_getUploadHeap(),
      D3D12_HEAP_FLAG_NONE,
      &g_getUploadResourceDesc(AlignedSize),
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&UploadResourceRHI));

  void*       MappedPointer = nullptr;
  D3D12_RANGE range         = {};
  UploadResourceRHI->Map(0, &range, reinterpret_cast<void**>(&MappedPointer));
  memcpy(MappedPointer, initData, size);
  UploadResourceRHI->Unmap(0, &range);
  return UploadResourceRHI;
}

void g_uploadByUsingStagingBuffer(ComPtr<ID3D12Resource>& DestBuffer,
                                  const void*             initData,
                                  uint64_t                size,
                                  uint64_t                alignment) {
  assert(DestBuffer);

  const uint64_t AlignedSize = g_align(size, alignment);
  assert(DestBuffer->GetDesc().Width >= AlignedSize);

  ComPtr<ID3D12Resource> StagingBuffer
      = g_createStagingBuffer(initData, size, alignment);
  auto commandBuffer = std::static_pointer_cast<CommandBufferDx12>(
      g_rhiDx12->beginSingleTimeCopyCommands());
  assert(commandBuffer->isValid());
  commandBuffer->get()->CopyBufferRegion(
      DestBuffer.Get(), 0, StagingBuffer.Get(), 0, AlignedSize);
  g_rhiDx12->endSingleTimeCopyCommands(commandBuffer);
}

D3D12_RESOURCE_DESC g_getDefaultResourceDesc(uint64_t alignedSize,
                                             bool     isAllowUAV) {
  static D3D12_RESOURCE_DESC Desc{
    .Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER,
    .Alignment        = 0,
    .Width            = alignedSize,
    .Height           = 1,
    .DepthOrArraySize = 1,
    .MipLevels        = 1,
    .Format           = DXGI_FORMAT_UNKNOWN,
    .SampleDesc       = {.Count = 1, .Quality = 0},
    .Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
    .Flags            = isAllowUAV ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
                                   : D3D12_RESOURCE_FLAG_NONE,
  };

  return Desc;
}

ComPtr<ID3D12Resource> g_createDefaultResource(
    uint64_t              alignedSize,
    D3D12_RESOURCE_STATES initialState,
    bool                  isAllowUAV,
    bool                  isCPUAccessible,
    const wchar_t*        name) {
  const D3D12_RESOURCE_DESC Desc = g_getDefaultResourceDesc(alignedSize, false);
  const D3D12_HEAP_PROPERTIES HeapProperties
      = isCPUAccessible ? g_getUploadHeap() : g_getDefaultHeap();

  D3D12_RESOURCE_STATES ResourceState = initialState;
  if (isCPUAccessible) {
    ResourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
  }

  ComPtr<ID3D12Resource> NewResourceRHI;
  if (SUCCEEDED(g_rhiDx12->m_device_->CreateCommittedResource(
          &HeapProperties,
          D3D12_HEAP_FLAG_NONE,
          &Desc,
          ResourceState,
          nullptr,
          IID_PPV_ARGS(&NewResourceRHI)))) {
    if (name) {
      NewResourceRHI->SetName(name);
    }
  }

  return NewResourceRHI;
}

void* g_copyInitialData(ComPtr<ID3D12Resource>& dest,
                        const void*             initData,
                        uint64_t                size,
                        uint64_t                alignment,
                        bool                    isCPUAccessible) {
  if (dest) {
    if (isCPUAccessible) {
      void*       MappedPointer = nullptr;
      D3D12_RANGE range         = {};
      dest->Map(0, &range, reinterpret_cast<void**>(&MappedPointer));
      assert(MappedPointer);

      if (initData && MappedPointer) {
        memcpy(MappedPointer, initData, size);
      }
      return MappedPointer;
    } else {
      if (initData) {
        g_uploadByUsingStagingBuffer(dest, initData, size, alignment);
      }
    }
  }
  return nullptr;
}

std::shared_ptr<CreatedResourceDx12> g_createBufferInternal(
    uint64_t              size,
    uint64_t              alignment,
    EBufferCreateFlag     bufferCreateFlag,
    D3D12_RESOURCE_STATES initialResourceState,
    const wchar_t*        resourceName) {
  if (!!(bufferCreateFlag & EBufferCreateFlag::AccelerationStructure)) {
    assert(initialResourceState
           == D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
  } else if (!!(bufferCreateFlag & EBufferCreateFlag::Readback)) {
    assert(initialResourceState == D3D12_RESOURCE_STATE_COPY_DEST);
  } else if (!!(bufferCreateFlag & EBufferCreateFlag::CPUAccess)) {
    assert(initialResourceState == D3D12_RESOURCE_STATE_GENERIC_READ);
  } else {
    assert(initialResourceState == D3D12_RESOURCE_STATE_COMMON);
  }

  size = (alignment > 0) ? g_align(size, alignment) : size;
  if (RhiDx12::s_kIsUsePlacedResource) {
    size = g_align(size, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);
  }

  D3D12_RESOURCE_DESC resourceDesc = {};
  resourceDesc.Dimension           = D3D12_RESOURCE_DIMENSION_BUFFER;
  resourceDesc.Width               = size;
  resourceDesc.Height              = 1;
  resourceDesc.DepthOrArraySize    = 1;
  resourceDesc.MipLevels           = 1;
  resourceDesc.Format              = DXGI_FORMAT_UNKNOWN;
  resourceDesc.Flags            = !!(bufferCreateFlag & EBufferCreateFlag::UAV)
                                    ? D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
                                    : D3D12_RESOURCE_FLAG_NONE;
  resourceDesc.SampleDesc.Count = 1;
  resourceDesc.SampleDesc.Quality = 0;
  resourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  resourceDesc.Alignment          = 0;

  assert(g_rhiDx12);

  std::shared_ptr<CreatedResourceDx12> createdResource;
  if (!!(bufferCreateFlag & EBufferCreateFlag::Readback)) {
    assert(EBufferCreateFlag::NONE
           == (bufferCreateFlag
               & EBufferCreateFlag::UAV));  // Not allowed Readback with UAV

    ComPtr<ID3D12Resource>         NewResource;
    const CD3DX12_HEAP_PROPERTIES& HeapProperties
        = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
    HRESULT hr = g_rhiDx12->m_device_->CreateCommittedResource(
        &HeapProperties,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        initialResourceState,
        nullptr,
        IID_PPV_ARGS(&NewResource));

    assert(SUCCEEDED(hr));

    createdResource = CreatedResourceDx12::s_createdFromStandalone(NewResource);
  } else if (!!(bufferCreateFlag & EBufferCreateFlag::CPUAccess)) {
    assert(EBufferCreateFlag::NONE
           == (bufferCreateFlag
               & EBufferCreateFlag::UAV));  // Not allowed Readback with UAV
    createdResource
        = g_rhiDx12->createUploadResource(&resourceDesc, initialResourceState);
  } else {
    createdResource
        = g_rhiDx12->createResource(&resourceDesc, initialResourceState);
  }

  assert(createdResource->m_resource_);

  if (resourceName && createdResource->m_resource_) {
    createdResource->m_resource_.get()->Get()->SetName(resourceName);
  }

  return createdResource;
}

std::shared_ptr<BufferDx12> g_createBuffer(uint64_t          size,
                                           uint64_t          alignment,
                                           EBufferCreateFlag bufferCreateFlag,
                                           EResourceLayout   layout,
                                           const void*       data,
                                           uint64_t          dataSize,
                                           const wchar_t*    resourceName) {
  // If the resource needed to be created with
  // EBufferCreateFlag::AccelerationStructure, you must initialize the buffer
  // resource state as ACCELERATION_STRUCTURE state.
  assert(
      layout != EResourceLayout::ACCELERATION_STRUCTURE
      || (layout == EResourceLayout::ACCELERATION_STRUCTURE
          && !!(bufferCreateFlag & EBufferCreateFlag::AccelerationStructure)));

  EResourceLayout       InitialLayout = EResourceLayout::UNDEFINED;
  D3D12_RESOURCE_STATES initialLayoutDx12
      = g_getDX12ResourceLayout(InitialLayout);
  if (!!(bufferCreateFlag & EBufferCreateFlag::AccelerationStructure)) {
    assert(layout == EResourceLayout::ACCELERATION_STRUCTURE);
    initialLayoutDx12 = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
    InitialLayout     = EResourceLayout::ACCELERATION_STRUCTURE;
  } else if (!!(bufferCreateFlag & EBufferCreateFlag::Readback)) {
    initialLayoutDx12 = D3D12_RESOURCE_STATE_COPY_DEST;
    InitialLayout     = EResourceLayout::TRANSFER_DST;
  } else if (!!(bufferCreateFlag & EBufferCreateFlag::CPUAccess)) {
    initialLayoutDx12 = D3D12_RESOURCE_STATE_GENERIC_READ;
    InitialLayout     = EResourceLayout::READ_ONLY;
  }

  std::shared_ptr<CreatedResourceDx12> BufferInternal = g_createBufferInternal(
      size, alignment, bufferCreateFlag, initialLayoutDx12, resourceName);
  if (!BufferInternal->m_resource_) {
    return nullptr;
  }

  auto BufferPtr = std::make_shared<BufferDx12>(
      BufferInternal, size, alignment, bufferCreateFlag);
  BufferPtr->m_layout_ = InitialLayout;
  if (resourceName) {
    // https://learn.microsoft.com/ko-kr/cpp/text/how-to-convert-between-various-string-types?view=msvc-170#example-convert-from-char-
    char         szResourceName[1024];
    size_t       length   = 0;
    size_t       origsize = wcslen(resourceName) + 1;
    const size_t newsize  = origsize * 2;
    wcstombs_s(&length, szResourceName, newsize, resourceName, _TRUNCATE);
    BufferPtr->m_resourceName_ = Name(szResourceName);
  }

  const bool HasInitialData = data && (dataSize > 0);
  if (HasInitialData) {
    if (!!(bufferCreateFlag & EBufferCreateFlag::Readback)) {
      // nothing todo
    } else if (!!(bufferCreateFlag & EBufferCreateFlag::CPUAccess)) {
      void* CPUAddress = BufferPtr->map();
      assert(CPUAddress);
      memcpy(CPUAddress, data, dataSize);
    } else {
      std::shared_ptr<CreatedResourceDx12> StagingBuffer
          = g_createBufferInternal(
              size,
              alignment,
              EBufferCreateFlag::CPUAccess,
              D3D12_RESOURCE_STATE_GENERIC_READ,
              resourceName);  // CPU Access should be created with
                              // 'D3D12_RESOURCE_STATE_GENERIC_READ'.
      assert(StagingBuffer->isValid());

      void*       MappedPointer = nullptr;
      D3D12_RANGE range         = {};
      HRESULT     hr            = StagingBuffer->m_resource_.get()->Get()->Map(
          0, &range, &MappedPointer);
      assert(SUCCEEDED(hr));

      if (SUCCEEDED(hr)) {
        assert(MappedPointer);
        memcpy(MappedPointer, data, dataSize);
        StagingBuffer->m_resource_.get()->Get()->Unmap(0, nullptr);
      }

      auto commandBuffer = std::static_pointer_cast<CommandBufferDx12>(
          g_rhiDx12->beginSingleTimeCopyCommands());

      assert(commandBuffer->isValid());

      commandBuffer->get()->CopyBufferRegion(
          (ID3D12Resource*)BufferPtr->getHandle(),
          0,
          StagingBuffer->get(),
          0,
          size);
      g_rhiDx12->endSingleTimeCopyCommands(commandBuffer);
    }
  }

  if (BufferPtr->m_layout_ != layout) {
    g_rhi->transitionLayoutImmediate(BufferPtr.get(), layout);
  }

  return BufferPtr;
}

std::shared_ptr<CreatedResourceDx12> g_createTexturenternal(
    uint32_t                 witdh,
    uint32_t                 height,
    uint32_t                 arrayLayers,
    uint32_t                 mipLevels,
    uint32_t                 numOfSamples,
    D3D12_RESOURCE_DIMENSION type,
    DXGI_FORMAT              format,
    ETextureCreateFlag       textureCreateFlag,
    EResourceLayout          imageLayout,
    D3D12_CLEAR_VALUE*       clearValue,
    const wchar_t*           resourceName) {
  assert(g_rhiDx12);
  assert(g_rhiDx12->m_device_);

  D3D12_RESOURCE_DESC TexDesc = {};
  TexDesc.MipLevels           = mipLevels;
  if (s_isDepthFormat(g_getDX12TextureFormat(format))) {
    DXGI_FORMAT TexFormat, SrvFormat;
    g_getDepthFormatForSRV(TexFormat, SrvFormat, format);
    TexDesc.Format = TexFormat;
  } else {
    TexDesc.Format = format;
  }
  TexDesc.Width  = witdh;
  TexDesc.Height = height;
  TexDesc.Flags  = D3D12_RESOURCE_FLAG_NONE;

  if (!!(textureCreateFlag & ETextureCreateFlag::RTV)) {
    TexDesc.Flags = s_isDepthFormat(g_getDX12TextureFormat(format))
                      ? D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
                      : D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
  }
  if (!!(textureCreateFlag & ETextureCreateFlag::UAV)) {
    TexDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
  }
  TexDesc.DepthOrArraySize = arrayLayers;

  const uint32_t StandardMSAAPattern = 0xFF'FF'FF'FF;
  TexDesc.SampleDesc.Count           = numOfSamples;
  TexDesc.SampleDesc.Quality = numOfSamples > 1 ? StandardMSAAPattern : 0;

  TexDesc.Dimension = type;
  TexDesc.Layout    = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  TexDesc.Alignment = 0;

  std::shared_ptr<CreatedResourceDx12> ImageResource
      = g_rhiDx12->createResource(
          &TexDesc, g_getDX12ResourceLayout(imageLayout), clearValue);
  assert(ImageResource->m_resource_);

  if (resourceName && ImageResource->m_resource_) {
    ImageResource->m_resource_.get()->Get()->SetName(resourceName);
  }

  return ImageResource;
}

std::shared_ptr<TextureDx12> g_createTexture(
    uint32_t            witdh,
    uint32_t            height,
    uint32_t            arrayLayers,
    uint32_t            mipLevels,
    uint32_t            numOfSamples,
    ETextureType        type,
    ETextureFormat      format,
    ETextureCreateFlag  textureCreateFlag,
    EResourceLayout     imageLayout,
    const RtClearValue& clearValue,
    const wchar_t*      resourceName) {
  bool              HasClearValue = false;
  D3D12_CLEAR_VALUE ClearValue{};
  if (!!(textureCreateFlag & ETextureCreateFlag::RTV)) {
    if (clearValue.getType() == ERTClearType::Color) {
      ClearValue.Color[0] = clearValue.getCleraColor()[0];
      ClearValue.Color[1] = clearValue.getCleraColor()[1];
      ClearValue.Color[2] = clearValue.getCleraColor()[2];
      ClearValue.Color[3] = clearValue.getCleraColor()[3];
      ClearValue.Format   = g_getDX12TextureFormat(format);
    } else if (clearValue.getType() == ERTClearType::DepthStencil) {
      ClearValue.DepthStencil.Depth   = clearValue.getClearDepth();
      ClearValue.DepthStencil.Stencil = clearValue.getClearStencil();
      ClearValue.Format               = g_getDX12TextureFormat(format);
    }
    HasClearValue = clearValue.getType() != ERTClearType::None;
  }

  std::shared_ptr<CreatedResourceDx12> TextureInternal
      = g_createTexturenternal(witdh,
                               height,
                               arrayLayers,
                               mipLevels,
                               numOfSamples,
                               g_getDX12TextureDemension(type),
                               g_getDX12TextureFormat(format),
                               textureCreateFlag,
                               imageLayout,
                               (HasClearValue ? &ClearValue : nullptr),
                               resourceName);
  assert(TextureInternal->isValid());

  auto TexturePtr = std::make_shared<TextureDx12>(
      type,
      format,
      // TODO: remove casting
      math::Dimension2Di{static_cast<int>(witdh), static_cast<int>(height)},
      arrayLayers,
      EMSAASamples::COUNT_1,
      false,
      clearValue,
      TextureInternal);
  assert(TexturePtr);
  // TODO: hotfix (remove mip level assigning here)
  TexturePtr->m_mipLevels_ = mipLevels;
  TexturePtr->m_layout_    = imageLayout;

  // TODO: general - why do we need ResourceName?
  if (resourceName) {
    // https://learn.microsoft.com/ko-kr/cpp/text/how-to-convert-between-various-string-types?view=msvc-170#example-convert-from-char-
    char         szResourceName[1024];
    size_t       length   = 0;
    size_t       origsize = wcslen(resourceName) + 1;
    const size_t newsize  = origsize * 2;
    wcstombs_s(&length, szResourceName, newsize, resourceName, _TRUNCATE);

    TexturePtr->m_resourceName_ = Name(szResourceName);
  }

  if (s_isDepthFormat(format)) {
    g_createShaderResourceView(TexturePtr.get());
    g_createDepthStencilView(TexturePtr.get());
  } else {
    g_createShaderResourceView(TexturePtr.get());
    if (!!(textureCreateFlag & ETextureCreateFlag::RTV)) {
      g_createRenderTargetView(TexturePtr.get());
    }
    if (!!(textureCreateFlag & ETextureCreateFlag::UAV)) {
      g_createUnorderedAccessView(TexturePtr.get());
    }
  }

  return TexturePtr;
}

std::shared_ptr<TextureDx12> g_createTexture(
    const std::shared_ptr<CreatedResourceDx12>& texture,
    ETextureCreateFlag                          textureCreateFlag,
    EResourceLayout                             imageLayout,
    const RtClearValue&                         clearValue,
    const wchar_t*                              resourceName) {
  const auto desc       = texture->m_resource_.get()->Get()->GetDesc();
  auto       TexturePtr = std::make_shared<TextureDx12>(
      g_getDX12TextureDemension(desc.Dimension, desc.DepthOrArraySize > 1),
      g_getDX12TextureFormat(desc.Format),
      // TODO: remove casting
      math::Dimension2Di{static_cast<int>(desc.Width),
                         static_cast<int>(desc.Height)},
      (int32_t)desc.DepthOrArraySize,
      EMSAASamples::COUNT_1,
      false,
      clearValue,
      texture);

  assert(TexturePtr);
  TexturePtr->m_layout_ = imageLayout;

  if (resourceName) {
    // https://learn.microsoft.com/ko-kr/cpp/text/how-to-convert-between-various-string-types?view=msvc-170#example-convert-from-char-
    char         szResourceName[1024];
    size_t       length   = 0;
    size_t       origsize = wcslen(resourceName) + 1;
    const size_t newsize  = origsize * 2;
    wcstombs_s(&length, szResourceName, newsize, resourceName, _TRUNCATE);

    TexturePtr->m_resourceName_ = Name(szResourceName);
  }

  if (s_isDepthFormat(g_getDX12TextureFormat(desc.Format))) {
    g_createShaderResourceView(TexturePtr.get());
    g_createDepthStencilView(TexturePtr.get());
  } else {
    g_createShaderResourceView(TexturePtr.get());
    if (!!(textureCreateFlag & ETextureCreateFlag::RTV)) {
      g_createRenderTargetView(TexturePtr.get());
    }
    if (!!(textureCreateFlag & ETextureCreateFlag::UAV)) {
      g_createUnorderedAccessView(TexturePtr.get());
    }
  }

  return TexturePtr;
}

void g_copyBufferToTexture(ID3D12GraphicsCommandList4*   commandBuffer,
                           ID3D12Resource*               buffer,
                           ID3D12Resource*               imageResource,
                           const std::shared_ptr<Image>& image) {
  DXGI_FORMAT    dxgiFormat = g_getDX12TextureFormat(image->format);
  const uint8_t* basePtr
      = reinterpret_cast<const uint8_t*>(image->pixels.data());

  for (size_t i = 0; i < image->subImages.size(); ++i) {
    const auto& sub = image->subImages[i];

    // Compute offset into pixels
    const uint8_t* subPtr
        = reinterpret_cast<const uint8_t*>(&(*sub.pixelBegin));
    uint64_t offset = static_cast<uint64_t>(std::distance(basePtr, subPtr));
    uint32_t width  = static_cast<uint32_t>(sub.width);
    uint32_t height = static_cast<uint32_t>(sub.height);
    uint32_t depth  = static_cast<uint32_t>(image->depth);

    D3D12_TEXTURE_COPY_LOCATION dst = {};
    dst.pResource                   = imageResource;
    dst.Type                        = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.SubresourceIndex            = static_cast<UINT>(i);

    D3D12_TEXTURE_COPY_LOCATION src = {};
    src.pResource                   = buffer;
    src.Type                        = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;

    src.PlacedFootprint.Footprint.Format   = dxgiFormat;
    src.PlacedFootprint.Footprint.Width    = width;
    src.PlacedFootprint.Footprint.Height   = height;
    src.PlacedFootprint.Footprint.Depth    = depth;
    src.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(sub.rowPitch);
    src.PlacedFootprint.Offset             = offset;

    commandBuffer->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
  }
}

void g_copyBuffer(ID3D12GraphicsCommandList4* commandBuffer,
                  ID3D12Resource*             srcBuffer,
                  ID3D12Resource*             dstBuffer,
                  uint64_t                    size,
                  uint64_t                    srcOffset,
                  uint64_t                    dstOffset) {
  assert(commandBuffer);
  assert(srcBuffer);
  assert(dstBuffer);

  commandBuffer->CopyBufferRegion(
      dstBuffer, dstOffset, srcBuffer, srcOffset, size);
}

void g_copyBuffer(ID3D12Resource* srcBuffer,
                  ID3D12Resource* dstBuffer,
                  uint64_t        size,
                  uint64_t        srcOffset,
                  uint64_t        dstOffset) {
  auto commandBuffer = std::static_pointer_cast<CommandBufferDx12>(
      g_rhiDx12->beginSingleTimeCopyCommands());

  g_copyBuffer(
      commandBuffer->get(), srcBuffer, dstBuffer, size, srcOffset, dstOffset);
  g_rhiDx12->endSingleTimeCopyCommands(commandBuffer);
}

void g_createConstantBufferView(BufferDx12* buffer) {
  assert(g_rhiDx12);
  assert(g_rhiDx12->m_device_);

  assert(buffer);
  if (!buffer) {
    return;
  }

  assert(!buffer->m_cbv_.isValid());
  buffer->m_cbv_ = g_rhiDx12->m_descriptorHeaps_.alloc();

  D3D12_CONSTANT_BUFFER_VIEW_DESC Desc{};
  Desc.BufferLocation = buffer->getGPUAddress();
  Desc.SizeInBytes    = (uint32_t)buffer->getAllocatedSize();

  g_rhiDx12->m_device_->CreateConstantBufferView(&Desc,
                                                 buffer->m_cbv_.m_cpuHandle_);
}

void g_createShaderResourceViewStructuredBuffer(BufferDx12* buffer,
                                                uint32_t    stride,
                                                uint32_t    count) {
  assert(g_rhiDx12);
  assert(g_rhiDx12->m_device_);

  assert(buffer);
  if (!buffer) {
    return;
  }

  assert(!buffer->m_srv_.isValid());
  buffer->m_srv_ = g_rhiDx12->m_descriptorHeaps_.alloc();

  D3D12_SHADER_RESOURCE_VIEW_DESC Desc{};
  Desc.Format                     = DXGI_FORMAT_UNKNOWN;
  Desc.ViewDimension              = D3D12_SRV_DIMENSION_BUFFER;
  Desc.Shader4ComponentMapping    = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  Desc.Buffer.FirstElement        = 0;
  Desc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_NONE;
  Desc.Buffer.NumElements         = count;
  Desc.Buffer.StructureByteStride = stride;
  g_rhiDx12->m_device_->CreateShaderResourceView(
      buffer->m_buffer->get(), &Desc, buffer->m_srv_.m_cpuHandle_);
}

void g_createShaderResourceViewRaw(BufferDx12* buffer, uint32_t bufferSize) {
  assert(g_rhiDx12);
  assert(g_rhiDx12->m_device_);

  assert(buffer);
  if (!buffer) {
    return;
  }

  assert(!buffer->m_srv_.isValid());
  buffer->m_srv_ = g_rhiDx12->m_descriptorHeaps_.alloc();

  D3D12_SHADER_RESOURCE_VIEW_DESC Desc{};
  Desc.Format                  = DXGI_FORMAT_R32_TYPELESS;
  Desc.ViewDimension           = D3D12_SRV_DIMENSION_BUFFER;
  Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  Desc.Buffer.FirstElement     = 0;
  Desc.Buffer.Flags            = D3D12_BUFFER_SRV_FLAG_RAW;
  Desc.Buffer.NumElements
      = bufferSize / 4;  // DXGI_FORMAT_R32_TYPELESS size is 4
  g_rhiDx12->m_device_->CreateShaderResourceView(
      buffer->m_buffer->get(), &Desc, buffer->m_srv_.m_cpuHandle_);
}

void g_createShaderResourceViewFormatted(BufferDx12*    buffer,
                                         ETextureFormat format,
                                         uint32_t       bufferSize) {
  assert(g_rhiDx12);
  assert(g_rhiDx12->m_device_);

  assert(buffer);
  if (!buffer) {
    return;
  }

  assert(!buffer->m_srv_.isValid());
  buffer->m_srv_ = g_rhiDx12->m_descriptorHeaps_.alloc();

  const uint32_t Stride = g_getDX12TextureComponentCount(format)
                        * g_getDX12TexturePixelSize(format);

  D3D12_SHADER_RESOURCE_VIEW_DESC Desc{};
  Desc.Format                  = g_getDX12TextureFormat(format);
  Desc.ViewDimension           = D3D12_SRV_DIMENSION_BUFFER;
  Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  Desc.Buffer.FirstElement     = 0;
  Desc.Buffer.Flags            = D3D12_BUFFER_SRV_FLAG_NONE;
  Desc.Buffer.NumElements      = bufferSize / Stride;
  g_rhiDx12->m_device_->CreateShaderResourceView(
      buffer->m_buffer->get(), &Desc, buffer->m_srv_.m_cpuHandle_);
}

void g_createUnorderedAccessViewStructuredBuffer(BufferDx12* buffer,
                                                 uint32_t    stride,
                                                 uint32_t    count) {
  assert(g_rhiDx12);
  assert(g_rhiDx12->m_device_);

  assert(buffer);
  if (!buffer) {
    return;
  }

  assert(!buffer->m_uav_.isValid());
  buffer->m_uav_ = g_rhiDx12->m_descriptorHeaps_.alloc();

  D3D12_UNORDERED_ACCESS_VIEW_DESC Desc{};
  Desc.Format                     = DXGI_FORMAT_UNKNOWN;
  Desc.ViewDimension              = D3D12_UAV_DIMENSION_BUFFER;
  Desc.Buffer.FirstElement        = 0;
  Desc.Buffer.Flags               = D3D12_BUFFER_UAV_FLAG_NONE;
  Desc.Buffer.NumElements         = count;
  Desc.Buffer.StructureByteStride = stride;
  g_rhiDx12->m_device_->CreateUnorderedAccessView(
      buffer->m_buffer->get(), nullptr, &Desc, buffer->m_uav_.m_cpuHandle_);
}

void g_createUnorderedAccessViewRaw(BufferDx12* buffer, uint32_t bufferSize) {
  assert(g_rhiDx12);
  assert(g_rhiDx12->m_device_);

  assert(buffer);
  if (!buffer) {
    return;
  }

  assert(!buffer->m_uav_.isValid());
  buffer->m_uav_ = g_rhiDx12->m_descriptorHeaps_.alloc();

  D3D12_UNORDERED_ACCESS_VIEW_DESC Desc{};
  Desc.Format              = DXGI_FORMAT_R32_TYPELESS;
  Desc.ViewDimension       = D3D12_UAV_DIMENSION_BUFFER;
  Desc.Buffer.FirstElement = 0;
  Desc.Buffer.Flags        = D3D12_BUFFER_UAV_FLAG_RAW;
  Desc.Buffer.NumElements
      = bufferSize / 4;  // DXGI_FORMAT_R32_TYPELESS size is 4
  g_rhiDx12->m_device_->CreateUnorderedAccessView(
      buffer->m_buffer->get(), nullptr, &Desc, buffer->m_uav_.m_cpuHandle_);
}

void g_createUnorderedAccessViewFormatted(BufferDx12*    buffer,
                                          ETextureFormat format,
                                          uint32_t       bufferSize) {
  assert(g_rhiDx12);
  assert(g_rhiDx12->m_device_);

  assert(buffer);
  if (!buffer) {
    return;
  }

  assert(!buffer->m_uav_.isValid());
  buffer->m_uav_ = g_rhiDx12->m_descriptorHeaps_.alloc();

  const uint32_t Stride = g_getDX12TextureComponentCount(format)
                        * g_getDX12TexturePixelSize(format);

  D3D12_UNORDERED_ACCESS_VIEW_DESC Desc{};
  Desc.Format              = g_getDX12TextureFormat(format);
  Desc.ViewDimension       = D3D12_UAV_DIMENSION_BUFFER;
  Desc.Buffer.FirstElement = 0;
  Desc.Buffer.Flags        = D3D12_BUFFER_UAV_FLAG_NONE;
  Desc.Buffer.NumElements  = bufferSize / Stride;
  g_rhiDx12->m_device_->CreateUnorderedAccessView(
      buffer->m_buffer->get(), nullptr, &Desc, buffer->m_uav_.m_cpuHandle_);
}

void g_createShaderResourceView(TextureDx12* texture) {
  assert(g_rhiDx12);
  assert(g_rhiDx12->m_device_);

  assert(texture);
  if (!texture) {
    return;
  }

  assert(!texture->m_srv_.isValid());
  texture->m_srv_ = g_rhiDx12->m_descriptorHeaps_.alloc();

  D3D12_SHADER_RESOURCE_VIEW_DESC Desc = {};
  Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
  if (s_isDepthFormat(texture->m_format_)) {
    DXGI_FORMAT TexFormat, SrvFormat;
    g_getDepthFormatForSRV(
        TexFormat, SrvFormat, g_getDX12TextureFormat(texture->m_format_));
    Desc.Format = SrvFormat;
  } else {
    Desc.Format = g_getDX12TextureFormat(texture->m_format_);
  }

  switch (texture->m_type_) {
    case ETextureType::TEXTURE_2D:
      Desc.ViewDimension             = ((int32_t)texture->m_sampleCount_ > 1)
                                         ? D3D12_SRV_DIMENSION_TEXTURE2DMS
                                         : D3D12_SRV_DIMENSION_TEXTURE2D;
      Desc.Texture2D.MipLevels       = texture->m_mipLevels_;
      Desc.Texture2D.MostDetailedMip = 0;
      Desc.Texture2D.PlaneSlice      = 0;
      Desc.Texture2D.ResourceMinLODClamp = 0.0f;
      break;
    case ETextureType::TEXTURE_2D_ARRAY:
      Desc.ViewDimension            = ((int32_t)texture->m_sampleCount_ > 1)
                                        ? D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY
                                        : D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
      Desc.Texture2DArray.MipLevels = texture->m_mipLevels_;
      Desc.Texture2DArray.MostDetailedMip     = 0;
      Desc.Texture2DArray.PlaneSlice          = 0;
      Desc.Texture2DArray.ResourceMinLODClamp = 0.0f;
      Desc.Texture2DArray.ArraySize           = texture->m_layerCount_;
      Desc.Texture2DArray.FirstArraySlice     = 0;
      break;
    case ETextureType::TEXTURE_CUBE:
      Desc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURECUBE;
      Desc.TextureCube.MipLevels           = texture->m_mipLevels_;
      Desc.TextureCube.MostDetailedMip     = 0;
      Desc.TextureCube.ResourceMinLODClamp = 0.0f;
      break;
    default:
      assert(0);
      break;
  }

  g_rhiDx12->m_device_->CreateShaderResourceView(
      texture->m_texture->get(), &Desc, texture->m_srv_.m_cpuHandle_);
}

void g_createDepthStencilView(TextureDx12* texture) {
  assert(g_rhiDx12);
  assert(g_rhiDx12->m_device_);

  assert(texture);
  if (!texture) {
    return;
  }

  assert(!texture->m_dsv_.isValid());
  texture->m_dsv_                    = g_rhiDx12->m_dsvDescriptorHeaps_.alloc();
  D3D12_DEPTH_STENCIL_VIEW_DESC Desc = {};

  Desc.Format               = g_getDX12TextureFormat(texture->m_format_);
  const bool IsMultisampled = ((int32_t)texture->m_sampleCount_ > 1);
  switch (texture->m_type_) {
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
        Desc.Texture2DMSArray.ArraySize       = texture->m_layerCount_;
      } else {
        Desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
        Desc.Texture2DArray.FirstArraySlice = 0;
        Desc.Texture2DArray.ArraySize       = texture->m_layerCount_;
        Desc.Texture2DArray.MipSlice        = 0;
      }
      break;
    default:
      assert(0);
      break;
  }

  // const bool HasStencil = !g_isDepthOnlyFormat(texture->format);
  // Desc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
  // if (HasStencil)
  //     Desc.Flags |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;
  Desc.Flags = D3D12_DSV_FLAG_NONE;

  g_rhiDx12->m_device_->CreateDepthStencilView(
      texture->m_texture->get(), &Desc, texture->m_dsv_.m_cpuHandle_);
}

void g_createUnorderedAccessView(TextureDx12* texture) {
  assert(g_rhiDx12);
  assert(g_rhiDx12->m_device_);

  assert(texture);
  if (!texture) {
    return;
  }

  assert(!texture->m_uav_.isValid());
  assert(texture->m_mipLevels_ > 0);
  for (int32_t i = 0; i < texture->m_mipLevels_; ++i) {
    DescriptorDx12 UAV = g_rhiDx12->m_descriptorHeaps_.alloc();

    D3D12_UNORDERED_ACCESS_VIEW_DESC Desc = {};
    Desc.Format = g_getDX12TextureFormat(texture->m_format_);
    switch (texture->m_type_) {
      case ETextureType::TEXTURE_2D:
        Desc.ViewDimension        = D3D12_UAV_DIMENSION_TEXTURE2D;
        Desc.Texture2D.MipSlice   = i;
        Desc.Texture2D.PlaneSlice = 0;
        break;
      case ETextureType::TEXTURE_2D_ARRAY:
        Desc.ViewDimension            = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        Desc.Texture2DArray.ArraySize = texture->m_layerCount_;
        Desc.Texture2DArray.FirstArraySlice = 0;
        Desc.Texture2DArray.MipSlice        = i;
        Desc.Texture2DArray.PlaneSlice      = 0;
        break;
      case ETextureType::TEXTURE_CUBE:
        Desc.ViewDimension            = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        Desc.Texture2DArray.ArraySize = texture->m_layerCount_;
        Desc.Texture2DArray.FirstArraySlice = 0;
        Desc.Texture2DArray.MipSlice        = i;
        Desc.Texture2DArray.PlaneSlice      = 0;
        break;
      default:
        assert(0);
        break;
    }

    g_rhiDx12->m_device_->CreateUnorderedAccessView(
        texture->m_texture->get(), nullptr, &Desc, UAV.m_cpuHandle_);

    if (i == 0) {
      texture->m_uav_ = UAV;
    }

    texture->m_uavMipMap[i] = UAV;
  }
}

void g_createRenderTargetView(TextureDx12* texture) {
  assert(g_rhiDx12);
  assert(g_rhiDx12->m_device_);

  assert(texture);
  if (!texture) {
    return;
  }

  assert(!texture->m_rtv_.isValid());
  texture->m_rtv_ = g_rhiDx12->m_rtvDescriptorHeaps_.alloc();

  D3D12_RENDER_TARGET_VIEW_DESC Desc = {};
  Desc.Format = g_getDX12TextureFormat(texture->m_format_);
  switch (texture->m_type_) {
    case ETextureType::TEXTURE_2D:
      Desc.ViewDimension        = D3D12_RTV_DIMENSION_TEXTURE2D;
      Desc.Texture2D.MipSlice   = 0;
      Desc.Texture2D.PlaneSlice = 0;
      break;
    case ETextureType::TEXTURE_2D_ARRAY:
      Desc.ViewDimension                  = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
      Desc.Texture2DArray.ArraySize       = texture->m_layerCount_;
      Desc.Texture2DArray.FirstArraySlice = 0;
      Desc.Texture2DArray.MipSlice        = 0;
      Desc.Texture2DArray.PlaneSlice      = 0;
      break;
    case ETextureType::TEXTURE_CUBE:
      assert(texture->m_layerCount_ == 6);
      Desc.ViewDimension                  = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
      Desc.Texture2DArray.ArraySize       = texture->m_layerCount_;
      Desc.Texture2DArray.FirstArraySlice = 0;
      Desc.Texture2DArray.MipSlice        = 0;
      Desc.Texture2DArray.PlaneSlice      = 0;
      break;
    default:
      assert(0);
      break;
  }

  g_rhiDx12->m_device_->CreateRenderTargetView(
      texture->m_texture->get(), &Desc, texture->m_rtv_.m_cpuHandle_);
}

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12
