#ifndef GAME_ENGINE_FENCE_DX12_H
#define GAME_ENGINE_FENCE_DX12_H

#include "gfx/rhi/fence_manager.h"
#include "gfx/rhi/lock.h"
#include "platform/windows/windows_platform_setup.h"

#include <unordered_set>

namespace game_engine {

class jFence_DX12 : public jFence {
  public:
  static constexpr uint64_t InitialFenceValue = uint64_t(0);

  virtual ~jFence_DX12() override;

  virtual void* GetHandle() const override { return Fence.Get(); }

  virtual bool IsValid() const override { return Fence && FenceEvent; }

  virtual bool IsComplete() const override;
  bool         IsComplete(uint64_t InFenceValue) const override;

  virtual void WaitForFence(uint64_t InTimeoutNanoSec = UINT64_MAX) override;

  void         WaitForFenceValue(uint64_t InFenceValue,
                                 uint64_t InTimeoutNanoSec = UINT64_MAX);

  uint64_t SignalWithNextFenceValue(ID3D12CommandQueue* InCommandQueue,
                                    bool bWaitUntilExecuteComplete = false);

  ComPtr<ID3D12Fence> Fence;
  HANDLE              FenceEvent = nullptr;

  MutexLock FenceValueLock;
  uint64_t   FenceValue = InitialFenceValue;
};

class jFenceManager_DX12 : public jFenceManager {
  public:
  virtual jFence* GetOrCreateFence() override;

  virtual void ReturnFence(jFence* fence) override {
    UsingFences.erase(fence);
    PendingFences.insert(fence);
  }

  virtual void Release() override;

  std::unordered_set<jFence*> UsingFences;
  std::unordered_set<jFence*> PendingFences;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_FENCE_DX12_H