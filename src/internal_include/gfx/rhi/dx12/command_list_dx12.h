#ifndef GAME_ENGINE_COMMAND_LIST_DX12_H
#define GAME_ENGINE_COMMAND_LIST_DX12_H

#include "gfx/rhi/command_buffer_manager.h"
#include "platform/windows/windows_platform_setup.h"

namespace game_engine {

// TODO: consider renaming to command list
struct jCommandBuffer_DX12 : public jCommandBuffer {
  ComPtr<ID3D12CommandAllocator>     CommandAllocator;
  ComPtr<ID3D12GraphicsCommandList4> CommandList;
  mutable bool                       IsClosed = false;

  class jOnlineDescriptorHeap_DX12* OnlineDescriptorHeap        = nullptr;
  class jOnlineDescriptorHeap_DX12* OnlineSamplerDescriptorHeap = nullptr;

  ID3D12GraphicsCommandList4* Get() { return CommandList.Get(); }

  bool IsValid() const { return CommandList.Get(); }

  inline ComPtr<ID3D12GraphicsCommandList4>& GetRef() { return CommandList; }

  virtual bool Begin() const override;
  virtual bool End() const override;
  virtual void Reset() const override;

  virtual void* GetNativeHandle() const override { return CommandList.Get(); }

  virtual void*   GetFenceHandle() const override;
  virtual void    SetFence(void* InFence) override;
  virtual jFence* GetFence() const override;

  const class jCommandBufferManager_DX12* Owner      = nullptr;
  uint64_t                                FenceValue = 0;
  bool                                    IsCompleteForWaitFence();
};

}  // namespace game_engine

#endif  // GAME_ENGINE_COMMAND_LIST_DX12_H