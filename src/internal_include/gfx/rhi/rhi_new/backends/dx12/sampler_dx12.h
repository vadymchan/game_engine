#ifndef GAME_ENGINE_SAMPLER_DX12_H
#define GAME_ENGINE_SAMPLER_DX12_H

#include "gfx/rhi/rhi_new/interface/sampler.h"
#include "platform/windows/windows_platform_setup.h"

#ifdef GAME_ENGINE_RHI_DX12

namespace game_engine {
namespace gfx {
namespace rhi {

class DeviceDx12;

class SamplerDx12 : public Sampler {
  public:
  SamplerDx12(const SamplerDesc& desc, DeviceDx12* device);
  ~SamplerDx12() override;

  SamplerDx12(const SamplerDx12&)            = delete;
  SamplerDx12& operator=(const SamplerDx12&) = delete;

  // DirectX 12-specific methods
  D3D12_CPU_DESCRIPTOR_HANDLE getCpuHandle() const { return m_cpuHandle_; }

  D3D12_GPU_DESCRIPTOR_HANDLE getGpuHandle() const { return m_gpuHandle_; }

  bool isValid() const { return m_descriptorIndex_ != UINT32_MAX; }

  private:
  DeviceDx12*                 m_device_;
  uint32_t                    m_descriptorIndex_ = UINT32_MAX;
  D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle_       = {};
  D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle_       = {};
  D3D12_SAMPLER_DESC          m_dx12Desc_        = {};
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12
#endif  // GAME_ENGINE_SAMPLER_DX12_H