#ifndef GAME_ENGINE_RENDER_PASS_DX12_H
#define GAME_ENGINE_RENDER_PASS_DX12_H

#include "platform/windows/windows_platform_setup.h"

#ifdef GAME_ENGINE_RHI_DX12

#include "gfx/rhi/dx12/command_list_dx12.h"
#include "gfx/rhi/render_pass.h"

namespace game_engine {

class RenderPassDx12 : public RenderPass {
  public:
  // ======= BEGIN: public aliases ============================================

  using RenderPass::RenderPass;

  // ======= END: public aliases   ============================================

  // ======= BEGIN: public destructor =========================================

  virtual ~RenderPassDx12() { release(); }

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overridden methods =================================

  virtual bool beginRenderPass(
      const std::shared_ptr<CommandBuffer>& commandBuffer) override;
  virtual void endRenderPass() override;

  // ======= END: public overridden methods   =================================

  // ======= BEGIN: public getters ============================================

  const std::vector<DXGI_FORMAT>& getRTVFormats() const {
    return m_rtvFormats_;
  }

  DXGI_FORMAT getDSVFormat() const { return m_dsvFormat_; }

  // ======= END: public getters   ============================================

  // ======= BEGIN: public misc methods =======================================

  void initialize();
  bool createRenderPass();
  void release();

  // Remove this. After organizing the relationship between CommandBufferDx12
  // and CommandBuffer.
  bool beginRenderPass(const std::shared_ptr<CommandBufferDx12>& commandBuffer,
                       D3D12_CPU_DESCRIPTOR_HANDLE               tempRTV) {
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

  // ======= END: public misc methods   =======================================

  private:
  // ======= BEGIN: private misc methods ======================================

  void setFinalLayoutToAttachment_(const Attachment& attachment) const;

  // ======= END: private misc methods   ======================================

  // ======= BEGIN: private misc fields =======================================

  std::shared_ptr<CommandBufferDx12> m_commandBuffer_ = nullptr;

  // TODO: consider if this is the appropriate naming convention
  std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> m_rtvCPUHandles_;
  D3D12_CPU_DESCRIPTOR_HANDLE              m_dsvCPUDHandle_ = {};

  std::vector<RtClearValue> m_rtvClears_;
  RtClearValue              m_dsvClear_        = RtClearValue(1.0f, 0);
  bool                      m_dsvDepthClear_   = false;
  bool                      m_dsvStencilClear_ = false;

  std::vector<DXGI_FORMAT> m_rtvFormats_;
  DXGI_FORMAT              m_dsvFormat_ = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;

  // ======= END: private misc fields   =======================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12

#endif  // GAME_ENGINE_RENDER_PASS_DX12_H
