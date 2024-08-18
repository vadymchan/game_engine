#include "gfx/rhi/dx12/ring_buffer_dx12.h"

#include "gfx/rhi/dx12/rhi_dx12.h"

namespace game_engine {

void RingBufferDx12::Create(uint64_t totalSize, uint32_t alignment /*= 16*/) {
  ScopedLock s(&m_lock_);

  Release();

  m_ringBufferSize_   = Align(totalSize, (uint64_t)alignment);
  m_ringBufferOffset_ = 0;
  m_alignment_        = alignment;

  D3D12_RESOURCE_DESC desc = {};
  desc.Dimension           = D3D12_RESOURCE_DIMENSION_BUFFER;
  desc.Width               = uint32_t(m_ringBufferSize_);
  desc.Height              = 1;
  desc.DepthOrArraySize    = 1;
  desc.MipLevels           = 1;
  desc.Format              = DXGI_FORMAT_UNKNOWN;
  desc.Flags               = D3D12_RESOURCE_FLAG_NONE;
  desc.SampleDesc.Count    = 1;
  desc.SampleDesc.Quality  = 0;
  desc.Layout              = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  desc.Alignment           = 0;

  assert(g_rhi_dx12);
  m_buffer_ = g_rhi_dx12->CreateUploadResource(&desc,
                                            D3D12_RESOURCE_STATE_GENERIC_READ);

  // TODO: refactor
  auto resource = m_buffer_->m_resource_;
  assert(resource);

  if (!resource) {
    return;
  }

  {
    assert(!m_cbv_.IsValid());
    m_cbv_ = g_rhi_dx12->m_descriptorHeaps_.Alloc();

    D3D12_CONSTANT_BUFFER_VIEW_DESC Desc;
    Desc.BufferLocation = m_buffer_->GetGPUVirtualAddress();
    Desc.SizeInBytes    = (uint32_t)m_ringBufferSize_;

    g_rhi_dx12->m_device_->CreateConstantBufferView(&Desc, m_cbv_.m_cpuHandle_);
  }

  D3D12_RANGE readRange = {};
  HRESULT     hr        = m_buffer_->m_resource_.get()->Get()->Map(
      0, &readRange, reinterpret_cast<void**>(&m_mappedPointer_));
  assert(SUCCEEDED(hr));

  if (FAILED(hr)) {
    return;
  }
}

}  // namespace game_engine