#ifndef GAME_ENGINE_COMMAND_ALLOCATOR_DX12_H
#define GAME_ENGINE_COMMAND_ALLOCATOR_DX12_H

#include "gfx/rhi/command_buffer_manager.h"
#include "gfx/rhi/dx12/command_list_dx12.h"
#include "gfx/rhi/dx12/fence_dx12.h"
#include "gfx/rhi/lock.h"
#include "platform/windows/windows_platform_setup.h"

#include <vector>

namespace game_engine {
// TODO: consider renaming this class to CommandAllocatorDx12
class CommandBufferManagerDx12 : public CommandBufferManager {
  public:
  CommandBufferManagerDx12()
      : m_commandListType_(D3D12_COMMAND_LIST_TYPE_DIRECT)
  //, FenceValue(0)
  {}

  virtual ~CommandBufferManagerDx12() {}

  virtual void Release() override;

  virtual CommandBufferDx12* GetOrCreateCommandBuffer() override;
  virtual void ReturnCommandBuffer(CommandBuffer* commandBuffer) override;

  bool Initialize(ComPtr<ID3D12Device>    InDevice,
                  D3D12_COMMAND_LIST_TYPE type
                  = D3D12_COMMAND_LIST_TYPE_DIRECT);

  ComPtr<ID3D12CommandQueue> GetCommandQueue() const { return m_commandQueue_; }

  // CommandList
  void ExecuteCommandList(CommandBufferDx12* commandList,
                          bool               bWaitUntilExecuteComplete = false);

  // Not destroying because it is referenced by the Fence manager
  FenceDx12* m_fence = nullptr;

  private:
  ComPtr<ID3D12CommandAllocator> CreateCommandAllocator() const {
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    if (FAILED(m_device_->CreateCommandAllocator(
            m_commandListType_, IID_PPV_ARGS(&commandAllocator)))) {
      return nullptr;
    }

    return commandAllocator;
  }

  CommandBufferDx12* CreateCommandList() const;

  MutexLock                               m_commandListLock_;
  std::vector<CommandBufferDx12*>         m_availableCommandLists_;
  mutable std::vector<CommandBufferDx12*> m_usingCommandBuffers_;

  D3D12_COMMAND_LIST_TYPE m_commandListType_ = D3D12_COMMAND_LIST_TYPE_DIRECT;
  ComPtr<ID3D12Device>    m_device_;
  ComPtr<ID3D12CommandQueue> m_commandQueue_;
};
}  // namespace game_engine

#endif  // GAME_ENGINE_COMMAND_ALLOCATOR_DX12_H