#ifndef GAME_ENGINE_BUFFER_DX12_H
#define GAME_ENGINE_BUFFER_DX12_H

#include "gfx/rhi/rhi_new/interface/buffer.h"
#include "platform/windows/windows_platform_setup.h"

#ifdef GAME_ENGINE_RHI_DX12

namespace game_engine {
namespace gfx {
namespace rhi {

class DeviceDx12;

class BufferDx12 : public Buffer {
  public:
  BufferDx12(const BufferDesc& desc, DeviceDx12* device);
  ~BufferDx12() override;

  ID3D12Resource* getResource() const { return m_resource_.Get(); }

  uint32_t getStride() const { return m_stride_; }

  void setStride(uint32_t stride) { m_stride_ = stride; }

  D3D12_GPU_VIRTUAL_ADDRESS getGPUVirtualAddress() const {
    return m_resource_ ? m_resource_->GetGPUVirtualAddress() : 0;
  }

  D3D12_RESOURCE_STATES getCurrentState() const { return m_currentState_; }

  void updateCurrentState_(D3D12_RESOURCE_STATES newState) { m_currentState_ = newState; }

  bool isMapped() const { return m_isMapped_; }

  void* getMappedData() const { return m_mappedData_; }

  private:
  friend class DeviceDx12;
  // These methods are called from DeviceDx12
  bool update_(const void* data, size_t size, size_t offset);
  bool map_(void** ppData, D3D12_RANGE* pReadRange);
  void unmap_(D3D12_RANGE* pWrittenRange);

  bool isUploadHeapBuffer_() const;

  bool isReadbackHeapBuffer_() const;

  D3D12_RESOURCE_STATES getInitialResourceState_() const;

  DeviceDx12*            m_device_ = nullptr;
  ComPtr<ID3D12Resource> m_resource_;
  uint32_t               m_stride_       = 0;        // For vertex buffers
  void*                  m_mappedData_   = nullptr;  // For persistently mapped buffers
  bool                   m_isMapped_     = false;
  D3D12_RESOURCE_STATES  m_currentState_ = D3D12_RESOURCE_STATE_COMMON;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12
#endif  // GAME_ENGINE_BUFFER_DX12_H