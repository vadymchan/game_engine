#include "gfx/rhi/dx12/fence_dx12.h"
#include "gfx/rhi/dx12/rhi_dx12.h"


#include <cassert>

namespace game_engine {

jFence_DX12::~jFence_DX12() {
  if (Fence) {
    Fence->Release();
    Fence = nullptr;
  }

  if (FenceEvent) {
    CloseHandle(FenceEvent);
    FenceEvent = nullptr;
  }
}

bool jFence_DX12::IsComplete() const {
  return Fence->GetCompletedValue() >= FenceValue;
}

bool jFence_DX12::IsComplete(uint64_t InFenceValue) const {
  return Fence->GetCompletedValue() >= InFenceValue;
}


void jFence_DX12::WaitForFence(uint64_t InTimeoutNanoSec) {
  WaitForFenceValue(FenceValue, InTimeoutNanoSec);
}

void jFence_DX12::WaitForFenceValue(uint64_t InFenceValue,
                                    uint64_t InTimeoutNanoSec) {
  if (InFenceValue == InitialFenceValue) {
    return;
  }

  assert(Fence);
  if (UINT64_MAX == InTimeoutNanoSec) {
    while (Fence->GetCompletedValue() < InFenceValue) {
      Sleep(0);
    }
  } else {
    std::chrono::system_clock::time_point lastTime
        = std::chrono::system_clock::now();

    while (Fence->GetCompletedValue() < InFenceValue) {
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

uint64_t jFence_DX12::SignalWithNextFenceValue(
    ID3D12CommandQueue* InCommandQueue, bool bWaitUntilExecuteComplete) {
  {
    ScopedLock s(&FenceValueLock);

    const auto NewFenceValue = FenceValue + 1;
    HRESULT    hr = InCommandQueue->Signal(Fence.Get(), NewFenceValue);
    assert(SUCCEEDED(hr));

    if (FAILED(hr)) {
      return FenceValue;
    }

    FenceValue = NewFenceValue;
  }

  HRESULT hr = Fence->SetEventOnCompletion(FenceValue, FenceEvent);
  assert(SUCCEEDED(hr));

  if (FAILED(hr)) {
    WaitForSingleObjectEx(FenceEvent, INFINITE, false);
  }
  return FenceValue;
}

jFence* jFenceManager_DX12::GetOrCreateFence() {
  if (PendingFences.size() > 0) {
    jFence* fence = *PendingFences.begin();
    PendingFences.erase(PendingFences.begin());
    UsingFences.insert(fence);
    return fence;
  }

  assert(g_rhi_dx12);
  assert(g_rhi_dx12->Device);

  jFence_DX12* newFence = new jFence_DX12();
  HRESULT hr = g_rhi_dx12->Device->CreateFence(jFence_DX12::InitialFenceValue,
                                               D3D12_FENCE_FLAG_NONE,
                                               IID_PPV_ARGS(&newFence->Fence));
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

void jFenceManager_DX12::Release() {
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