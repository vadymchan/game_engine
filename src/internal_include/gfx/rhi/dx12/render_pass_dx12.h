#ifndef GAME_ENGINE_RENDER_PASS_DX12_H
#define GAME_ENGINE_RENDER_PASS_DX12_H

#include "gfx/rhi/dx12/command_list_dx12.h"
#include "gfx/rhi/render_pass.h"
#include "platform/windows/windows_platform_setup.h"

namespace game_engine {

class RenderPassDx12 : public RenderPass {
  public:
  using RenderPass::RenderPass;

  virtual ~RenderPassDx12() { Release(); }

  void Initialize();
  bool CreateRenderPass();
  void Release();

  // virtual void* GetRenderPass() const override { return m_renderPass; }
  // inline const VkRenderPass& GetRenderPassRaw() const { return
  // m_renderPass; } virtual void* GetFrameBuffer() const override { return
  // m_frameBuffer; }

  virtual bool BeginRenderPass(const CommandBuffer* commandBuffer) override;

  // Remove this. After organizing the relationship between CommandBufferDx12
  // and CommandBuffer.
  bool BeginRenderPass(const CommandBufferDx12*    commandBuffer,
                       D3D12_CPU_DESCRIPTOR_HANDLE InTempRTV) {
    assert(commandBuffer);
    if (!commandBuffer) {
      return false;
    }

    // If updating the RenderPass every frame, remove this code and use the code
    // below.
    // commandBuffer->CommandList->OMSetRenderTargets(
    //    1, &InTempRTV, false, nullptr);
    // const float clearColor[] = {0.0f, 0.2f, 0.4f, 1.0f};
    // commandBuffer->CommandList->ClearRenderTargetView(
    //    InTempRTV, clearColor, 0, nullptr);
    // return true;

    commandBuffer->m_commandList_->OMSetRenderTargets(
        (uint32_t)m_rtvCPUHandles_.size(),
        &m_rtvCPUHandles_[0],
        false,
        &m_dsvCPUDHandle_);

    for (int32_t i = 0; i < m_rtvClears_.size(); ++i) {
      if (m_rtvClears_[i].GetType() != ERTClearType::Color) {
        continue;
      }

      commandBuffer->m_commandList_->ClearRenderTargetView(
          m_rtvCPUHandles_[i], m_rtvClears_[i].GetCleraColor(), 0, nullptr);
    }

    if (m_dsvClear_.GetType() == ERTClearType::DepthStencil) {
      if (m_dsvDepthClear_ || m_dsvStencilClear_) {
        D3D12_CLEAR_FLAGS DSVClearFlags = (D3D12_CLEAR_FLAGS)0;
        if (m_dsvDepthClear_) {
          DSVClearFlags |= D3D12_CLEAR_FLAG_DEPTH;
        }

        if (m_dsvStencilClear_) {
          DSVClearFlags |= D3D12_CLEAR_FLAG_STENCIL;
        }

        commandBuffer->m_commandList_->ClearDepthStencilView(
            m_dsvCPUDHandle_,
            DSVClearFlags,
            m_dsvClear_.GetCleraDepth(),
            (uint8_t)m_dsvClear_.GetCleraStencil(),
            0,
            nullptr);
      }
    }

    return true;
  }

  virtual void EndRenderPass() override;

  const std::vector<DXGI_FORMAT>& GetRTVFormats() const { return m_rtvFormats_; }

  DXGI_FORMAT GetDSVFormat() const { return m_dsvFormat_; }

  private:
  void SetFinalLayoutToAttachment(const Attachment& attachment) const;

  private:
  const CommandBufferDx12* m_commandBuffer_ = nullptr;

  // TODO: consider if this is the appropriate naming convention
  std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_rtvCPUHandles_;
  D3D12_CPU_DESCRIPTOR_HANDLE              m_dsvCPUDHandle_ = {};

  std::vector<RTClearValue> m_rtvClears_;
  RTClearValue              m_dsvClear_     = RTClearValue(1.0f, 0);
  bool                      m_dsvDepthClear_   = false;
  bool                      m_dsvStencilClear_ = false;

  std::vector<DXGI_FORMAT> m_rtvFormats_;
  DXGI_FORMAT              m_dsvFormat_ = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_PASS_DX12_H