#include "gfx/rhi/dx12/command_allocator_dx12.h"

#include "gfx/rhi/dx12/rhi_dx12.h"
#include "gfx/rhi/rhi.h"

#include <cassert>

namespace game_engine {

void jCommandBufferManager_DX12::Release() {
  ScopedLock s(&CommandListLock);

  for (auto& iter : UsingCommandBuffers) {
    iter->GetFence()->WaitForFence();
    g_rhi->GetFenceManager()->ReturnFence(iter->GetFence());
    delete iter;
  }
  UsingCommandBuffers.clear();

  for (auto& iter : AvailableCommandLists) {
    iter->GetFence()->WaitForFence();
    g_rhi->GetFenceManager()->ReturnFence(iter->GetFence());
    delete iter;
  }
  AvailableCommandLists.clear();
}

jCommandBuffer_DX12* jCommandBufferManager_DX12::GetOrCreateCommandBuffer() {
  ScopedLock s(&CommandListLock);

  jCommandBuffer_DX12* SelectedCmdBuffer = nullptr;
  for (int32_t i = 0; i < AvailableCommandLists.size(); ++i) {
    if (AvailableCommandLists[i]->IsCompleteForWaitFence()) {
      SelectedCmdBuffer = AvailableCommandLists[i];
      AvailableCommandLists.erase(AvailableCommandLists.begin() + i);
      break;
    }
  }

  if (!SelectedCmdBuffer) {
    SelectedCmdBuffer = CreateCommandList();
    assert(SelectedCmdBuffer);
  }

  UsingCommandBuffers.push_back(SelectedCmdBuffer);
  SelectedCmdBuffer->Begin();
  return SelectedCmdBuffer;
}

void jCommandBufferManager_DX12::ReturnCommandBuffer(
    jCommandBuffer* commandBuffer) {
  ScopedLock s(&CommandListLock);

  for (int32_t i = 0; i < UsingCommandBuffers.size(); ++i) {
    if (UsingCommandBuffers[i]->GetNativeHandle()
        == commandBuffer->GetNativeHandle()) {
      UsingCommandBuffers.erase(UsingCommandBuffers.begin() + i);
      AvailableCommandLists.push_back((jCommandBuffer_DX12*)commandBuffer);
      return;
    }
  }
}

bool jCommandBufferManager_DX12::Initialize(ComPtr<ID3D12Device>    InDevice,
                                            D3D12_COMMAND_LIST_TYPE InType) {
  assert(InDevice);
  Device = InDevice;

  CommandListType = InType;

  if (CommandListType != D3D12_COMMAND_LIST_TYPE_BUNDLE) {
    D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
    commandQueueDesc.Type                     = CommandListType;
    commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    commandQueueDesc.Flags    = D3D12_COMMAND_QUEUE_FLAG_NONE;
    commandQueueDesc.NodeMask = 0;

    if (FAILED(Device->CreateCommandQueue(&commandQueueDesc,
                                          IID_PPV_ARGS(&CommandQueue)))) {
      return false;
    }
  }

  Fence = (jFence_DX12*)g_rhi_dx12->FenceManager.GetOrCreateFence();

  return true;
}

void jCommandBufferManager_DX12::ExecuteCommandList(
    jCommandBuffer_DX12* InCommandList, bool bWaitUntilExecuteComplete) {
  if (!InCommandList->End()) {
    return;
  }

  ID3D12CommandList* pCommandLists[] = {InCommandList->Get()};
  CommandQueue->ExecuteCommandLists(1, pCommandLists);

  auto* fence = InCommandList->Owner->Fence;
  assert(fence);

  if (fence) {
    InCommandList->FenceValue = fence->SignalWithNextFenceValue(
        InCommandList->Owner->CommandQueue.Get(), bWaitUntilExecuteComplete);
  }
}

jCommandBuffer_DX12* jCommandBufferManager_DX12::CreateCommandList() const {
  jCommandBuffer_DX12* commandBuffer = new jCommandBuffer_DX12();
  commandBuffer->Owner               = this;
  commandBuffer->CommandAllocator    = CreateCommandAllocator();
  if (FAILED(Device->CreateCommandList(
          0,
          CommandListType,
          commandBuffer->CommandAllocator.Get(),
          nullptr,
          IID_PPV_ARGS(&commandBuffer->CommandList)))) {
    delete commandBuffer;
    return nullptr;
  }

  if (D3D12_COMMAND_LIST_TYPE_COPY != CommandListType) {
    commandBuffer->OnlineDescriptorHeap
        = g_rhi_dx12->OnlineDescriptorHeapManager.Alloc(
            EDescriptorHeapTypeDX12::CBV_SRV_UAV);
    commandBuffer->OnlineSamplerDescriptorHeap
        = g_rhi_dx12->OnlineDescriptorHeapManager.Alloc(
            EDescriptorHeapTypeDX12::SAMPLER);

    assert(commandBuffer->OnlineDescriptorHeap);
    assert(commandBuffer->OnlineSamplerDescriptorHeap);
  }

  return commandBuffer;
}

}  // namespace game_engine