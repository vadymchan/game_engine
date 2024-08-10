#ifndef GAME_ENGINE_RENDER_PASS_DX12_H
#define GAME_ENGINE_RENDER_PASS_DX12_H

#include "gfx/rhi/render_pass.h"
#include "gfx/rhi/dx12/command_list_dx12.h"
#include "platform/windows/windows_platform_setup.h"

namespace game_engine {

class jRenderPass_DX12 : public jRenderPass {
  public:
  using jRenderPass::jRenderPass;

  virtual ~jRenderPass_DX12() { Release(); }

  void Initialize();
  bool CreateRenderPass();
  void Release();

  // virtual void* GetRenderPass() const override { return RenderPass; }
  // inline const VkRenderPass& GetRenderPassRaw() const { return
  // RenderPass; } virtual void* GetFrameBuffer() const override { return
  // FrameBuffer; }

  virtual bool BeginRenderPass(const jCommandBuffer* commandBuffer) override;

  // Remove this. After organizing the relationship between jCommandBuffer_DX12
  // and jCommandBuffer.
  bool BeginRenderPass(const jCommandBuffer_DX12*  commandBuffer,
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

    commandBuffer->CommandList->OMSetRenderTargets(
        (uint32_t)RTVCPUHandles.size(),
        &RTVCPUHandles[0],
        false,
        &DSVCPUDHandle);

    for (int32_t i = 0; i < RTVClears.size(); ++i) {
      if (RTVClears[i].GetType() != ERTClearType::Color) {
        continue;
      }

      commandBuffer->CommandList->ClearRenderTargetView(
          RTVCPUHandles[i], RTVClears[i].GetCleraColor(), 0, nullptr);
    }

    if (DSVClear.GetType() == ERTClearType::DepthStencil) {
      if (DSVDepthClear || DSVStencilClear) {
        D3D12_CLEAR_FLAGS DSVClearFlags = (D3D12_CLEAR_FLAGS)0;
        if (DSVDepthClear) {
          DSVClearFlags |= D3D12_CLEAR_FLAG_DEPTH;
        }

        if (DSVStencilClear) {
          DSVClearFlags |= D3D12_CLEAR_FLAG_STENCIL;
        }

        commandBuffer->CommandList->ClearDepthStencilView(
            DSVCPUDHandle,
            DSVClearFlags,
            DSVClear.GetCleraDepth(),
            (uint8_t)DSVClear.GetCleraStencil(),
            0,
            nullptr);
      }
    }

    return true;
  }

  virtual void EndRenderPass() override;

  const std::vector<DXGI_FORMAT>& GetRTVFormats() const { return RTVFormats; }

  DXGI_FORMAT GetDSVFormat() const { return DSVFormat; }

  private:
  void SetFinalLayoutToAttachment(const jAttachment& attachment) const;

  private:
  const jCommandBuffer_DX12* CommandBuffer = nullptr;

  std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> RTVCPUHandles;
  D3D12_CPU_DESCRIPTOR_HANDLE              DSVCPUDHandle = {};

  std::vector<jRTClearValue> RTVClears;
  jRTClearValue              DSVClear        = jRTClearValue(1.0f, 0);
  bool                       DSVDepthClear   = false;
  bool                       DSVStencilClear = false;

  std::vector<DXGI_FORMAT> RTVFormats;
  DXGI_FORMAT              DSVFormat = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_PASS_DX12_H