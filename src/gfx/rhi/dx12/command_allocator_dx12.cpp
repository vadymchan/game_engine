#include "gfx/rhi/dx12/command_allocator_dx12.h"

#include "gfx/rhi/dx12/rhi_dx12.h"
#include "gfx/rhi/rhi.h"

#include <cassert>

namespace game_engine {

void CommandBufferManagerDx12::Release() {
  ScopedLock s(&m_commandListLock_);

  for (auto& iter : m_usingCommandBuffers_) {
    iter->GetFence()->WaitForFence();
    g_rhi->GetFenceManager()->ReturnFence(iter->GetFence());
    delete iter;
  }
  m_usingCommandBuffers_.clear();

  for (auto& iter : m_availableCommandLists_) {
    iter->GetFence()->WaitForFence();
    g_rhi->GetFenceManager()->ReturnFence(iter->GetFence());
    delete iter;
  }
  m_availableCommandLists_.clear();
}

CommandBufferDx12* CommandBufferManagerDx12::GetOrCreateCommandBuffer() {
  ScopedLock s(&m_commandListLock_);

  CommandBufferDx12* SelectedCmdBuffer = nullptr;
  for (int32_t i = 0; i < m_availableCommandLists_.size(); ++i) {
    if (m_availableCommandLists_[i]->IsCompleteForWaitFence()) {
      SelectedCmdBuffer = m_availableCommandLists_[i];
      m_availableCommandLists_.erase(m_availableCommandLists_.begin() + i);
      break;
    }
  }

  if (!SelectedCmdBuffer) {
    SelectedCmdBuffer = CreateCommandList();
    assert(SelectedCmdBuffer);
  }

  m_usingCommandBuffers_.push_back(SelectedCmdBuffer);
  SelectedCmdBuffer->Begin();
  return SelectedCmdBuffer;
}

void CommandBufferManagerDx12::ReturnCommandBuffer(
    CommandBuffer* commandBuffer) {
  ScopedLock s(&m_commandListLock_);

  for (int32_t i = 0; i < m_usingCommandBuffers_.size(); ++i) {
    if (m_usingCommandBuffers_[i]->GetNativeHandle()
        == commandBuffer->GetNativeHandle()) {
      m_usingCommandBuffers_.erase(m_usingCommandBuffers_.begin() + i);
      m_availableCommandLists_.push_back((CommandBufferDx12*)commandBuffer);
      return;
    }
  }
}

bool CommandBufferManagerDx12::Initialize(ComPtr<ID3D12Device>    device,
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

  m_fence = (FenceDx12*)g_rhi_dx12->m_fenceManager_.GetOrCreateFence();

  return true;
}

void CommandBufferManagerDx12::ExecuteCommandList(
    CommandBufferDx12* commandList, bool waitUntilExecuteComplete) {
  if (!commandList->End()) {
    return;
  }

  ID3D12CommandList* pCommandLists[] = {commandList->Get()};
  m_commandQueue_->ExecuteCommandLists(1, pCommandLists);

  auto* fence = commandList->m_owner_->m_fence;
  assert(fence);

  if (fence) {
    commandList->m_fenceValue_ = fence->SignalWithNextFenceValue(
        commandList->m_owner_->m_commandQueue_.Get(), waitUntilExecuteComplete);
  }
}

CommandBufferDx12* CommandBufferManagerDx12::CreateCommandList() const {
  CommandBufferDx12* commandBuffer = new CommandBufferDx12();
  commandBuffer->m_owner_             = this;
  commandBuffer->m_commandAllocator_  = CreateCommandAllocator();
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
        = g_rhi_dx12->m_onlineDescriptorHeapManager_.Alloc(
            EDescriptorHeapTypeDX12::CBV_SRV_UAV);
    commandBuffer->m_onlineSamplerDescriptorHeap_
        = g_rhi_dx12->m_onlineDescriptorHeapManager_.Alloc(
            EDescriptorHeapTypeDX12::SAMPLER);

    assert(commandBuffer->m_onlineDescriptorHeap_);
    assert(commandBuffer->m_onlineSamplerDescriptorHeap_);
  }

  return commandBuffer;
}

}  // namespace game_engine