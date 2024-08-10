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
class jCommandBufferManager_DX12 : public jCommandBufferManager {
  public:
  jCommandBufferManager_DX12()
      : CommandListType(D3D12_COMMAND_LIST_TYPE_DIRECT)
  //, FenceValue(0)
  {}

  virtual ~jCommandBufferManager_DX12() {}

  virtual void Release() override;

  virtual jCommandBuffer_DX12* GetOrCreateCommandBuffer() override;
  virtual void ReturnCommandBuffer(jCommandBuffer* commandBuffer) override;

  bool Initialize(ComPtr<ID3D12Device>    InDevice,
                  D3D12_COMMAND_LIST_TYPE InType
                  = D3D12_COMMAND_LIST_TYPE_DIRECT);

  ComPtr<ID3D12CommandQueue> GetCommandQueue() const { return CommandQueue; }

  // CommandList
  void ExecuteCommandList(jCommandBuffer_DX12* InCommandList,
                          bool bWaitUntilExecuteComplete = false);

  // Not destroying because it is referenced by the Fence manager
  jFence_DX12* Fence = nullptr;

  private:
  ComPtr<ID3D12CommandAllocator> CreateCommandAllocator() const {
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    if (FAILED(Device->CreateCommandAllocator(
            CommandListType, IID_PPV_ARGS(&commandAllocator)))) {
      return nullptr;
    }

    return commandAllocator;
  }

  jCommandBuffer_DX12* CreateCommandList() const;

  MutexLock                                 CommandListLock;
  std::vector<jCommandBuffer_DX12*>         AvailableCommandLists;
  mutable std::vector<jCommandBuffer_DX12*> UsingCommandBuffers;

  D3D12_COMMAND_LIST_TYPE    CommandListType = D3D12_COMMAND_LIST_TYPE_DIRECT;
  ComPtr<ID3D12Device>       Device;
  ComPtr<ID3D12CommandQueue> CommandQueue;
};
}  // namespace game_engine

#endif  // GAME_ENGINE_COMMAND_ALLOCATOR_DX12_H