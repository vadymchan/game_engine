#include "gfx/rhi/dx12/fence_dx12.h"
#include "gfx/rhi/dx12/rhi_dx12.h"


#include <cassert>

namespace game_engine {

FenceDx12::~FenceDx12() {
  if (m_fence_) {
    m_fence_->Release();
    m_fence_ = nullptr;
  }

  if (m_fenceEvent_) {
    CloseHandle(m_fenceEvent_);
    m_fenceEvent_ = nullptr;
  }
}

bool FenceDx12::isComplete() const {
  return m_fence_->GetCompletedValue() >= m_fenceValue_;
}

bool FenceDx12::isComplete(uint64_t fenceValue) const {
  return m_fence_->GetCompletedValue() >= fenceValue;
}


void FenceDx12::waitForFence(uint64_t timeoutNanoSec) {
  waitForFenceValue(m_fenceValue_, timeoutNanoSec);
}

void FenceDx12::waitForFenceValue(uint64_t fenceValue,
                                    uint64_t timeoutNanoSec) {
  if (fenceValue == s_kInitialFenceValue) {
    return;
  }

  assert(m_fence_);
  if (UINT64_MAX == timeoutNanoSec) {
    while (m_fence_->GetCompletedValue() < fenceValue) {
      Sleep(0);
    }
  } else {
    std::chrono::system_clock::time_point lastTime
        = std::chrono::system_clock::now();

    while (m_fence_->GetCompletedValue() < fenceValue) {
      std::chrono::nanoseconds elapsed_nanoseconds
          = std::chrono::duration_cast<std::chrono::nanoseconds>(
              std::chrono::system_clock::now() - lastTime);
      if (elapsed_nanoseconds.count() >= (int64_t)timeoutNanoSec) {
        return;
      }

      Sleep(0);
    }
  }
}

uint64_t FenceDx12::signalWithNextFenceValue(
    ID3D12CommandQueue* commandQueue, bool waitUntilExecuteComplete) {
  {
    ScopedLock s(&m_fenceValueLock_);

    const auto NewFenceValue = m_fenceValue_ + 1;
    HRESULT    hr = commandQueue->Signal(m_fence_.Get(), NewFenceValue);
    assert(SUCCEEDED(hr));

    if (FAILED(hr)) {
      return m_fenceValue_;
    }

    m_fenceValue_ = NewFenceValue;
  }

  HRESULT hr = m_fence_->SetEventOnCompletion(m_fenceValue_, m_fenceEvent_);
  assert(SUCCEEDED(hr));

  if (FAILED(hr)) {
    WaitForSingleObjectEx(m_fenceEvent_, INFINITE, false);
  }
  return m_fenceValue_;
}

IFence* FenceManagerDx12::getOrCreateFence() {
  if (m_pendingFences_.size() > 0) {
    IFence* fence = *m_pendingFences_.begin();
    m_pendingFences_.erase(m_pendingFences_.begin());
    m_usingFences_.insert(fence);
    return fence;
  }

  assert(g_rhiDx12);
  assert(g_rhiDx12->m_device_);

  FenceDx12* newFence = new FenceDx12();
  HRESULT hr = g_rhiDx12->m_device_->CreateFence(FenceDx12::s_kInitialFenceValue,
                                               D3D12_FENCE_FLAG_NONE,
                                               IID_PPV_ARGS(&newFence->m_fence_));
  assert(SUCCEEDED(hr));

  if (SUCCEEDED(hr)) {
    newFence->m_fenceEvent_ = ::CreateEvent(nullptr, false, false, nullptr);
    if (newFence->m_fenceEvent_) {
      m_usingFences_.insert(newFence);
      return newFence;
    }
  }

  assert(0);

  delete newFence;
  return nullptr;
}

void FenceManagerDx12::release() {
  for (auto& fence : m_usingFences_) {
    delete fence;
  }
  m_usingFences_.clear();

  for (auto& fence : m_pendingFences_) {
    delete fence;
  }
  m_pendingFences_.clear();
}

}  // namespace game_engine