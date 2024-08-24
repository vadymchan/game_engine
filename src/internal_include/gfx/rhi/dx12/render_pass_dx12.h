#ifndef GAME_ENGINE_RENDER_PASS_DX12_H
#define GAME_ENGINE_RENDER_PASS_DX12_H

#include "gfx/rhi/dx12/command_list_dx12.h"
#include "gfx/rhi/render_pass.h"
#include "platform/windows/windows_platform_setup.h"

namespace game_engine {

class RenderPassDx12 : public RenderPass {
  public:
  using RenderPass::RenderPass;

  virtual ~RenderPassDx12() { release(); }

  void initialize();
  bool createRenderPass();
  void release();

  // virtual void* getRenderPass() const override { return kRenderPass; }
  // inline const VkRenderPass& GetRenderPassRaw() const { return
  // kRenderPass; } virtual void* getFrameBuffer() const override { return
  // m_frameBuffer; }

  virtual bool beginRenderPass(const CommandBuffer* commandBuffer) override;

  // Remove this. After organizing the relationship between CommandBufferDx12
  // and CommandBuffer.
  bool beginRenderPass(const CommandBufferDx12*    commandBuffer,
                       D3D12_CPU_DESCRIPTOR_HANDLE tempRTV) {
    assert(commandBuffer);
    if (!commandBuffer) {
      return false;
    }

    // If updating the RenderPass every frame, remove this code and use the code
    // below.
    // commandBuffer->CommandList->OMSetRenderTargets(
    //    1, &tempRTV, false, nullptr);
    // const float clearColor[] = {0.0f, 0.2f, 0.4f, 1.0f};
    // commandBuffer->CommandList->ClearRenderTargetView(
    //    tempRTV, clearColor, 0, nullptr);
    // return true;

    commandBuffer->m_commandList_->OMSetRenderTargets(
        (uint32_t)m_rtvCPUHandles_.size(),
        &m_rtvCPUHandles_[0],
        false,
        &m_dsvCPUDHandle_);

    for (int32_t i = 0; i < m_rtvClears_.size(); ++i) {
      if (m_rtvClears_[i].getType() != ERTClearType::Color) {
        continue;
      }

      commandBuffer->m_commandList_->ClearRenderTargetView(
          m_rtvCPUHandles_[i], m_rtvClears_[i].getCleraColor(), 0, nullptr);
    }

    if (m_dsvClear_.getType() == ERTClearType::DepthStencil) {
      if (m_dsvDepthClear_ || m_dsvStencilClear_) {
        D3D12_CLEAR_FLAGS dsvClearFlags = (D3D12_CLEAR_FLAGS)0;
        if (m_dsvDepthClear_) {
          dsvClearFlags |= D3D12_CLEAR_FLAG_DEPTH;
        }

        if (m_dsvStencilClear_) {
          dsvClearFlags |= D3D12_CLEAR_FLAG_STENCIL;
        }

        commandBuffer->m_commandList_->ClearDepthStencilView(
            m_dsvCPUDHandle_,
            dsvClearFlags,
            m_dsvClear_.getClearDepth(),
            (uint8_t)m_dsvClear_.getClearStencil(),
            0,
            nullptr);
      }
    }

    return true;
  }

  virtual void endRenderPass() override;

  const std::vector<DXGI_FORMAT>& getRTVFormats() const { return m_rtvFormats_; }

  DXGI_FORMAT getDSVFormat() const { return m_dsvFormat_; }

  private:
  void setFinalLayoutToAttachment_(const Attachment& attachment) const;

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