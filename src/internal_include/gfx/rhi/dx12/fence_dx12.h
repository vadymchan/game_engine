#ifndef GAME_ENGINE_FENCE_DX12_H
#define GAME_ENGINE_FENCE_DX12_H

#include "gfx/rhi/fence_manager.h"
#include "gfx/rhi/lock.h"
#include "platform/windows/windows_platform_setup.h"

#include <unordered_set>

namespace game_engine {

class FenceDx12 : public Fence {
  public:
  static constexpr uint64_t s_kInitialFenceValue = uint64_t(0);

  virtual ~FenceDx12() override;

  virtual void* GetHandle() const override { return m_fence_.Get(); }

  virtual bool IsValid() const override { return m_fence_ && m_fenceEvent_; }

  virtual bool IsComplete() const override;
  bool         IsComplete(uint64_t fenceValue) const override;

  virtual void WaitForFence(uint64_t timeoutNanoSec = UINT64_MAX) override;

  void WaitForFenceValue(uint64_t fenceValue,
                         uint64_t timeoutNanoSec = UINT64_MAX);

  uint64_t SignalWithNextFenceValue(ID3D12CommandQueue* commandQueue,
                                    bool bWaitUntilExecuteComplete = false);

  ComPtr<ID3D12Fence> m_fence_;
  HANDLE              m_fenceEvent_ = nullptr;

  MutexLock m_fenceValueLock_;
  uint64_t  m_fenceValue_ = s_kInitialFenceValue;
};

class FenceManagerDx12 : public FenceManager {
  public:
  virtual Fence* GetOrCreateFence() override;

  virtual void ReturnFence(Fence* fence) override {
    UsingFences.erase(fence);
    PendingFences.insert(fence);
  }

  virtual void Release() override;

  std::unordered_set<Fence*> UsingFences;
  std::unordered_set<Fence*> PendingFences;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_FENCE_DX12_H