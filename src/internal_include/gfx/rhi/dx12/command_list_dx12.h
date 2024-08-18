#ifndef GAME_ENGINE_COMMAND_LIST_DX12_H
#define GAME_ENGINE_COMMAND_LIST_DX12_H

#include "gfx/rhi/command_buffer_manager.h"
#include "platform/windows/windows_platform_setup.h"

namespace game_engine {

// TODO: consider renaming to command list
struct CommandBufferDx12 : public CommandBuffer {
  ID3D12GraphicsCommandList4* Get() { return m_commandList_.Get(); }

  bool IsValid() const { return m_commandList_.Get(); }

  inline ComPtr<ID3D12GraphicsCommandList4>& GetRef() { return m_commandList_; }

  virtual bool Begin() const override;
  virtual bool End() const override;
  virtual void Reset() const override;

  virtual void* GetNativeHandle() const override { return m_commandList_.Get(); }

  virtual void*  GetFenceHandle() const override;
  virtual void   SetFence(void* fence) override;
  virtual Fence* GetFence() const override;

  bool IsCompleteForWaitFence();

  ComPtr<ID3D12CommandAllocator>     m_commandAllocator_;
  ComPtr<ID3D12GraphicsCommandList4> m_commandList_;
  mutable bool                       m_isClosed_ = false;

  class OnlineDescriptorHeapDx12* m_onlineDescriptorHeap_        = nullptr;
  class OnlineDescriptorHeapDx12* m_onlineSamplerDescriptorHeap_ = nullptr;

  const class CommandBufferManagerDx12* m_owner_      = nullptr;
  uint64_t                              m_fenceValue_ = 0;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_COMMAND_LIST_DX12_H