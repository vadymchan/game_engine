#include "gfx/rhi/rhi_new/backends/dx12/texture_dx12.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/rhi_new/backends/dx12/command_buffer_dx12.h"
#include "gfx/rhi/rhi_new/backends/dx12/descriptor_dx12.h"
#include "gfx/rhi/rhi_new/backends/dx12/device_dx12.h"
#include "gfx/rhi/rhi_new/backends/dx12/rhi_enums_dx12.h"
#include "gfx/rhi/rhi_new/backends/dx12/synchronization_dx12.h"
#include "utils/logger/global_logger.h"

namespace game_engine {
namespace gfx {
namespace rhi {

TextureDx12::TextureDx12(const TextureDesc& desc, DeviceDx12* device)
    : Texture(desc)
    , m_device_(device) {
  // Convert our generic format to DXGI format
  m_dxgiFormat_ = g_getTextureFormatDx12(desc.format);

  // Set initial state based on the initial layout
  m_currentState_  = g_getResourceLayoutDx12(desc.initialLayout);
  m_currentLayout_ = desc.initialLayout;

  // Create the resource and views
  if (!createResource_() || !createViews_()) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create DirectX 12 texture");
  }
}

TextureDx12::TextureDx12(DeviceDx12*                 device,
                         const TextureDesc&          desc,
                         ID3D12Resource*             existingResource,
                         D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle,
                         D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle)
    : Texture(desc)
    , m_device_(device)
    , m_resource_(existingResource)
    , m_rtvHandle_(rtvHandle)
    , m_dsvHandle_(dsvHandle)
    , m_ownsResource_(false) {
  // Convert our generic format to DXGI format
  m_dxgiFormat_ = g_getTextureFormatDx12(desc.format);

  // Set initial state based on the initial layout
  m_currentState_  = g_getResourceLayoutDx12(desc.initialLayout);
  m_currentLayout_ = desc.initialLayout;

  // For externally-provided resources, we might still want to create additional views
  // (like SRV), but we'll skip this for simplicity unless needed.
}

TextureDx12::~TextureDx12() {
  if (m_device_) {
    DescriptorHeap* rtvHeap    = m_device_->getRtvHeap();
    DescriptorHeap* dsvHeap    = m_device_->getDsvHeap();
    DescriptorHeap* srvUavHeap = m_device_->getCbvSrvUavHeap();

    // Free descriptor heap allocations
    if (rtvHeap && m_rtvDescriptorIndex_ != UINT32_MAX) {
      rtvHeap->free(m_rtvDescriptorIndex_);
    }

    if (dsvHeap && m_dsvDescriptorIndex_ != UINT32_MAX) {
      dsvHeap->free(m_dsvDescriptorIndex_);
    }

    if (srvUavHeap && m_srvDescriptorIndex_ != UINT32_MAX) {
      srvUavHeap->free(m_srvDescriptorIndex_);
    }

    // Free UAV descriptors
    for (auto index : m_uavDescriptorIndices_) {
      if (srvUavHeap && index != UINT32_MAX) {
        srvUavHeap->free(index);
      }
    }
  }

  // The ComPtr will automatically release the resource
}

bool TextureDx12::createResource_() {
  // Set up resource description
  D3D12_RESOURCE_DESC resourceDesc = {};

  // Set dimension based on texture type
  switch (m_desc_.type) {
    case TextureType::Texture1D:
      resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
      break;
    case TextureType::Texture2D:
    case TextureType::Texture2DArray:
    case TextureType::TextureCube:
      resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
      break;
    case TextureType::Texture3D:
    case TextureType::Texture3DArray:
      resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
      break;
    default:
      GlobalLogger::Log(LogLevel::Error, "Unsupported texture type");
      return false;
  }

  // Set format and dimensions
  resourceDesc.Format           = m_dxgiFormat_;
  resourceDesc.Width            = m_desc_.width;
  resourceDesc.Height           = m_desc_.height;
  resourceDesc.DepthOrArraySize = (m_desc_.type == TextureType::TextureCube)
                                    ? 6
                                    : ((m_desc_.type == TextureType::Texture3D) ? m_desc_.depth : m_desc_.arraySize);
  resourceDesc.MipLevels        = m_desc_.mipLevels;

  // Set MSAA samples
  resourceDesc.SampleDesc.Count   = 1;  // Default to 1 sample
  resourceDesc.SampleDesc.Quality = 0;

  if (m_desc_.sampleCount == MSAASamples::Count2) {
    resourceDesc.SampleDesc.Count = 2;
  } else if (m_desc_.sampleCount == MSAASamples::Count4) {
    resourceDesc.SampleDesc.Count = 4;
  } else if (m_desc_.sampleCount == MSAASamples::Count8) {
    resourceDesc.SampleDesc.Count = 8;
  } else if (m_desc_.sampleCount == MSAASamples::Count16) {
    resourceDesc.SampleDesc.Count = 16;
  }

  // Set layout and flags
  resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
  resourceDesc.Flags  = D3D12_RESOURCE_FLAG_NONE;

  // Set resource flags based on create flags
  if (hasRtvUsage()) {
    resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
  }

  if (hasDsvUsage()) {
    resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
  }

  if (hasUavUsage()) {
    resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
  }

  // Determine heap type and properties
  D3D12_HEAP_PROPERTIES heapProps = {};
  heapProps.Type                  = D3D12_HEAP_TYPE_DEFAULT;  // GPU-local memory
  heapProps.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  heapProps.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
  heapProps.CreationNodeMask      = 1;
  heapProps.VisibleNodeMask       = 1;

  // Set optimized clear value for render targets and depth buffers
  D3D12_CLEAR_VALUE* clearValue          = nullptr;
  D3D12_CLEAR_VALUE  optimizedClearValue = {};

  if (hasRtvUsage()) {
    optimizedClearValue.Format = m_dxgiFormat_;
    clearValue                 = &optimizedClearValue;
  } else if (hasDsvUsage()) {
    optimizedClearValue.Format               = m_dxgiFormat_;
    optimizedClearValue.DepthStencil.Depth   = 1.0f;
    optimizedClearValue.DepthStencil.Stencil = 0;
    clearValue                               = &optimizedClearValue;
  }

  // Create the resource
  HRESULT hr = m_device_->getDevice()->CreateCommittedResource(
      &heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, m_currentState_, clearValue, IID_PPV_ARGS(&m_resource_));

  if (FAILED(hr)) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create DirectX 12 texture resource");
    return false;
  }

  return true;
}

bool TextureDx12::createViews_() {
  // Create render target view if needed
  if (hasRtvUsage()) {
    DescriptorHeap* rtvHeap = m_device_->getRtvHeap();
    if (!rtvHeap) {
      GlobalLogger::Log(LogLevel::Error, "RTV heap not available");
      return false;
    }

    m_rtvDescriptorIndex_ = rtvHeap->allocate();
    if (m_rtvDescriptorIndex_ == UINT32_MAX) {
      GlobalLogger::Log(LogLevel::Error, "Failed to allocate RTV descriptor");
      return false;
    }

    m_rtvHandle_ = rtvHeap->getCPUHandle(m_rtvDescriptorIndex_);

    // Create the RTV
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format                        = m_dxgiFormat_;

    // Set view dimension based on texture type
    switch (m_desc_.type) {
      case TextureType::Texture1D:
        rtvDesc.ViewDimension      = D3D12_RTV_DIMENSION_TEXTURE1D;
        rtvDesc.Texture1D.MipSlice = 0;
        break;
      case TextureType::Texture2D:
        if (m_desc_.sampleCount != MSAASamples::Count1) {
          rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
        } else {
          rtvDesc.ViewDimension        = D3D12_RTV_DIMENSION_TEXTURE2D;
          rtvDesc.Texture2D.MipSlice   = 0;
          rtvDesc.Texture2D.PlaneSlice = 0;
        }
        break;
      case TextureType::Texture2DArray:
      case TextureType::TextureCube:
        if (m_desc_.sampleCount != MSAASamples::Count1) {
          rtvDesc.ViewDimension                    = D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY;
          rtvDesc.Texture2DMSArray.FirstArraySlice = 0;
          rtvDesc.Texture2DMSArray.ArraySize       = m_desc_.arraySize;
        } else {
          rtvDesc.ViewDimension                  = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
          rtvDesc.Texture2DArray.MipSlice        = 0;
          rtvDesc.Texture2DArray.FirstArraySlice = 0;
          rtvDesc.Texture2DArray.ArraySize       = m_desc_.arraySize;
          rtvDesc.Texture2DArray.PlaneSlice      = 0;
        }
        break;
      case TextureType::Texture3D:
        rtvDesc.ViewDimension         = D3D12_RTV_DIMENSION_TEXTURE3D;
        rtvDesc.Texture3D.MipSlice    = 0;
        rtvDesc.Texture3D.FirstWSlice = 0;
        rtvDesc.Texture3D.WSize       = m_desc_.depth;
        break;
      default:
        GlobalLogger::Log(LogLevel::Error, "Unsupported texture type for RTV");
        return false;
    }

    m_device_->getDevice()->CreateRenderTargetView(m_resource_.Get(), &rtvDesc, m_rtvHandle_);
  }

  // Create depth stencil view if needed
  if (hasDsvUsage()) {
    DescriptorHeap* dsvHeap = m_device_->getDsvHeap();
    if (!dsvHeap) {
      GlobalLogger::Log(LogLevel::Error, "DSV heap not available");
      return false;
    }

    m_dsvDescriptorIndex_ = dsvHeap->allocate();
    if (m_dsvDescriptorIndex_ == UINT32_MAX) {
      GlobalLogger::Log(LogLevel::Error, "Failed to allocate DSV descriptor");
      return false;
    }

    m_dsvHandle_ = dsvHeap->getCPUHandle(m_dsvDescriptorIndex_);

    // Create the DSV
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format                        = m_dxgiFormat_;
    dsvDesc.Flags                         = D3D12_DSV_FLAG_NONE;

    // Set view dimension based on texture type
    switch (m_desc_.type) {
      case TextureType::Texture1D:
        dsvDesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE1D;
        dsvDesc.Texture1D.MipSlice = 0;
        break;
      case TextureType::Texture2D:
        if (m_desc_.sampleCount != MSAASamples::Count1) {
          dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
        } else {
          dsvDesc.ViewDimension      = D3D12_DSV_DIMENSION_TEXTURE2D;
          dsvDesc.Texture2D.MipSlice = 0;
        }
        break;
      case TextureType::Texture2DArray:
      case TextureType::TextureCube:
        if (m_desc_.sampleCount != MSAASamples::Count1) {
          dsvDesc.ViewDimension                    = D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY;
          dsvDesc.Texture2DMSArray.FirstArraySlice = 0;
          dsvDesc.Texture2DMSArray.ArraySize       = m_desc_.arraySize;
        } else {
          dsvDesc.ViewDimension                  = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
          dsvDesc.Texture2DArray.MipSlice        = 0;
          dsvDesc.Texture2DArray.FirstArraySlice = 0;
          dsvDesc.Texture2DArray.ArraySize       = m_desc_.arraySize;
        }
        break;
      default:
        GlobalLogger::Log(LogLevel::Error, "Unsupported texture type for DSV");
        return false;
    }

    m_device_->getDevice()->CreateDepthStencilView(m_resource_.Get(), &dsvDesc, m_dsvHandle_);
  }

  // Create shader resource view
  DescriptorHeap* srvUavHeap = m_device_->getCbvSrvUavHeap();
  if (!srvUavHeap) {
    GlobalLogger::Log(LogLevel::Error, "SRV/UAV heap not available");
    return false;
  }

  m_srvDescriptorIndex_ = srvUavHeap->allocate();
  if (m_srvDescriptorIndex_ == UINT32_MAX) {
    GlobalLogger::Log(LogLevel::Error, "Failed to allocate SRV descriptor");
    return false;
  }

  m_srvHandle_ = srvUavHeap->getCPUHandle(m_srvDescriptorIndex_);

  // Different format for depth textures when used as SRV
  DXGI_FORMAT srvFormat = m_dxgiFormat_;
  if (g_isDepthFormat(m_desc_.format)) {
    DXGI_FORMAT typelessFormat;
    g_getDepthFormatForSRV(typelessFormat, srvFormat, m_dxgiFormat_);
  }

  // Create the SRV
  D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Format                          = srvFormat;
  srvDesc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

  // Set view dimension based on texture type
  switch (m_desc_.type) {
    case TextureType::Texture1D:
      srvDesc.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE1D;
      srvDesc.Texture1D.MostDetailedMip     = 0;
      srvDesc.Texture1D.MipLevels           = m_desc_.mipLevels;
      srvDesc.Texture1D.ResourceMinLODClamp = 0.0f;
      break;
    case TextureType::Texture2D:
      if (m_desc_.sampleCount != MSAASamples::Count1) {
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
      } else {
        srvDesc.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip     = 0;
        srvDesc.Texture2D.MipLevels           = m_desc_.mipLevels;
        srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
        srvDesc.Texture2D.PlaneSlice          = 0;
      }
      break;
    case TextureType::Texture2DArray:
      if (m_desc_.sampleCount != MSAASamples::Count1) {
        srvDesc.ViewDimension                    = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
        srvDesc.Texture2DMSArray.FirstArraySlice = 0;
        srvDesc.Texture2DMSArray.ArraySize       = m_desc_.arraySize;
      } else {
        srvDesc.ViewDimension                      = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        srvDesc.Texture2DArray.MostDetailedMip     = 0;
        srvDesc.Texture2DArray.MipLevels           = m_desc_.mipLevels;
        srvDesc.Texture2DArray.FirstArraySlice     = 0;
        srvDesc.Texture2DArray.ArraySize           = m_desc_.arraySize;
        srvDesc.Texture2DArray.ResourceMinLODClamp = 0.0f;
        srvDesc.Texture2DArray.PlaneSlice          = 0;
      }
      break;
    case TextureType::TextureCube:
      srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURECUBE;
      srvDesc.TextureCube.MostDetailedMip     = 0;
      srvDesc.TextureCube.MipLevels           = m_desc_.mipLevels;
      srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
      break;
    case TextureType::Texture3D:
      srvDesc.ViewDimension                 = D3D12_SRV_DIMENSION_TEXTURE3D;
      srvDesc.Texture3D.MostDetailedMip     = 0;
      srvDesc.Texture3D.MipLevels           = m_desc_.mipLevels;
      srvDesc.Texture3D.ResourceMinLODClamp = 0.0f;
      break;
    default:
      GlobalLogger::Log(LogLevel::Error, "Unsupported texture type for SRV");
      return false;
  }

  m_device_->getDevice()->CreateShaderResourceView(m_resource_.Get(), &srvDesc, m_srvHandle_);

  // Create UAVs if needed
  if (hasUavUsage()) {
    // Create UAVs for each mip level
    m_uavHandles_.resize(m_desc_.mipLevels);
    m_uavDescriptorIndices_.resize(m_desc_.mipLevels, UINT32_MAX);

    for (uint32_t i = 0; i < m_desc_.mipLevels; i++) {
      m_uavDescriptorIndices_[i] = srvUavHeap->allocate();
      if (m_uavDescriptorIndices_[i] == UINT32_MAX) {
        GlobalLogger::Log(LogLevel::Error, "Failed to allocate UAV descriptor");
        continue;
      }

      m_uavHandles_[i] = srvUavHeap->getCPUHandle(m_uavDescriptorIndices_[i]);

      // Create the UAV
      D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
      uavDesc.Format                           = m_dxgiFormat_;

      // Set view dimension based on texture type
      switch (m_desc_.type) {
        case TextureType::Texture1D:
          uavDesc.ViewDimension      = D3D12_UAV_DIMENSION_TEXTURE1D;
          uavDesc.Texture1D.MipSlice = i;
          break;
        case TextureType::Texture2D:
          uavDesc.ViewDimension        = D3D12_UAV_DIMENSION_TEXTURE2D;
          uavDesc.Texture2D.MipSlice   = i;
          uavDesc.Texture2D.PlaneSlice = 0;
          break;
        case TextureType::Texture2DArray:
        case TextureType::TextureCube:
          uavDesc.ViewDimension                  = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
          uavDesc.Texture2DArray.MipSlice        = i;
          uavDesc.Texture2DArray.FirstArraySlice = 0;
          uavDesc.Texture2DArray.ArraySize       = m_desc_.arraySize;
          uavDesc.Texture2DArray.PlaneSlice      = 0;
          break;
        case TextureType::Texture3D:
          uavDesc.ViewDimension         = D3D12_UAV_DIMENSION_TEXTURE3D;
          uavDesc.Texture3D.MipSlice    = i;
          uavDesc.Texture3D.FirstWSlice = 0;
          uavDesc.Texture3D.WSize       = m_desc_.depth >> i;
          break;
        default:
          GlobalLogger::Log(LogLevel::Error, "Unsupported texture type for UAV");
          continue;
      }

      m_device_->getDevice()->CreateUnorderedAccessView(m_resource_.Get(), nullptr, &uavDesc, m_uavHandles_[i]);
    }
  }

  return true;
}

void TextureDx12::updateCurrentState(D3D12_RESOURCE_STATES state) {
  m_currentState_  = state;
  m_currentLayout_ = g_getResourceLayoutDx12(state);
}

D3D12_CPU_DESCRIPTOR_HANDLE TextureDx12::getRTVHandle(uint32_t mipLevel, uint32_t arraySlice) const {
  // For simplicity, we only support the default RTV handle for the first mip level and array slice
  // In a more complete implementation, we would create additional descriptors or a descriptor table
  if (mipLevel == 0 && arraySlice == 0) {
    return m_rtvHandle_;
  }

  // Return invalid handle for unsupported mip levels/array slices
  D3D12_CPU_DESCRIPTOR_HANDLE invalidHandle = {};
  return invalidHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE TextureDx12::getDSVHandle(uint32_t mipLevel, uint32_t arraySlice) const {
  // For simplicity, we only support the default DSV handle for the first mip level and array slice
  if (mipLevel == 0 && arraySlice == 0) {
    return m_dsvHandle_;
  }

  // Return invalid handle for unsupported mip levels/array slices
  D3D12_CPU_DESCRIPTOR_HANDLE invalidHandle = {};
  return invalidHandle;
}

D3D12_CPU_DESCRIPTOR_HANDLE TextureDx12::getUAVHandle(uint32_t mipLevel) const {
  // Return the UAV handle for the specified mip level if available
  if (mipLevel < m_uavHandles_.size()) {
    return m_uavHandles_[mipLevel];
  }

  // Return invalid handle for unsupported mip levels
  D3D12_CPU_DESCRIPTOR_HANDLE invalidHandle = {};
  return invalidHandle;
}

void TextureDx12::update(const void* data, size_t dataSize, uint32_t mipLevel, uint32_t arrayLayer) {
  if (!data || !dataSize) {
    GlobalLogger::Log(LogLevel::Error, "Invalid data or size for texture update");
    return;
  }

  // For DX12, we need to:
  // 1. Create a staging buffer in an upload heap
  // 2. Copy the data to the staging buffer
  // 3. Use a command buffer to copy from the staging buffer to the texture

  // Calculate footprint for the specified mip level
  uint32_t mipWidth  = std::max<uint32_t>(1, m_desc_.width >> mipLevel);
  uint32_t mipHeight = std::max<uint32_t>(1, m_desc_.height >> mipLevel);
  uint32_t mipDepth  = std::max<uint32_t>(1, m_desc_.depth >> mipLevel);

  D3D12_RESOURCE_DESC textureDesc = m_resource_->GetDesc();

  D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
  UINT                               numRows;
  UINT64                             rowSizeInBytes;
  UINT64                             totalBytes;

  // Get the required layout and size
  m_device_->getDevice()->GetCopyableFootprints(
      &textureDesc,
      D3D12CalcSubresource(mipLevel, arrayLayer, 0, m_desc_.mipLevels, m_desc_.arraySize),
      1,
      0,
      &footprint,
      &numRows,
      &rowSizeInBytes,
      &totalBytes);

  // Create a staging buffer in an upload heap
  ComPtr<ID3D12Resource> stagingBuffer;

  D3D12_HEAP_PROPERTIES uploadHeapProps = {};
  uploadHeapProps.Type                  = D3D12_HEAP_TYPE_UPLOAD;
  uploadHeapProps.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  uploadHeapProps.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
  uploadHeapProps.CreationNodeMask      = 1;
  uploadHeapProps.VisibleNodeMask       = 1;

  D3D12_RESOURCE_DESC bufferDesc = {};
  bufferDesc.Dimension           = D3D12_RESOURCE_DIMENSION_BUFFER;
  bufferDesc.Alignment           = 0;
  bufferDesc.Width               = totalBytes;
  bufferDesc.Height              = 1;
  bufferDesc.DepthOrArraySize    = 1;
  bufferDesc.MipLevels           = 1;
  bufferDesc.Format              = DXGI_FORMAT_UNKNOWN;
  bufferDesc.SampleDesc.Count    = 1;
  bufferDesc.SampleDesc.Quality  = 0;
  bufferDesc.Layout              = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  bufferDesc.Flags               = D3D12_RESOURCE_FLAG_NONE;

  HRESULT hr = m_device_->getDevice()->CreateCommittedResource(&uploadHeapProps,
                                                                 D3D12_HEAP_FLAG_NONE,
                                                                 &bufferDesc,
                                                                 D3D12_RESOURCE_STATE_GENERIC_READ,
                                                                 nullptr,
                                                                 IID_PPV_ARGS(&stagingBuffer));

  if (FAILED(hr)) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create staging buffer for texture update");
    return;
  }

  // Map the staging buffer
  void* mappedData = nullptr;
  hr               = stagingBuffer->Map(0, nullptr, &mappedData);

  if (FAILED(hr)) {
    GlobalLogger::Log(LogLevel::Error, "Failed to map staging buffer for texture update");
    return;
  }

  // Copy data to the staging buffer
  // For row-by-row copy with proper stride
  uint8_t* srcData = (uint8_t*)data;
  uint8_t* dstData = (uint8_t*)mappedData;

  for (UINT row = 0; row < numRows; row++) {
    memcpy(dstData + row * footprint.Footprint.RowPitch, srcData + row * rowSizeInBytes, rowSizeInBytes);
  }

  stagingBuffer->Unmap(0, nullptr);

  // Create a command buffer for the copy
  CommandBufferDesc cmdBufferDesc;
  cmdBufferDesc.primary = true;

  std::unique_ptr<CommandBuffer> commandBuffer = m_device_->createCommandBuffer(cmdBufferDesc);
  if (!commandBuffer) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create command buffer for texture update");
    return;
  }

  CommandBufferDx12* cmdBufferDx12 = static_cast<CommandBufferDx12*>(commandBuffer.get());

  // Begin the command buffer
  cmdBufferDx12->begin();

  // Transition the texture to COPY_DEST state
  ResourceBarrierDesc barrier = {};
  barrier.texture             = this;
  barrier.oldLayout           = m_currentLayout_;
  barrier.newLayout           = ResourceLayout::TransferDst;
  cmdBufferDx12->resourceBarrier(barrier);

  // Copy from buffer to texture
  D3D12_TEXTURE_COPY_LOCATION src = {};
  src.pResource                   = stagingBuffer.Get();
  src.Type                        = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
  src.PlacedFootprint             = footprint;

  D3D12_TEXTURE_COPY_LOCATION dst = {};
  dst.pResource                   = m_resource_.Get();
  dst.Type                        = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
  dst.SubresourceIndex            = D3D12CalcSubresource(mipLevel, arrayLayer, 0, m_desc_.mipLevels, m_desc_.arraySize);

  cmdBufferDx12->getCommandList()->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

  // Transition the texture back to its original state
  barrier.oldLayout = ResourceLayout::TransferDst;
  barrier.newLayout = m_currentLayout_;
  cmdBufferDx12->resourceBarrier(barrier);

  // End the command buffer
  cmdBufferDx12->end();

  // Create a fence to wait for the upload to complete
  FenceDesc fenceDesc;
  fenceDesc.signaled = false;
  auto fence         = m_device_->createFence(fenceDesc);

  // Submit the command buffer
  m_device_->submitCommandBuffer(commandBuffer.get(), fence.get());

  // Wait for the upload to complete
  fence->wait();

  // The staging buffer will be released when it goes out of scope
}

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12