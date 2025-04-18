#include "gfx/rhi/rhi_new/backends/dx12/buffer_dx12.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/rhi_new/backends/dx12/device_dx12.h"
#include "utils/logger/global_logger.h"

namespace game_engine {
namespace gfx {
namespace rhi {

BufferDx12::BufferDx12(const BufferDesc& desc, DeviceDx12* device)
    : Buffer(desc)
    , m_device_(device) {
  // Determine heap type and resource state
  D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT;  // GPU-only memory by default
  m_currentState_          = getInitialResourceState_();

  if (isUploadHeapBuffer_()) {
    heapType        = D3D12_HEAP_TYPE_UPLOAD;
    m_currentState_ = D3D12_RESOURCE_STATE_GENERIC_READ;
  } else if (isReadbackHeapBuffer_()) {
    heapType        = D3D12_HEAP_TYPE_READBACK;
    m_currentState_ = D3D12_RESOURCE_STATE_COPY_DEST;
  }

  // Create buffer resource
  D3D12_HEAP_PROPERTIES heapProps = {};
  heapProps.Type                  = heapType;
  heapProps.CPUPageProperty       = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
  heapProps.MemoryPoolPreference  = D3D12_MEMORY_POOL_UNKNOWN;
  heapProps.CreationNodeMask      = 1;
  heapProps.VisibleNodeMask       = 1;

  D3D12_RESOURCE_DESC resourceDesc = {};
  resourceDesc.Dimension           = D3D12_RESOURCE_DIMENSION_BUFFER;
  resourceDesc.Alignment           = 0;
  resourceDesc.Width               = m_desc_.size;
  resourceDesc.Height              = 1;
  resourceDesc.DepthOrArraySize    = 1;
  resourceDesc.MipLevels           = 1;
  resourceDesc.Format              = DXGI_FORMAT_UNKNOWN;
  resourceDesc.SampleDesc.Count    = 1;
  resourceDesc.SampleDesc.Quality  = 0;
  resourceDesc.Layout              = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
  resourceDesc.Flags               = D3D12_RESOURCE_FLAG_NONE;

  // Add UAV flag if needed
  if ((m_desc_.createFlags & BufferCreateFlag::Uav) != BufferCreateFlag::None) {
    resourceDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
  }

  HRESULT hr = device->getDevice()->CreateCommittedResource(
      &heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc, m_currentState_, nullptr, IID_PPV_ARGS(&m_resource_));

  if (FAILED(hr)) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create DirectX 12 buffer");
    return;
  }

  // For upload heap buffers (CPU-accessible), map buffer memory immediately
  if (isUploadHeapBuffer_()) {
    D3D12_RANGE readRange = {0, 0};  // We do not intend to read
    hr                    = m_resource_->Map(0, &readRange, &m_mappedData_);

    if (FAILED(hr)) {
      GlobalLogger::Log(LogLevel::Error, "Failed to map buffer memory");
      m_mappedData_ = nullptr;
    } else {
      m_isMapped_ = true;
    }
  }
}

BufferDx12::~BufferDx12() {
  // Unmap if mapped
  if (m_isMapped_ && m_resource_) {
    D3D12_RANGE writtenRange = {0, static_cast<SIZE_T>(m_desc_.size)};
    m_resource_->Unmap(0, &writtenRange);
    m_mappedData_ = nullptr;
    m_isMapped_   = false;
  }

  // ComPtr automatically releases the resource
}

bool BufferDx12::update_(const void* data, size_t size, size_t offset) {
  if (!data || size == 0) {
    return false;
  }

  // Check that size + offset does not exceed buffer size
  if (offset + size > m_desc_.size) {
    GlobalLogger::Log(LogLevel::Error, "Buffer update exceeds buffer size");
    return false;
  }

  // If this is an upload heap buffer, update it directly
  if (m_isMapped_ && m_mappedData_) {
    char* mappedCharPtr = static_cast<char*>(m_mappedData_);
    memcpy(mappedCharPtr + offset, data, size);
    return true;
  } else if (isUploadHeapBuffer_() && !m_isMapped_) {
    // Attempt to map if it's an upload heap buffer but not currently mapped
    D3D12_RANGE readRange  = {0, 0};  // We do not intend to read
    void*       mappedData = nullptr;

    HRESULT hr = m_resource_->Map(0, &readRange, &mappedData);
    if (SUCCEEDED(hr)) {
      // Copy data
      char* mappedCharPtr = static_cast<char*>(mappedData);
      memcpy(mappedCharPtr + offset, data, size);

      // Unmap
      D3D12_RANGE writtenRange = {static_cast<SIZE_T>(offset), static_cast<SIZE_T>(offset + size)};
      m_resource_->Unmap(0, &writtenRange);
      return true;
    } else {
      GlobalLogger::Log(LogLevel::Error, "Failed to map buffer for update");
      return false;
    }
  }

  // For default heap buffers, a staging buffer and copy is used (implemented in DeviceDx12::updateBuffer)
  return false;
}

bool BufferDx12::map_(void** ppData, D3D12_RANGE* pReadRange) {
  if (!ppData) {
    return false;
  }

  if (m_isMapped_) {
    *ppData = m_mappedData_;
    return true;
  }

  if (!m_resource_) {
    return false;
  }

  if (!isUploadHeapBuffer_() && !isReadbackHeapBuffer_()) {
    GlobalLogger::Log(LogLevel::Error, "Cannot map a non-mappable buffer");
    return false;
  }

  HRESULT hr = m_resource_->Map(0, pReadRange, ppData);
  if (FAILED(hr)) {
    GlobalLogger::Log(LogLevel::Error, "Failed to map buffer");
    return false;
  }

  m_mappedData_ = *ppData;
  m_isMapped_   = true;
  return true;
}

void BufferDx12::unmap_(D3D12_RANGE* pWrittenRange) {
  if (!m_isMapped_ || !m_resource_) {
    return;
  }

  m_resource_->Unmap(0, pWrittenRange);
  m_mappedData_ = nullptr;
  m_isMapped_   = false;
}

bool BufferDx12::isUploadHeapBuffer_() const {
  // Buffers accessible by CPU or dynamic buffers are created in the upload heap
  return ((m_desc_.createFlags & BufferCreateFlag::CpuAccess) != BufferCreateFlag::None)
      || (m_desc_.type == BufferType::Dynamic);
}

bool BufferDx12::isReadbackHeapBuffer_() const {
  // Readback buffers are created in the readback heap
  return (m_desc_.createFlags & BufferCreateFlag::Readback) != BufferCreateFlag::None;
}

D3D12_RESOURCE_STATES BufferDx12::getInitialResourceState_() const {
  // Default resource state based on buffer usage
  if ((m_desc_.createFlags & BufferCreateFlag::VertexBuffer) != BufferCreateFlag::None) {
    return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
  }

  if ((m_desc_.createFlags & BufferCreateFlag::IndexBuffer) != BufferCreateFlag::None) {
    return D3D12_RESOURCE_STATE_INDEX_BUFFER;
  }

  if ((m_desc_.createFlags & BufferCreateFlag::Uav) != BufferCreateFlag::None) {
    return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
  }

  if ((m_desc_.createFlags & BufferCreateFlag::IndirectCommand) != BufferCreateFlag::None) {
    return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
  }

  if ((m_desc_.createFlags & BufferCreateFlag::Readback) != BufferCreateFlag::None) {
    return D3D12_RESOURCE_STATE_COPY_DEST;
  }

  // Default to constant buffer state for uniform buffers
  return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
}

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12
