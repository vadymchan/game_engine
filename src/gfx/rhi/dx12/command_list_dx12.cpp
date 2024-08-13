#include "gfx/rhi/dx12/command_list_dx12.h"

#include "gfx/rhi/dx12/command_allocator_dx12.h"
#include "gfx/rhi/dx12/descriptor_heap_dx12.h"
#include "utils/logger/global_logger.h"

#include <cassert>

namespace game_engine {

bool CommandBufferDx12::Begin() const {
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

void CommandBufferDx12::Reset() const {
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

void* CommandBufferDx12::GetFenceHandle() const {
  return Owner->m_fence ? Owner->m_fence->GetHandle() : nullptr;
}

void CommandBufferDx12::SetFence(void* InFence) {
  // m_fence = (FenceDx12*)InFence;
  assert(0);
}

Fence* CommandBufferDx12::GetFence() const {
  return Owner->m_fence;
}

bool CommandBufferDx12::IsCompleteForWaitFence() {
  return Owner->m_fence->IsComplete(FenceValue);
}

bool CommandBufferDx12::End() const {
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