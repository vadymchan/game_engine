#ifndef GAME_ENGINE_DESCRIPTOR_DX12_H
#define GAME_ENGINE_DESCRIPTOR_DX12_H

#include "gfx/rhi/rhi_new/interface/descriptor.h"
#include "platform/windows/windows_platform_setup.h"

#ifdef GAME_ENGINE_RHI_DX12

#include <vector>

namespace game_engine {
namespace gfx {
namespace rhi {

class DeviceDx12;
class BufferDx12;
class TextureDx12;
class SamplerDx12;

/**
 * In DirectX 12, this corresponds to a portion of the root signature.
 * For educational purposes, we map Vulkan's descriptor set concept to
 * a segment of descriptors in a DX12 descriptor table.
 */
class DescriptorSetLayoutDx12 : public DescriptorSetLayout {
  public:
  DescriptorSetLayoutDx12(const DescriptorSetLayoutDesc& desc, DeviceDx12* device);
  ~DescriptorSetLayoutDx12() = default;

  DescriptorSetLayoutDx12(const DescriptorSetLayoutDx12&)            = delete;
  DescriptorSetLayoutDx12& operator=(const DescriptorSetLayoutDx12&) = delete;

  // DX12-specific methods
  const std::vector<D3D12_DESCRIPTOR_RANGE>& getDescriptorRanges() const { return m_descriptorRanges_; }

  uint32_t getDescriptorCount() const { return m_descriptorCount_; }

  private:
  friend class DescriptorSetDx12; // for accessing m_bindingMap_

  DeviceDx12*                         m_device_ = nullptr;
  std::vector<D3D12_DESCRIPTOR_RANGE> m_descriptorRanges_;
  uint32_t                            m_descriptorCount_ = 0;

  // Track binding to offset mapping for descriptor creation
  struct BindingInfo {
    uint32_t                    offset;
    D3D12_DESCRIPTOR_RANGE_TYPE type;
  };

  std::unordered_map<uint32_t, BindingInfo> m_bindingMap_;
};

/**
 * In DirectX 12, this corresponds to a region of a descriptor heap
 * that contains the actual descriptors (CBV/SRV/UAV and samplers).
 */
class DescriptorSetDx12 : public DescriptorSet {
  public:
  DescriptorSetDx12(DeviceDx12* device, const DescriptorSetLayoutDx12* layout);
  ~DescriptorSetDx12() override;

  DescriptorSetDx12(const DescriptorSetDx12&)            = delete;
  DescriptorSetDx12& operator=(const DescriptorSetDx12&) = delete;

  // Resource binding methods
  void setUniformBuffer(uint32_t binding, Buffer* buffer, uint64_t offset = 0, uint64_t range = 0) override;
  void setTextureSampler(uint32_t binding, Texture* texture, Sampler* sampler) override;
  void setTexture(uint32_t binding, Texture* texture, ResourceLayout layout = ResourceLayout::ShaderReadOnly) override;
  void setSampler(uint32_t binding, Sampler* sampler) override;
  void setStorageBuffer(uint32_t binding, Buffer* buffer, uint64_t offset = 0, uint64_t range = 0) override;

  // DX12-specific methods
  D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle() const { return m_gpuHandle_; }

  private:
  D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandleForBinding_(uint32_t binding) const;

  DeviceDx12*                    m_device_ = nullptr;
  const DescriptorSetLayoutDx12* m_layout_ = nullptr;

  uint32_t                    m_descriptorIndex_ = UINT32_MAX;  // Base index in descriptor heap
  D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle_       = {};          // Base CPU handle
  D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle_       = {};          // Base GPU handle

  // Maintain a map of binding to offset for quick lookups
  std::unordered_map<uint32_t, uint32_t> m_bindingOffsets_;
};

class DescriptorHeap {
  public:
  DescriptorHeap() = default;
  ~DescriptorHeap();

  bool initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t count, bool shaderVisible);
  void release();

  ID3D12DescriptorHeap* getHeap() const { return m_heap_.Get(); }

  D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandle(uint32_t index) const;
  D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle(uint32_t index) const;

  uint32_t allocate();
  void     free(uint32_t index);

  uint32_t getDescriptorSize() const { return m_descriptorSize_; }

  uint32_t getCapacity() const { return m_capacity_; }

  uint32_t getAllocatedCount() const { return m_allocated_; }

  private:
  ComPtr<ID3D12DescriptorHeap> m_heap_;
  std::vector<bool>            m_freeList_;
  uint32_t                     m_descriptorSize_ = 0;
  uint32_t                     m_capacity_       = 0;
  uint32_t                     m_allocated_      = 0;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12

#endif  // GAME_ENGINE_DESCRIPTOR_DX12_H