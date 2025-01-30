#include "gfx/rhi/dx12/command_list_dx12.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/dx12/command_allocator_dx12.h"
#include "gfx/rhi/dx12/descriptor_heap_dx12.h"
#include "utils/logger/global_logger.h"

#include <cassert>

namespace game_engine {

bool CommandBufferDx12::begin() const {
  reset();

  assert(m_onlineDescriptorHeap_ && m_onlineSamplerDescriptorHeap_
         || (!m_onlineDescriptorHeap_ && !m_onlineSamplerDescriptorHeap_));
  if (m_onlineDescriptorHeap_ && m_onlineSamplerDescriptorHeap_) {
    ID3D12DescriptorHeap* ppHeaps[]
        = {m_onlineDescriptorHeap_->getHeap(),
           m_onlineSamplerDescriptorHeap_->getHeap()};
    m_commandList_->SetDescriptorHeaps(std::size(ppHeaps), ppHeaps);
  }

  return true;
}

void CommandBufferDx12::reset() const {
  if (m_isClosed_) {
    HRESULT hr;

    hr = m_commandAllocator_->Reset();
    if (FAILED(hr)) {
      GlobalLogger::Log(
          LogLevel::Error,
          "Failed to reset command allocator: " + std::to_string(hr));
      return;
    }

    hr = m_commandList_->Reset(m_commandAllocator_.Get(), nullptr);
    if (FAILED(hr)) {
      GlobalLogger::Log(LogLevel::Error,
                        "Failed to reset command list: " + std::to_string(hr));
      return;
    }

    if (m_onlineDescriptorHeap_) {
      m_onlineDescriptorHeap_->reset();
    }
    if (m_onlineSamplerDescriptorHeap_) {
      m_onlineSamplerDescriptorHeap_->reset();
    }
    m_isClosed_ = false;

    // GlobalLogger::Log(LogLevel::Info, "Command buffer reset successfully.");
  }
}

void* CommandBufferDx12::getFenceHandle() const {
  return m_owner_->m_fence_ ? m_owner_->m_fence_->getHandle() : nullptr;
}

void CommandBufferDx12::setFence(void* fence) {
  // m_fence_ = (FenceDx12*)fence;
  assert(0);
}

IFence* CommandBufferDx12::getFence() const {
  return m_owner_->m_fence_;
}

bool CommandBufferDx12::isCompleteForWaitFence() {
  return m_owner_->m_fence_->isComplete(m_fenceValue_);
}

bool CommandBufferDx12::end() const {
  if (m_isClosed_) {
    return true;
  }

  m_isClosed_ = true;
  HRESULT hr  = m_commandList_->Close();
  assert(SUCCEEDED(hr));

  if (FAILED(hr)) {
    return false;
  }

  return true;
}

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12
