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
class CommandBufferManagerDx12 : public ICommandBufferManager {
  public:
  // ======= BEGIN: public constructors =======================================

  CommandBufferManagerDx12()
      : m_commandListType_(D3D12_COMMAND_LIST_TYPE_DIRECT)
  //, FenceValue(0)
  {}

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~CommandBufferManagerDx12() {}

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual void release() override;
  virtual void returnCommandBuffer(CommandBuffer* commandBuffer) override;

  virtual CommandBufferDx12* getOrCreateCommandBuffer() override;

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public getters ============================================

  ComPtr<ID3D12CommandQueue> getCommandQueue() const { return m_commandQueue_; }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  bool initialize(ComPtr<ID3D12Device>    device,
                  D3D12_COMMAND_LIST_TYPE type
                  = D3D12_COMMAND_LIST_TYPE_DIRECT);

  // CommandList
  void executeCommandList(CommandBufferDx12* commandList,
                          bool               waitUntilExecuteComplete = false);

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public misc fields ========================================

  // Not destroying because it is referenced by the Fence manager
  FenceDx12* m_fence_ = nullptr;

  // ======= END: public misc fields   ========================================

  private:
  // ======= BEGIN: private misc methods ======================================

  ComPtr<ID3D12CommandAllocator> createCommandAllocator_() const {
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    if (FAILED(m_device_->CreateCommandAllocator(
            m_commandListType_, IID_PPV_ARGS(&commandAllocator)))) {
      return nullptr;
    }

    return commandAllocator;
  }

  CommandBufferDx12* createCommandList_() const;

  // ======= END: private misc methods   ======================================

  // ======= BEGIN: private misc fields =======================================

  MutexLock                               m_commandListLock_;
  std::vector<CommandBufferDx12*>         m_availableCommandLists_;
  mutable std::vector<CommandBufferDx12*> m_usingCommandBuffers_;

  D3D12_COMMAND_LIST_TYPE m_commandListType_ = D3D12_COMMAND_LIST_TYPE_DIRECT;
  ComPtr<ID3D12Device>    m_device_;
  ComPtr<ID3D12CommandQueue> m_commandQueue_;

  // ======= END: private misc fields   =======================================
};
}  // namespace game_engine

#endif  // GAME_ENGINE_COMMAND_ALLOCATOR_DX12_H