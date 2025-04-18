#ifndef GAME_ENGINE_DEVICE_DX12_H
#define GAME_ENGINE_DEVICE_DX12_H

#include "gfx/rhi/rhi_new/interface/device.h"
#include "gfx/rhi/rhi_new/backends/dx12/descriptor_dx12.h"
#include "gfx/rhi/rhi_new/backends/dx12/command_buffer_dx12.h"
#include "platform/windows/windows_platform_setup.h"

#ifdef GAME_ENGINE_RHI_DX12

namespace game_engine {
namespace gfx {
namespace rhi {

class DeviceDx12 : public Device {
  public:
  DeviceDx12(const DeviceDesc& desc);
  ~DeviceDx12() override;

  RenderingApi getApiType() const override { return RenderingApi::Dx12; }

  std::unique_ptr<Buffer>              createBuffer(const BufferDesc& desc) override;
  std::unique_ptr<Texture>             createTexture(const TextureDesc& desc) override;
  std::unique_ptr<Sampler>             createSampler(const SamplerDesc& desc) override;
  std::unique_ptr<Shader>              createShader(const ShaderDesc& desc) override;
  std::unique_ptr<GraphicsPipeline>    createGraphicsPipeline(const GraphicsPipelineDesc& desc) override;
  std::unique_ptr<DescriptorSetLayout> createDescriptorSetLayout(const DescriptorSetLayoutDesc& desc) override;
  std::unique_ptr<DescriptorSet>       createDescriptorSet(const DescriptorSetLayout* layout) override;
  std::unique_ptr<RenderPass>          createRenderPass(const RenderPassDesc& desc) override;
  std::unique_ptr<Framebuffer>         createFramebuffer(const FramebufferDesc& desc) override;
  std::unique_ptr<CommandBuffer> createCommandBuffer(const CommandBufferDesc& desc = CommandBufferDesc()) override;
  std::unique_ptr<Fence>         createFence(const FenceDesc& desc = FenceDesc()) override;
  std::unique_ptr<Semaphore>     createSemaphore() override;
  std::unique_ptr<SwapChain>     createSwapChain(const SwapchainDesc& desc) override;

  void updateBuffer(Buffer* buffer, const void* data, size_t size, size_t offset = 0) override;
  void updateTexture(
      Texture* texture, const void* data, size_t dataSize, uint32_t mipLevel = 0, uint32_t arrayLayer = 0) override;

  void submitCommandBuffer(CommandBuffer*                 cmdBuffer,
                           Fence*                         signalFence      = nullptr,
                           const std::vector<Semaphore*>& waitSemaphores   = {},
                           const std::vector<Semaphore*>& signalSemaphores = {}) override;

  void present(SwapChain* swapChain, Semaphore* waitSemaphore = nullptr) override;

  void waitIdle() override;

  IDXGIFactory6* getFactory() const { return m_factory_.Get(); }

  ID3D12Device* getDevice() const { return m_device_.Get(); }

  ID3D12CommandQueue* getCommandQueue() const { return m_commandQueue_.Get(); }

  DescriptorHeap* getRtvHeap() { return &m_rtvHeap_; }

  DescriptorHeap* getDsvHeap() { return &m_dsvHeap_; }

  DescriptorHeap* getCbvSrvUavHeap() { return &m_cbvSrvUavHeap_; }

  DescriptorHeap* getSamplerHeap() { return &m_samplerHeap_; }

  private:
  bool createFactory_();
  bool createDevice_();
  bool createCommandQueue_();
  bool createCommandPools_();
  bool createDescriptorHeaps_();

  bool findAdapter_(IDXGIAdapter1** adapter);

  ComPtr<IDXGIFactory6>      m_factory_;
  ComPtr<IDXGIAdapter3>      m_adapter_;
  ComPtr<ID3D12Device>       m_device_;
  ComPtr<ID3D12CommandQueue> m_commandQueue_;

  // Descriptor heaps
  DescriptorHeap m_rtvHeap_;        // Render target views
  DescriptorHeap m_dsvHeap_;        // Depth stencil views
  DescriptorHeap m_cbvSrvUavHeap_;  // Constant/shader/unordered access views
  DescriptorHeap m_samplerHeap_;    // Samplers

  CommandAllocatorManager m_commandAllocatorManager_;

  // Frame synchronization
  uint32_t m_frameIndex_ = 0;
  uint32_t m_frameCount_ = 0;
};

}  // namespace rhi
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RHI_DX12

#endif  // GAME_ENGINE_DEVICE_DX12_H