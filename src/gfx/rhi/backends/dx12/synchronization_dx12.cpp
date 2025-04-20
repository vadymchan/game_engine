#include "gfx/rhi/backends/dx12/synchronization_dx12.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/backends/dx12/device_dx12.h"
#include "utils/logger/global_logger.h"

namespace game_engine {
namespace gfx {
namespace rhi {

//-------------------------------------------------------------------------
// FenceDx12 implementation
//-------------------------------------------------------------------------

FenceDx12::FenceDx12(const FenceDesc& desc, DeviceDx12* device)
    : Fence(desc)
    , m_device_(device)
    , m_fenceValue_(desc.signaled ? 1 : 0) {
  // Create fence
  HRESULT hr = device->getDevice()->CreateFence(m_fenceValue_, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence_));

  if (FAILED(hr)) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create DirectX12 fence");
    return;
  }

  // Create event for CPU-side waiting
  m_event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
  if (m_event_ == nullptr) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create fence event handle");
  }
}

FenceDx12::~FenceDx12() {
  if (m_event_) {
    CloseHandle(m_event_);
    m_event_ = nullptr;
  }
}

void FenceDx12::reset() {
  // In DirectX12, we don't reset the fence itself, but we can reset our tracking
  // by setting a new value to wait for
  m_signaled_ = false;
}

bool FenceDx12::wait(uint64_t timeout) {
  // If the fence is already at or beyond our value, it's signaled
  if (m_fence_->GetCompletedValue() >= m_fenceValue_) {
    m_signaled_ = true;
    return true;
  }

  // Otherwise, set up the event and wait
  HRESULT hr = m_fence_->SetEventOnCompletion(m_fenceValue_, m_event_);
  if (FAILED(hr)) {
    GlobalLogger::Log(LogLevel::Error, "Failed to set fence completion event");
    return false;
  }

  // Convert timeout from nanoseconds to milliseconds
  constexpr auto kNanosecondsPerMillisecond = 1'000'000;
  DWORD timeoutMs = (timeout == UINT64_MAX) ? INFINITE : static_cast<DWORD>(timeout / kNanosecondsPerMillisecond);

  DWORD result = WaitForSingleObject(m_event_, timeoutMs);
  if (result == WAIT_OBJECT_0) {
    m_signaled_ = true;
    return true;
  }

  return false;
}

bool FenceDx12::isSignaled() {
  if (m_fence_->GetCompletedValue() >= m_fenceValue_) {
    m_signaled_ = true;
    return true;
  }

  return false;
}

void FenceDx12::signal(ID3D12CommandQueue* queue) {
  // Increment the value to signal
  m_fenceValue_++;

  // Signal the fence with the new value
  HRESULT hr = queue->Signal(m_fence_.Get(), m_fenceValue_);
  if (FAILED(hr)) {
    GlobalLogger::Log(LogLevel::Error, "Failed to signal DirectX12 fence");
  }
}

//-------------------------------------------------------------------------
// SemaphoreDx12 implementation
//-------------------------------------------------------------------------

SemaphoreDx12::SemaphoreDx12(DeviceDx12* device)
    : m_device_(device) {
  // Create a fence object for GPU-GPU synchronization
  HRESULT hr = device->getDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence_));
  if (FAILED(hr)) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create DirectX12 fence for semaphore");
  }

  // Create event handle for CPU-side waiting (if needed)
  m_event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);
  if (m_event_ == nullptr) {
    GlobalLogger::Log(LogLevel::Error, "Failed to create event for DirectX12 semaphore");
  }
}

SemaphoreDx12::~SemaphoreDx12() {
  if (m_event_) {
    CloseHandle(m_event_);
    m_event_ = nullptr;
  }
}

void SemaphoreDx12::signal(ID3D12CommandQueue* queue) {
  // Increment value for this signal operation
  m_value_++;

  // Signal the fence with the new value
  HRESULT hr = queue->Signal(m_fence_.Get(), m_value_);
  if (FAILED(hr)) {
    GlobalLogger::Log(LogLevel::Error, "Failed to signal DirectX12 semaphore");
  }
}

void SemaphoreDx12::wait() {
  // If fence value is already reached, we don't need to wait
  if (m_fence_->GetCompletedValue() < m_value_) {
    // Otherwise, set up event to wait for fence completion
    HRESULT hr = m_fence_->SetEventOnCompletion(m_value_, m_event_);
    if (FAILED(hr)) {
      GlobalLogger::Log(LogLevel::Error, "Failed to set completion event for DirectX12 semaphore");
      return;
    }

    // Wait for the event
    WaitForSingleObject(m_event_, INFINITE);
  }
}

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12