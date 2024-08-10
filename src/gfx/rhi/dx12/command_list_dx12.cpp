#include "gfx/rhi/dx12/command_list_dx12.h"

#include "gfx/rhi/dx12/command_allocator_dx12.h"
#include "gfx/rhi/dx12/descriptor_heap_dx12.h"
#include "utils/logger/global_logger.h"

#include <cassert>

namespace game_engine {

bool jCommandBuffer_DX12::Begin() const {
  Reset();

  assert(OnlineDescriptorHeap && OnlineSamplerDescriptorHeap
         || (!OnlineDescriptorHeap && !OnlineSamplerDescriptorHeap));
  if (OnlineDescriptorHeap && OnlineSamplerDescriptorHeap) {
    ID3D12DescriptorHeap* ppHeaps[] = {OnlineDescriptorHeap->GetHeap(),
                                       OnlineSamplerDescriptorHeap->GetHeap()};
    CommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
  }

  return true;
}

void jCommandBuffer_DX12::Reset() const {
  if (IsClosed) {
    HRESULT hr;

    hr = CommandAllocator->Reset();
    if (FAILED(hr)) {
      GlobalLogger::Log(
          LogLevel::Error,
          "Failed to reset command allocator: " + std::to_string(hr));
      return;
    }

    hr = CommandList->Reset(CommandAllocator.Get(), nullptr);
    if (FAILED(hr)) {
      GlobalLogger::Log(LogLevel::Error,
                        "Failed to reset command list: " + std::to_string(hr));
      return;
    }

    if (OnlineDescriptorHeap) {
      OnlineDescriptorHeap->Reset();
    }
    if (OnlineSamplerDescriptorHeap) {
      OnlineSamplerDescriptorHeap->Reset();
    }
    IsClosed = false;

    GlobalLogger::Log(LogLevel::Info, "Command buffer reset successfully.");
  }
}

void* jCommandBuffer_DX12::GetFenceHandle() const {
  return Owner->Fence ? Owner->Fence->GetHandle() : nullptr;
}

void jCommandBuffer_DX12::SetFence(void* InFence) {
  // Fence = (jFence_DX12*)InFence;
  assert(0);
}

jFence* jCommandBuffer_DX12::GetFence() const {
  return Owner->Fence;
}

bool jCommandBuffer_DX12::IsCompleteForWaitFence() {
  return Owner->Fence->IsComplete(FenceValue);
}

bool jCommandBuffer_DX12::End() const {
  if (IsClosed) {
    return true;
  }

  IsClosed   = true;
  HRESULT hr = CommandList->Close();
  assert(SUCCEEDED(hr));

  if (FAILED(hr)) {
    return false;
  }

  return true;
}

}  // namespace game_engine