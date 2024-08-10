#include "gfx/rhi/dx12/ring_buffer_dx12.h"

#include "gfx/rhi/dx12/rhi_dx12.h"

namespace game_engine {

void jRingBuffer_DX12::Create(uint64_t totalSize, uint32_t alignment /*= 16*/) {
  ScopedLock s(&Lock);

  Release();

  RingBufferSize   = Align(totalSize, (uint64_t)alignment);
  RingBufferOffset = 0;
  Alignment        = alignment;

  D3D12_RESOURCE_DESC desc = {};
  desc.Dimension           = D3D12_RESOURCE_DIMENSION_BUFFER;
  desc.Width               = uint32_t(RingBufferSize);
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
  Buffer = g_rhi_dx12->CreateUploadResource(&desc,
                                            D3D12_RESOURCE_STATE_GENERIC_READ);

  // TODO: refactor
  auto resource = Buffer->Resource;
  assert(resource);

  if (!resource) {
    return;
  }

  {
    assert(!CBV.IsValid());
    CBV = g_rhi_dx12->DescriptorHeaps.Alloc();

    D3D12_CONSTANT_BUFFER_VIEW_DESC Desc;
    Desc.BufferLocation = Buffer->GetGPUVirtualAddress();
    Desc.SizeInBytes    = (uint32_t)RingBufferSize;

    g_rhi_dx12->Device->CreateConstantBufferView(&Desc, CBV.CPUHandle);
  }

  D3D12_RANGE readRange = {};
  HRESULT     hr        = Buffer->Resource.get()->Get()->Map(
      0, &readRange, reinterpret_cast<void**>(&MappedPointer));
  assert(SUCCEEDED(hr));

  if (FAILED(hr)) {
    return;
  }
}

}  // namespace game_engine