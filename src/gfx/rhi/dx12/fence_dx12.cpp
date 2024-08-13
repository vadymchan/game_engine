#include "gfx/rhi/dx12/fence_dx12.h"
#include "gfx/rhi/dx12/rhi_dx12.h"


#include <cassert>

namespace game_engine {

FenceDx12::~FenceDx12() {
  if (m_fence) {
    m_fence->Release();
    m_fence = nullptr;
  }

  if (FenceEvent) {
    CloseHandle(FenceEvent);
    FenceEvent = nullptr;
  }
}

bool FenceDx12::IsComplete() const {
  return m_fence->GetCompletedValue() >= FenceValue;
}

bool FenceDx12::IsComplete(uint64_t InFenceValue) const {
  return m_fence->GetCompletedValue() >= InFenceValue;
}


void FenceDx12::WaitForFence(uint64_t InTimeoutNanoSec) {
  WaitForFenceValue(FenceValue, InTimeoutNanoSec);
}

void FenceDx12::WaitForFenceValue(uint64_t InFenceValue,
                                    uint64_t InTimeoutNanoSec) {
  if (InFenceValue == InitialFenceValue) {
    return;
  }

  assert(m_fence);
  if (UINT64_MAX == InTimeoutNanoSec) {
    while (m_fence->GetCompletedValue() < InFenceValue) {
      Sleep(0);
    }
  } else {
    std::chrono::system_clock::time_point lastTime
        = std::chrono::system_clock::now();

    while (m_fence->GetCompletedValue() < InFenceValue) {
      std::chrono::nanoseconds elapsed_nanoseconds
          = std::chrono::duration_cast<std::chrono::nanoseconds>(
              std::chrono::system_clock::now() - lastTime);
      if (elapsed_nanoseconds.count() >= (int64_t)InTimeoutNanoSec) {
        return;
      }

      Sleep(0);
    }
  }
}

uint64_t FenceDx12::SignalWithNextFenceValue(
    ID3D12CommandQueue* InCommandQueue, bool bWaitUntilExecuteComplete) {
  {
    ScopedLock s(&FenceValueLock);

    const auto NewFenceValue = FenceValue + 1;
    HRESULT    hr = InCommandQueue->Signal(m_fence.Get(), NewFenceValue);
    assert(SUCCEEDED(hr));

    if (FAILED(hr)) {
      return FenceValue;
    }

    FenceValue = NewFenceValue;
  }

  HRESULT hr = m_fence->SetEventOnCompletion(FenceValue, FenceEvent);
  assert(SUCCEEDED(hr));

  if (FAILED(hr)) {
    WaitForSingleObjectEx(FenceEvent, INFINITE, false);
  }
  return FenceValue;
}

Fence* FenceManagerDx12::GetOrCreateFence() {
  if (PendingFences.size() > 0) {
    Fence* fence = *PendingFences.begin();
    PendingFences.erase(PendingFences.begin());
    UsingFences.insert(fence);
    return fence;
  }

  assert(g_rhi_dx12);
  assert(g_rhi_dx12->Device);

  FenceDx12* newFence = new FenceDx12();
  HRESULT hr = g_rhi_dx12->Device->CreateFence(FenceDx12::InitialFenceValue,
                                               D3D12_FENCE_FLAG_NONE,
                                               IID_PPV_ARGS(&newFence->m_fence));
  assert(SUCCEEDED(hr));

  if (SUCCEEDED(hr)) {
    newFence->FenceEvent = ::CreateEvent(nullptr, false, false, nullptr);
    if (newFence->FenceEvent) {
      UsingFences.insert(newFence);
      return newFence;
    }
  }

  assert(0);

  delete newFence;
  return nullptr;
}

void FenceManagerDx12::Release() {
  for (auto& fence : UsingFences) {
    delete fence;
  }
  UsingFences.clear();

  for (auto& fence : PendingFences) {
    delete fence;
  }
  PendingFences.clear();
}

}  // namespace game_engine