#include "gfx/rhi/dx12/command_allocator_dx12.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/dx12/rhi_dx12.h"
#include "gfx/rhi/rhi.h"

#include <cassert>

namespace game_engine {

void CommandBufferManagerDx12::release() {
  ScopedLock s(&m_commandListLock_);

  for (auto& iter : m_usingCommandBuffers_) {
    iter->getFence()->waitForFence();
    g_rhi->getFenceManager()->returnFence(iter->getFence());
    delete iter;
  }
  m_usingCommandBuffers_.clear();

  for (auto& iter : m_availableCommandLists_) {
    iter->getFence()->waitForFence();
    g_rhi->getFenceManager()->returnFence(iter->getFence());
    delete iter;
  }
  m_availableCommandLists_.clear();
}

CommandBufferDx12* CommandBufferManagerDx12::getOrCreateCommandBuffer() {
  ScopedLock s(&m_commandListLock_);

  CommandBufferDx12* SelectedCmdBuffer = nullptr;
  for (int32_t i = 0; i < m_availableCommandLists_.size(); ++i) {
    if (m_availableCommandLists_[i]->isCompleteForWaitFence()) {
      SelectedCmdBuffer = m_availableCommandLists_[i];
      m_availableCommandLists_.erase(m_availableCommandLists_.begin() + i);
      break;
    }
  }

  if (!SelectedCmdBuffer) {
    SelectedCmdBuffer = createCommandList_();
    assert(SelectedCmdBuffer);
  }

  m_usingCommandBuffers_.push_back(SelectedCmdBuffer);
  SelectedCmdBuffer->begin();
  return SelectedCmdBuffer;
}

void CommandBufferManagerDx12::returnCommandBuffer(
    CommandBuffer* commandBuffer) {
  ScopedLock s(&m_commandListLock_);

  for (int32_t i = 0; i < m_usingCommandBuffers_.size(); ++i) {
    if (m_usingCommandBuffers_[i]->getNativeHandle()
        == commandBuffer->getNativeHandle()) {
      m_usingCommandBuffers_.erase(m_usingCommandBuffers_.begin() + i);
      m_availableCommandLists_.push_back((CommandBufferDx12*)commandBuffer);
      return;
    }
  }
}

bool CommandBufferManagerDx12::initialize(ComPtr<ID3D12Device>    device,
                                          D3D12_COMMAND_LIST_TYPE type) {
  assert(device);
  m_device_ = device;

  m_commandListType_ = type;

  if (m_commandListType_ != D3D12_COMMAND_LIST_TYPE_BUNDLE) {
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
    commandQueueDesc.Type                     = m_commandListType_;
    commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    commandQueueDesc.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    commandQueueDesc.NodeMask = 0;

    if (FAILED(m_device_->CreateCommandQueue(&commandQueueDesc,
                                          IID_PPV_ARGS(&m_commandQueue_)))) {
      return false;
    }
  }

  m_fence_ = (FenceDx12*)g_rhiDx12->m_fenceManager_.getOrCreateFence();

  return true;
}

void CommandBufferManagerDx12::executeCommandList(
    CommandBufferDx12* commandList, bool waitUntilExecuteComplete) {
  if (!commandList->end()) {
    return;
  }

  ID3D12CommandList* pCommandLists[] = {commandList->get()};
  m_commandQueue_->ExecuteCommandLists(1, pCommandLists);

  auto* fence = commandList->m_owner_->m_fence_;
  assert(fence);

  if (fence) {
    commandList->m_fenceValue_ = fence->signalWithNextFenceValue(
        commandList->m_owner_->m_commandQueue_.Get(), waitUntilExecuteComplete);
  }
}

CommandBufferDx12* CommandBufferManagerDx12::createCommandList_() const {
  CommandBufferDx12* commandBuffer = new CommandBufferDx12();
  commandBuffer->m_owner_             = this;
  commandBuffer->m_commandAllocator_  = createCommandAllocator_();
  if (FAILED(m_device_->CreateCommandList(
          0,
          m_commandListType_,
          commandBuffer->m_commandAllocator_.Get(),
          nullptr,
          IID_PPV_ARGS(&commandBuffer->m_commandList_)))) {
    delete commandBuffer;
    return nullptr;
  }

  if (D3D12_COMMAND_LIST_TYPE_COPY != m_commandListType_) {
    commandBuffer->m_onlineDescriptorHeap_
        = g_rhiDx12->m_onlineDescriptorHeapManager_.alloc(
            EDescriptorHeapTypeDX12::CBV_SRV_UAV);
    commandBuffer->m_onlineSamplerDescriptorHeap_
        = g_rhiDx12->m_onlineDescriptorHeapManager_.alloc(
            EDescriptorHeapTypeDX12::SAMPLER);

    assert(commandBuffer->m_onlineDescriptorHeap_);
    assert(commandBuffer->m_onlineSamplerDescriptorHeap_);
  }

  return commandBuffer;
}

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12