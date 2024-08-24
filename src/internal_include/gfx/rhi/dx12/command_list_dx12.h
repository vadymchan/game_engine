#ifndef GAME_ENGINE_COMMAND_LIST_DX12_H
#define GAME_ENGINE_COMMAND_LIST_DX12_H

#include "gfx/rhi/command_buffer_manager.h"
#include "platform/windows/windows_platform_setup.h"

namespace game_engine {

// TODO: consider renaming to command list
struct CommandBufferDx12 : public CommandBuffer {
  ID3D12GraphicsCommandList4* get() { return m_commandList_.Get(); }

  bool isValid() const { return m_commandList_.Get(); }

  inline ComPtr<ID3D12GraphicsCommandList4>& getRef() { return m_commandList_; }

  virtual bool begin() const override;
  virtual bool end() const override;
  virtual void reset() const override;

  virtual void* getNativeHandle() const override { return m_commandList_.Get(); }

  virtual void*  getFenceHandle() const override;
  virtual void   setFence(void* fence) override;
  virtual IFence* getFence() const override;

  bool isCompleteForWaitFence();

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