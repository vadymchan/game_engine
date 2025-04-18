#ifndef GAME_ENGINE_TEXTURE_DX12_H
#define GAME_ENGINE_TEXTURE_DX12_H

#include "gfx/rhi/rhi_new/interface/texture.h"
#include "platform/windows/windows_platform_setup.h"

#ifdef GAME_ENGINE_RHI_DX12

namespace game_engine {
namespace gfx {
namespace rhi {

class DeviceDx12;

class TextureDx12 : public Texture {
  public:

  TextureDx12(const TextureDesc& desc, DeviceDx12* device);

  /**
   * Create a texture from an existing resource (for swap chain images)
   */
  TextureDx12(DeviceDx12*                 device,
              const TextureDesc&          desc,
              ID3D12Resource*             existingResource,
              D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = {},
              D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = {});

  ~TextureDx12() override;

  TextureDx12(const TextureDx12&)            = delete;
  TextureDx12& operator=(const TextureDx12&) = delete;

  // DX12-specific methods
  ID3D12Resource* getResource() const { return m_resource_.Get(); }

  DXGI_FORMAT getDxgiFormat() const { return m_dxgiFormat_; }

  D3D12_RESOURCE_STATES getCurrentState() const { return m_currentState_; }

  D3D12_CPU_DESCRIPTOR_HANDLE getRTVHandle(uint32_t mipLevel = 0, uint32_t arraySlice = 0) const;
  D3D12_CPU_DESCRIPTOR_HANDLE getDSVHandle(uint32_t mipLevel = 0, uint32_t arraySlice = 0) const;

  D3D12_CPU_DESCRIPTOR_HANDLE getSRVHandle() const { return m_srvHandle_; }

  D3D12_CPU_DESCRIPTOR_HANDLE getUAVHandle(uint32_t mipLevel = 0) const;

  // Updates texture data (for staging uploads)
  void update(const void* data, size_t dataSize, uint32_t mipLevel = 0, uint32_t arrayLayer = 0);

  private:
  friend class CommandBufferDx12;
  // Only CommandBufferDx12 should update state through barriers
  void updateCurrentState(D3D12_RESOURCE_STATES state);

  bool createResource_();
  bool createViews_();



  DeviceDx12* m_device_;

  // DirectX 12 resources
  ComPtr<ID3D12Resource> m_resource_;

  // Track if we own this resource (false for swapchain images)
  bool m_ownsResource_ = true;

  // DirectX 12 format
  DXGI_FORMAT m_dxgiFormat_ = DXGI_FORMAT_UNKNOWN;

  // Track the current resource state
  D3D12_RESOURCE_STATES m_currentState_ = D3D12_RESOURCE_STATE_COMMON;

  // Descriptor handles
  D3D12_CPU_DESCRIPTOR_HANDLE              m_rtvHandle_ = {};
  D3D12_CPU_DESCRIPTOR_HANDLE              m_dsvHandle_ = {};
  D3D12_CPU_DESCRIPTOR_HANDLE              m_srvHandle_ = {};
  std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_uavHandles_;

  // Descriptor heap allocations
  uint32_t              m_rtvDescriptorIndex_ = UINT32_MAX;
  uint32_t              m_dsvDescriptorIndex_ = UINT32_MAX;
  uint32_t              m_srvDescriptorIndex_ = UINT32_MAX;
  std::vector<uint32_t> m_uavDescriptorIndices_;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12
#endif  // GAME_ENGINE_TEXTURE_DX12_H