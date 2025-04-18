#include "gfx/rhi/rhi_new/backends/dx12/descriptor_dx12.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/rhi_new/backends/dx12/buffer_dx12.h"
#include "gfx/rhi/rhi_new/backends/dx12/device_dx12.h"
#include "gfx/rhi/rhi_new/backends/dx12/rhi_enums_dx12.h"
#include "gfx/rhi/rhi_new/backends/dx12/sampler_dx12.h"
#include "gfx/rhi/rhi_new/backends/dx12/texture_dx12.h"
#include "utils/logger/global_logger.h"

namespace game_engine {
namespace gfx {
namespace rhi {

//-------------------------------------------------------------------------
// DescriptorSetLayoutDx12 implementation
//-------------------------------------------------------------------------

DescriptorSetLayoutDx12::DescriptorSetLayoutDx12(const DescriptorSetLayoutDesc& desc, DeviceDx12* device)
    : DescriptorSetLayout(desc)
    , m_device_(device) {
  // Group bindings by type for creating descriptor ranges
  std::unordered_map<D3D12_DESCRIPTOR_RANGE_TYPE, std::vector<uint32_t>> bindingsByType;

  // Track current offset for each binding
  uint32_t currentOffset = 0;

  // First, organize bindings by type and track their offsets
  for (const auto& binding : desc.bindings) {
    D3D12_DESCRIPTOR_RANGE_TYPE rangeType = g_getShaderBindingTypeDx12(binding.type);

    // Store binding info for later use
    BindingInfo info;
    info.offset                    = currentOffset;
    info.type                      = rangeType;
    m_bindingMap_[binding.binding] = info;

    // Group by type for descriptor ranges
    bindingsByType[rangeType].push_back(binding.binding);

    // Increment offset based on descriptor count
    currentOffset += binding.descriptorCount;
  }

  // Total number of descriptors
  m_descriptorCount_ = currentOffset;

  // Create descriptor ranges for each type
  for (const auto& [rangeType, bindings] : bindingsByType) {
    if (bindings.empty()) {
      continue;
    }

    D3D12_DESCRIPTOR_RANGE range            = {};
    range.RangeType                         = rangeType;
    range.NumDescriptors                    = static_cast<UINT>(bindings.size());
    range.BaseShaderRegister                = bindings[0];  // Start with the lowest binding
    range.RegisterSpace                     = 0;            // Use register space 0 for simplicity
    range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    m_descriptorRanges_.push_back(range);
  }
}

//-------------------------------------------------------------------------
// DescriptorSetDx12 implementation
//-------------------------------------------------------------------------

DescriptorSetDx12::DescriptorSetDx12(DeviceDx12* device, const DescriptorSetLayoutDx12* layout)
    : m_device_(device)
    , m_layout_(layout) {
  // Allocate descriptors from the device's descriptor heap
  DescriptorHeap* cbvSrvUavHeap = device->getCbvSrvUavHeap();
  if (!cbvSrvUavHeap) {
    GlobalLogger::Log(LogLevel::Error, "Failed to get CBV/SRV/UAV descriptor heap");
    return;
  }

  // Allocate a contiguous block of descriptors
  uint32_t descriptorCount = layout->getDescriptorCount();
  if (descriptorCount == 0) {
    return;  // No descriptors needed
  }

  m_descriptorIndex_ = cbvSrvUavHeap->allocate();
  if (m_descriptorIndex_ == UINT32_MAX) {
    GlobalLogger::Log(LogLevel::Error, "Failed to allocate descriptors from heap");
    return;
  }

  // Get CPU and GPU handles for the allocated descriptor
  m_cpuHandle_ = cbvSrvUavHeap->getCPUHandle(m_descriptorIndex_);
  m_gpuHandle_ = cbvSrvUavHeap->getGPUHandle(m_descriptorIndex_);

  // Set up binding offset map
  for (const auto& [binding, info] : layout->m_bindingMap_) {
    m_bindingOffsets_[binding] = info.offset;
  }
}

DescriptorSetDx12::~DescriptorSetDx12() {
  // Free allocated descriptors
  if (m_device_ && m_descriptorIndex_ != UINT32_MAX) {
    DescriptorHeap* cbvSrvUavHeap = m_device_->getCbvSrvUavHeap();
    if (cbvSrvUavHeap) {
      cbvSrvUavHeap->free(m_descriptorIndex_);
    }
    m_descriptorIndex_ = UINT32_MAX;
  }
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorSetDx12::getCPUHandleForBinding_(uint32_t binding) const {
  auto it = m_bindingOffsets_.find(binding);
  if (it == m_bindingOffsets_.end()) {
    GlobalLogger::Log(LogLevel::Error, "Binding not found in descriptor set layout");
    return m_cpuHandle_;  // Return base handle as fallback
  }

  // Calculate CPU handle with offset
  D3D12_CPU_DESCRIPTOR_HANDLE handle  = m_cpuHandle_;
  handle.ptr                         += it->second * m_device_->getCbvSrvUavHeap()->getDescriptorSize();
  return handle;
}

void DescriptorSetDx12::setUniformBuffer(uint32_t binding, Buffer* buffer, uint64_t offset, uint64_t range) {
  if (!buffer) {
    GlobalLogger::Log(LogLevel::Error, "Null buffer in setUniformBuffer");
    return;
  }

  BufferDx12* bufferDx12 = dynamic_cast<BufferDx12*>(buffer);
  if (!bufferDx12) {
    GlobalLogger::Log(LogLevel::Error, "Invalid buffer type in setUniformBuffer");
    return;
  }

  // Get CPU handle for this binding
  D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = getCPUHandleForBinding_(binding);

  // Create constant buffer view
  D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
  cbvDesc.BufferLocation                  = bufferDx12->getGPUVirtualAddress() + offset;
  cbvDesc.SizeInBytes                     = static_cast<UINT>((range == 0) ? bufferDx12->getSize() - offset : range);

  // DX12 requires CBV sizes to be aligned to 256 bytes
  cbvDesc.SizeInBytes = (cbvDesc.SizeInBytes + 255) & ~255;

  m_device_->getDevice()->CreateConstantBufferView(&cbvDesc, cpuHandle);
}

void DescriptorSetDx12::setTextureSampler(uint32_t binding, Texture* texture, Sampler* sampler) {
  // DX12 doesn't have combined texture samplers, so we need to bind them separately

  // Bind texture to specified binding
  setTexture(binding, texture);

  // For simplicity, we'll assume the sampler binding is the next consecutive binding
  // In a real engine, you'd track sampler bindings separately or derive them from the layout
  setSampler(binding + 1, sampler);
}

void DescriptorSetDx12::setTexture(uint32_t binding, Texture* texture, ResourceLayout layout) {
  if (!texture) {
    GlobalLogger::Log(LogLevel::Error, "Null texture in setTexture");
    return;
  }

  TextureDx12* textureDx12 = dynamic_cast<TextureDx12*>(texture);
  if (!textureDx12) {
    GlobalLogger::Log(LogLevel::Error, "Invalid texture type in setTexture");
    return;
  }

  // Get CPU handle for this binding
  D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = getCPUHandleForBinding_(binding);

  // Get the texture's SRV handle
  D3D12_CPU_DESCRIPTOR_HANDLE srvHandle = textureDx12->getSRVHandle();

  // For efficiency, if the texture already has an SRV, copy it
  if (srvHandle.ptr != 0) {
    m_device_->getDevice()->CopyDescriptorsSimple(1, cpuHandle, srvHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
  } else {
    // Otherwise, create a new SRV
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                          = textureDx12->getDxgiFormat();
    srvDesc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    // Configure view dimension based on texture type
    switch (textureDx12->getType()) {
      case TextureType::Texture1D:
        srvDesc.ViewDimension       = D3D12_SRV_DIMENSION_TEXTURE1D;
        srvDesc.Texture1D.MipLevels = textureDx12->getMipLevels();
        break;

      case TextureType::Texture2D:
        if (textureDx12->getDesc().sampleCount != MSAASamples::Count1) {
          srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
        } else {
          srvDesc.ViewDimension       = D3D12_SRV_DIMENSION_TEXTURE2D;
          srvDesc.Texture2D.MipLevels = textureDx12->getMipLevels();
        }
        break;

      case TextureType::Texture2DArray:
        if (textureDx12->getDesc().sampleCount != MSAASamples::Count1) {
          srvDesc.ViewDimension              = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
          srvDesc.Texture2DMSArray.ArraySize = textureDx12->getArraySize();
        } else {
          srvDesc.ViewDimension            = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
          srvDesc.Texture2DArray.MipLevels = textureDx12->getMipLevels();
          srvDesc.Texture2DArray.ArraySize = textureDx12->getArraySize();
        }
        break;

      case TextureType::TextureCube:
        srvDesc.ViewDimension         = D3D12_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MipLevels = textureDx12->getMipLevels();
        break;

      case TextureType::Texture3D:
        srvDesc.ViewDimension       = D3D12_SRV_DIMENSION_TEXTURE3D;
        srvDesc.Texture3D.MipLevels = textureDx12->getMipLevels();
        break;

      default:
        GlobalLogger::Log(LogLevel::Error, "Unsupported texture type for SRV");
        return;
    }

    m_device_->getDevice()->CreateShaderResourceView(textureDx12->getResource(), &srvDesc, cpuHandle);
  }
}

void DescriptorSetDx12::setSampler(uint32_t binding, Sampler* sampler) {
  if (!sampler) {
    GlobalLogger::Log(LogLevel::Error, "Null sampler in setSampler");
    return;
  }

  SamplerDx12* samplerDx12 = dynamic_cast<SamplerDx12*>(sampler);
  if (!samplerDx12) {
    GlobalLogger::Log(LogLevel::Error, "Invalid sampler type in setSampler");
    return;
  }

  // Samplers need special handling because they're in a separate heap
  DescriptorHeap* samplerHeap = m_device_->getSamplerHeap();
  if (!samplerHeap) {
    GlobalLogger::Log(LogLevel::Error, "Sampler heap not available");
    return;
  }

  // Allocate a descriptor from the sampler heap
  uint32_t samplerIndex = samplerHeap->allocate();
  if (samplerIndex == UINT32_MAX) {
    GlobalLogger::Log(LogLevel::Error, "Failed to allocate sampler descriptor");
    return;
  }

  // Get the CPU handle for the sampler descriptor
  D3D12_CPU_DESCRIPTOR_HANDLE samplerHandle = samplerHeap->getCPUHandle(samplerIndex);

  // Copy the sampler descriptor
  if (samplerDx12->isValid()) {
    m_device_->getDevice()->CopyDescriptorsSimple(
        1, samplerHandle, samplerDx12->getCpuHandle(), D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
  } else {
    GlobalLogger::Log(LogLevel::Error, "Invalid sampler descriptor");
    samplerHeap->free(samplerIndex);
  }

  // Note: In a more complete implementation, we would track allocated sampler descriptors
  // and free them in the destructor. For this educational example, we're keeping it simple.
}

void DescriptorSetDx12::setStorageBuffer(uint32_t binding, Buffer* buffer, uint64_t offset, uint64_t range) {
  if (!buffer) {
    GlobalLogger::Log(LogLevel::Error, "Null buffer in setStorageBuffer");
    return;
  }

  BufferDx12* bufferDx12 = dynamic_cast<BufferDx12*>(buffer);
  if (!bufferDx12) {
    GlobalLogger::Log(LogLevel::Error, "Invalid buffer type in setStorageBuffer");
    return;
  }

  // Get CPU handle for this binding
  D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = getCPUHandleForBinding_(binding);

  // Create unordered access view
  D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
  uavDesc.Format                           = DXGI_FORMAT_UNKNOWN;
  uavDesc.ViewDimension                    = D3D12_UAV_DIMENSION_BUFFER;
  uavDesc.Buffer.FirstElement              = offset / sizeof(uint32_t);  // Treat as uint array
  uavDesc.Buffer.NumElements
      = static_cast<UINT>((range == 0 ? (bufferDx12->getSize() - offset) : range) / sizeof(uint32_t));
  uavDesc.Buffer.StructureByteStride  = 0;                               // Not a structured buffer
  uavDesc.Buffer.CounterOffsetInBytes = 0;
  uavDesc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE;

  m_device_->getDevice()->CreateUnorderedAccessView(bufferDx12->getResource(), nullptr, &uavDesc, cpuHandle);
}

//-------------------------------------------------------------------------
// DescriptorHeap implementation
//-------------------------------------------------------------------------

DescriptorHeap::~DescriptorHeap() {
  release();
}

bool DescriptorHeap::initialize(ID3D12Device*              device,
                                D3D12_DESCRIPTOR_HEAP_TYPE type,
                                uint32_t                   count,
                                bool                       shaderVisible) {
  release();

  m_capacity_  = count;
  m_allocated_ = 0;
  m_freeList_.resize(count, true);  // make all slots free initially

  D3D12_DESCRIPTOR_HEAP_DESC desc = {};
  desc.Type                       = type;
  desc.NumDescriptors             = count;
  desc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

  HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap_));
  if (FAILED(hr)) {
    return false;
  }

  m_descriptorSize_ = device->GetDescriptorHandleIncrementSize(type);
  return true;
}

void DescriptorHeap::release() {
  m_heap_.Reset();
  m_freeList_.clear();
  m_capacity_       = 0;
  m_allocated_      = 0;
  m_descriptorSize_ = 0;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::getCPUHandle(uint32_t index) const {
  D3D12_CPU_DESCRIPTOR_HANDLE handle  = m_heap_->GetCPUDescriptorHandleForHeapStart();
  handle.ptr                         += index * m_descriptorSize_;
  return handle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::getGPUHandle(uint32_t index) const {
  D3D12_GPU_DESCRIPTOR_HANDLE handle  = m_heap_->GetGPUDescriptorHandleForHeapStart();
  handle.ptr                         += index * m_descriptorSize_;
  return handle;
}

uint32_t DescriptorHeap::allocate() {
  for (uint32_t i = 0; i < m_capacity_; ++i) {
    if (m_freeList_[i]) {
      m_freeList_[i] = false;
      m_allocated_++;
      return i;
    }
  }
  // TODO: log - no free descriptors available
  return UINT32_MAX;
}

void DescriptorHeap::free(uint32_t index) {
  if (index < m_capacity_ && !m_freeList_[index]) {
    m_freeList_[index] = true;
    m_allocated_--;
  }
}

#endif  // GAME_ENGINE_RHI_DX12

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine