#ifndef GAME_ENGINE_FENCE_DX12_H
#define GAME_ENGINE_FENCE_DX12_H

#include "gfx/rhi/fence_manager.h"
#include "gfx/rhi/lock.h"
#include "platform/windows/windows_platform_setup.h"

#include <unordered_set>

namespace game_engine {

class FenceDx12 : public IFence {
  public:
  static constexpr uint64_t s_kInitialFenceValue = uint64_t(0);

  virtual ~FenceDx12() override;

  virtual void* getHandle() const override { return m_fence_.Get(); }

  virtual bool isValid() const override { return m_fence_ && m_fenceEvent_; }

  virtual bool isComplete() const override;
  bool         isComplete(uint64_t fenceValue) const override;

  virtual void waitForFence(uint64_t timeoutNanoSec = UINT64_MAX) override;

  void waitForFenceValue(uint64_t fenceValue,
                         uint64_t timeoutNanoSec = UINT64_MAX);

  uint64_t signalWithNextFenceValue(ID3D12CommandQueue* commandQueue,
                                    bool waitUntilExecuteComplete = false);

  ComPtr<ID3D12Fence> m_fence_;
  HANDLE              m_fenceEvent_ = nullptr;

  MutexLock m_fenceValueLock_;
  uint64_t  m_fenceValue_ = s_kInitialFenceValue;
};

class FenceManagerDx12 : public IFenceManager {
  public:
  virtual IFence* getOrCreateFence() override;

  virtual void returnFence(IFence* fence) override {
    m_usingFences_.erase(fence);
    m_pendingFences_.insert(fence);
  }

  virtual void release() override;

  std::unordered_set<IFence*> m_usingFences_;
  std::unordered_set<IFence*> m_pendingFences_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_FENCE_DX12_H