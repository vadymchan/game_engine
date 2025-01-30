#ifndef GAME_ENGINE_EDITOR_H
#define GAME_ENGINE_EDITOR_H

#include "config/config_manager.h"
#include "gfx/rhi/dx12/rhi_dx12.h"
#include "gfx/rhi/render_frame_context.h"
#include "gfx/rhi/vulkan/rhi_vk.h"
#include "utils/logger/global_logger.h"
#include "utils/logger/memory_logger.h"
#include "utils/path_manager/path_manager.h"
#include "utils/service/service_locator.h"
#include "utils/time/timing_manager.h"

#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_vulkan.h>

namespace game_engine {

class Editor {
  public:
  auto getRenderParams() -> const EditorRenderParams& { return renderParams; }

  void init(const std::shared_ptr<Window>& window) {
    auto renderingApi = RuntimeSettings::s_get().getRenderingApi();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Enable Docking and Multi-Viewport
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      ImGuiStyle& style                 = ImGui::GetStyle();
      style.WindowRounding              = 0.0f;
      style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    if (renderingApi == RenderingApi::Vulkan) {
      if (!ImGui_ImplSDL2_InitForVulkan(window->getNativeWindowHandle())) {
        GlobalLogger::Log(LogLevel::Error,
                          "Failed to initialize ImGui SDL2 backend.");
      }
      auto renderPass = createRenderPass_(nullptr);

      ImGui_ImplVulkan_InitInfo initInfo = {};
      initInfo.Instance                  = g_rhiVk->m_instance_;
      initInfo.PhysicalDevice            = g_rhiVk->m_physicalDevice_;
      initInfo.Device                    = g_rhiVk->m_device_;
      // QueueFamily == m_queueIndex_ ?
      initInfo.QueueFamily   = g_rhiVk->m_graphicsQueue_.m_queueIndex_;
      initInfo.Queue         = g_rhiVk->m_graphicsQueue_.m_queue_;
      initInfo.PipelineCache = VK_NULL_HANDLE;
      // TODO: ensure that it's correctly handled
      initInfo.DescriptorPool
          = g_rhiVk->getDescriptorPoolMultiFrame()->m_descriptorPool_;
      initInfo.Allocator     = nullptr;  // default allocator
      initInfo.MinImageCount = g_rhiVk->m_swapchain_->getNumOfSwapchainImages();
      initInfo.ImageCount    = g_rhiVk->m_swapchain_->getNumOfSwapchainImages();
      initInfo.CheckVkResultFn = nullptr;
      initInfo.RenderPass      = (VkRenderPass)renderPass->getRenderPass();

      if (!ImGui_ImplVulkan_Init(&initInfo)) {
        GlobalLogger::Log(LogLevel::Error,
                          "Failed to initialize ImGui Vulkan backend.");
      }
    } else if (renderingApi == RenderingApi::Dx12) {
      if (!ImGui_ImplSDL2_InitForD3D(window->getNativeWindowHandle())) {
        GlobalLogger::Log(LogLevel::Error,
                          "Failed to initialize ImGui SDL2 backend.");
      }

      imguiDescriptorHeap = std::make_shared<DescriptorHeapDx12>();
      imguiDescriptorHeap->initialize(EDescriptorHeapTypeDX12::CBV_SRV_UAV,
                                      true);

      auto fontDescriptor = imguiDescriptorHeap->alloc();
      //auto fontDescriptor1 = imguiDescriptorHeap->alloc();

      // auto descriptorHeapDx12 =
      // g_rhiDx12->m_descriptorHeaps_.getCurrentHeap();

      ImGui_ImplDX12_InitInfo initInfo = {};
      initInfo.Device                  = g_rhiDx12->m_device_.Get();
      initInfo.NumFramesInFlight
          = g_rhiDx12->m_swapchain_->getNumOfSwapchainImages();
      initInfo.RTVFormat
          = g_getDX12TextureFormat(g_rhiDx12->m_swapchain_->getFormat());
      // initInfo.SrvDescriptorHeap = descriptorHeapDx12->m_heap_.Get();
      // initInfo.LegacySingleSrvCpuDescriptor
      //     =
      //     descriptorHeapDx12->m_heap_->GetCPUDescriptorHandleForHeapStart();
      // initInfo.LegacySingleSrvGpuDescriptor
      //     =
      //     descriptorHeapDx12->m_heap_->GetGPUDescriptorHandleForHeapStart();
      initInfo.SrvDescriptorHeap = imguiDescriptorHeap->m_heap_.Get();
      initInfo.LegacySingleSrvCpuDescriptor
          = imguiDescriptorHeap->m_heap_->GetCPUDescriptorHandleForHeapStart();
      initInfo.LegacySingleSrvGpuDescriptor
          = imguiDescriptorHeap->m_heap_->GetGPUDescriptorHandleForHeapStart();

      ImGui_ImplDX12_Init(&initInfo);

      ImGui_ImplDX12_CreateDeviceObjects();
    }
  }

  // TODO: refactor method (separate into smaller methods - log,
  // performance)
  void render(const std::shared_ptr<RenderContext>& renderContext) {
    auto renderingApi = RuntimeSettings::s_get().getRenderingApi();

    auto commandBuffer
        = renderContext->renderFrameContext->getActiveCommandBuffer();

    if (renderingApi == RenderingApi::Dx12) {
      auto commandBufferDx12
          = std::static_pointer_cast<CommandBufferDx12>(commandBuffer);
      auto nativeCommandBuffer
          = (ID3D12GraphicsCommandList*)commandBufferDx12->getNativeHandle();

      ID3D12DescriptorHeap* descriptorHeaps[]
          = {imguiDescriptorHeap->m_heap_.Get()};
      nativeCommandBuffer->SetDescriptorHeaps(std::size(descriptorHeaps),
                                              descriptorHeaps);
    }

    // change from COLOR_ATTACHMENT to SHADER_READ_ONLY before render pass
    g_rhi->transitionLayout(
        commandBuffer,
        renderContext->renderFrameContext->m_sceneRenderTarget_->m_colorBuffer_
            ->m_texture_,
        EResourceLayout::SHADER_READ_ONLY);

    g_rhi->transitionLayout(
        commandBuffer,
        renderContext->renderFrameContext->m_sceneRenderTarget_->m_backBuffer_
            ->m_texture_,
        EResourceLayout::ATTACHMENT);

    if (renderingApi == RenderingApi::Vulkan) {
      ImGui_ImplVulkan_NewFrame();
    } else if (renderingApi == RenderingApi::Dx12) {
      ImGui_ImplDX12_NewFrame();
    }
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport();

    renderPass_ = createRenderPass_(renderContext);
    renderPass_->beginRenderPass(commandBuffer);

    // FPS Window
    {
      auto fps       = ServiceLocator::s_get<TimingManager>()->getFPS();
      auto frameTime = ServiceLocator::s_get<TimingManager>()->getFrameTime();

      ImGui::Begin("Performance");
      ImGui::Text("FPS: %.1f", fps);
      ImGui::Text("Frame Time: %.1f ms", frameTime);
      ImGui::End();
    }

    // Log Window
    //    {
    //      // use dynamic_pointer_cast instead of static_pointer_cast to make
    //      it
    //      // optional
    //      auto logger = std::static_pointer_cast<MemoryLogger>(
    //          GlobalLogger::GetLogger("memory_logger"));
    //
    //      auto logs = logger->getLogEntries();
    //
    //      ImGui::Begin("Log");
    //
    //      if (!logs.empty()) {
    //        ImGui::BeginChild("LogScrollRegion",
    //                          ImVec2(0, 0),
    //                          true,
    //                          ImGuiWindowFlags_AlwaysVerticalScrollbar);
    //
    //        for (const auto& entry : logs) {
    //          // TODO: use my custom class for timings
    //          auto    timeT =
    //          std::chrono::system_clock::to_time_t(entry.timestamp); std::tm
    //          timeStruct{};
    // #ifdef _WIN32
    //          localtime_s(&timeStruct, &timeT);
    // #else
    //          localtime_r(&timeT, &timeStruct);
    // #endif
    //          char timeBuffer[64];
    //          std::strftime(
    //              timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S",
    //              &timeStruct);
    //
    //          ImVec4 color;
    //          switch (entry.level) {
    //            case LogLevel::Info:
    //              color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
    //              break;  // Green
    //            case LogLevel::Warning:
    //              color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    //              break;  // Yellow
    //            case LogLevel::Error:
    //              color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    //              break;  // Red
    //            default:
    //              color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    //              break;  // White
    //          }
    //
    //          std::string logLevel;
    //          switch (entry.level) {
    //            case LogLevel::Info:
    //              logLevel = "Info";
    //            case LogLevel::Warning:
    //              logLevel = "Warning";
    //            case LogLevel::Error:
    //              logLevel = "Error";
    //            default:
    //              logLevel = "Unknown";
    //          }
    //
    //          ImGui::PushStyleColor(ImGuiCol_Text, color);
    //          ImGui::Text("[%s] [%s] %s",
    //                      timeBuffer,
    //                      logLevel.c_str(),
    //                      entry.message.c_str());
    //          ImGui::PopStyleColor();
    //        }
    //
    //        ImGui::EndChild();
    //      } else {
    //        ImGui::Text("No logs available.");
    //      }
    //
    //      ImGui::End();
    //    }

    // Render Window
    {
      ImGui::Begin("Render Window");

      ImVec2 renderWindow = ImGui::GetContentRegionAvail();

      if (renderWindow.x <= 0.0f) {
        renderWindow.x = 1.0f;
      }
      if (renderWindow.y <= 0.0f) {
        renderWindow.y = 1.0f;
      }

      renderParams.editorViewportDimension
          = math::Dimension2Di(renderWindow.x, renderWindow.y);
      // renderParams.editorViewportDimension.width() = renderWindow.x;
      // renderParams.editorViewportDimension.height() = renderWindow.y;

      updateImGuiTexture_(renderContext);

      ImGui::Image(imguiTextureID, renderWindow);
      ImGui::End();
    }

    // Render Mode Window
    {
      ImGui::Begin("Render Mode");

      if (ImGui::RadioButton("Solid",
                             renderParams.renderMode == RenderMode::Solid)) {
        renderParams.renderMode = RenderMode::Solid;
      }
      if (ImGui::RadioButton(
              "Wireframe", renderParams.renderMode == RenderMode::Wireframe)) {
        renderParams.renderMode = RenderMode::Wireframe;
      }
      if (ImGui::RadioButton(
              "Normal Map Visualization",
              renderParams.renderMode == RenderMode::NormalMapVisualization)) {
        renderParams.renderMode = RenderMode::NormalMapVisualization;
      }
      if (ImGui::RadioButton("Vertex Normal Visualization",
                             renderParams.renderMode
                                 == RenderMode::VertexNormalVisualization)) {
        renderParams.renderMode = RenderMode::VertexNormalVisualization;
      }
      if (ImGui::RadioButton(
              "Shader Overdraw",
              renderParams.renderMode == RenderMode::ShaderOverdraw)) {
        renderParams.renderMode = RenderMode::ShaderOverdraw;
      }

      ImGui::End();
    }

    // Post Process Window
    {
      ImGui::Begin("Post Process Mode");

      if (ImGui::RadioButton(
              "None", renderParams.postProcessMode == PostProcessMode::None)) {
        renderParams.postProcessMode = PostProcessMode::None;
      }
      if (ImGui::RadioButton(
              "Grayscale",
              renderParams.postProcessMode == PostProcessMode::Grayscale)) {
        renderParams.postProcessMode = PostProcessMode::Grayscale;
      }
      if (ImGui::RadioButton("Color Inversion",
                             renderParams.postProcessMode
                                 == PostProcessMode::ColorInversion)) {
        renderParams.postProcessMode = PostProcessMode::ColorInversion;
      }

      ImGui::End();
    }

    ImGui::Render();
    ImDrawData* drawData = ImGui::GetDrawData();
    if (drawData != nullptr) {
      auto commandBuffer
          = renderContext->renderFrameContext->getActiveCommandBuffer();
      if (renderingApi == RenderingApi::Vulkan) {
        auto commandBufferVk
            = std::static_pointer_cast<CommandBufferVk>(commandBuffer);
        auto nativeCommandBuffer
            = (VkCommandBuffer)commandBufferVk->getNativeHandle();
        ImGui_ImplVulkan_RenderDrawData(drawData, nativeCommandBuffer);

      } else if (renderingApi == RenderingApi::Dx12) {
        auto commandBufferDx12
            = std::static_pointer_cast<CommandBufferDx12>(commandBuffer);
        auto nativeCommandBuffer
            = (ID3D12GraphicsCommandList*)commandBufferDx12->getNativeHandle();
        ImGui_ImplDX12_RenderDrawData(drawData, nativeCommandBuffer);
      }
    }

    // Handle additional viewports
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
    }

    renderPass_->endRenderPass();

    g_rhi->transitionLayout(
        commandBuffer,
        renderContext->renderFrameContext->m_sceneRenderTarget_->m_backBuffer_
            ->m_texture_,
        EResourceLayout::PRESENT_SRC);
  }

  private:
  RenderPass* createRenderPass_(
      const std::shared_ptr<RenderContext>& renderContext) {
    // auto viewportDimension = renderParams.editorViewportDimension;

    math::Dimension2Di            viewportDimension;
    std::shared_ptr<RenderTarget> backBuffer;

    if (renderContext) {
      viewportDimension = renderContext->viewportDimension;
      backBuffer = renderContext->renderFrameContext->m_sceneRenderTarget_
                       ->m_backBuffer_;
    } else {
      viewportDimension = math::Dimension2Di(0, 0);
      backBuffer        = RenderTarget::s_createFromTexture(
          g_rhiVk->m_swapchain_
              ->getSwapchainImage(g_rhiVk->m_currentFrameIndex_)
              ->m_texture_);
    }

    auto clearColor = RtClearValue(0.0f, 0.0f, 0.0f, 1.0f);
    // const RtClearValue clearDepth = RtClearValue(1.0f, 0);

    Attachment colorAttachment(backBuffer,
                               EAttachmentLoadStoreOp::CLEAR_STORE,
                               EAttachmentLoadStoreOp::DONTCARE_DONTCARE,
                               clearColor,
                               EResourceLayout::UNDEFINED,
                               EResourceLayout::PRESENT_SRC);

    // Attachment depthAttachment(
    // renderContext->renderFrameContext->m_sceneRenderTarget_->m_depthBuffer_,
    //     EAttachmentLoadStoreOp::CLEAR_STORE,
    //     EAttachmentLoadStoreOp::DONTCARE_DONTCARE,
    //     clearDepth,
    //     EResourceLayout::UNDEFINED,
    //     EResourceLayout::DEPTH_STENCIL_ATTACHMENT);

    RenderPassInfo renderPassInfo;
    renderPassInfo.m_attachments_.push_back(colorAttachment);
    // renderPassInfo.m_attachments_.push_back(depthAttachment);

    Subpass subpass;
    subpass.initialize(0, /*sourceSubpassIndex*/
                       1, /*destSubpassIndex*/
                       EPipelineStageMask::COLOR_ATTACHMENT_OUTPUT_BIT,
                       EPipelineStageMask::FRAGMENT_SHADER_BIT);

    subpass.m_outputColorAttachments_.push_back(0);
    // subpass.m_outputDepthAttachment_ = 0;

    renderPassInfo.m_subpasses_.push_back(subpass);

    return g_rhi->getOrCreateRenderPass(
        renderPassInfo,
        {0, 0},
        {viewportDimension.width(), viewportDimension.height()});
  }

  void updateImGuiTexture_(
      const std::shared_ptr<RenderContext>& renderContext) {
    auto renderApi = RuntimeSettings::s_get().getRenderingApi();

    // Remove the old texture if necessary
    // Note: ImGui does not provide a direct way to remove textures.
    // If managing multiple textures, consider implementing a texture manager.

    auto texture = renderContext->renderFrameContext->m_sceneRenderTarget_
                       ->m_colorBuffer_->m_texture_;

    if (renderApi == RenderingApi::Vulkan) {
      // VkSamplerCreateInfo samplerInfo{};
      // samplerInfo.sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
      // samplerInfo.magFilter        = VK_FILTER_LINEAR;
      // samplerInfo.minFilter        = VK_FILTER_LINEAR;
      // samplerInfo.addressModeU     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
      // samplerInfo.addressModeV     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
      // samplerInfo.addressModeW     = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
      // samplerInfo.anisotropyEnable = VK_TRUE;
      // samplerInfo.maxAnisotropy    = 16;
      // samplerInfo.borderColor      = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
      // samplerInfo.unnormalizedCoordinates = VK_FALSE;
      // samplerInfo.compareEnable           = VK_FALSE;
      // samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
      // samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
      // samplerInfo.mipLodBias              = 0.0f;
      // samplerInfo.minLod                  = 0.0f;
      // samplerInfo.maxLod                  = 0.0f;

      // VkSampler textureSampler;
      // if (vkCreateSampler(
      //         g_rhiVk->m_device_, &samplerInfo, nullptr, &textureSampler)
      //     != VK_SUCCESS) {
      //   GlobalLogger::Log(LogLevel::Error, "Failed to create texture
      //   sampler");
      //   // TODO: Cleanup and return false
      // }

      auto textureVk = std::static_pointer_cast<TextureVk>(texture);

      if (imguiTextureID) {
        VkDescriptorSet oldDescSet
            = reinterpret_cast<VkDescriptorSet>(imguiTextureID);

        // Free the old descriptor set from descriptor pool
        vkFreeDescriptorSets(
            g_rhiVk->m_device_,
            g_rhiVk->getDescriptorPoolMultiFrame()->m_descriptorPool_,
            1,
            &oldDescSet);

        // Reset our ImTextureID so ImGui no longer points to a freed descriptor
        imguiTextureID = 0;
      }

      // Add the new texture
      VkDescriptorSet descriptorSet = ImGui_ImplVulkan_AddTexture(
          TextureVk::s_createDefaultSamplerState(),
          textureVk->m_imageView_,
          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

      imguiTextureID = reinterpret_cast<ImTextureID>(descriptorSet);

    } else if (renderApi == RenderingApi::Dx12) {
      auto textureDx12 = std::static_pointer_cast<TextureDx12>(texture);

      auto srcCpuHandle = textureDx12->m_srv_.m_cpuHandle_;

      static DescriptorDx12 dstDescriptor;
      static bool           firstTime = true;
      if (firstTime) {
        dstDescriptor = imguiDescriptorHeap->alloc();
        firstTime     = false;
      }

      g_rhiDx12->m_device_->CopyDescriptorsSimple(
          1,
          dstDescriptor.m_cpuHandle_,
          srcCpuHandle,
          D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

      auto descriptorIndex = dstDescriptor.m_gpuHandle_;

      imguiTextureID = (ImTextureID)descriptorIndex.ptr;
    }
  }

  ImTextureID imguiTextureID;

  EditorRenderParams renderParams;

  RenderPass* renderPass_ = nullptr;

  // TODO: remove (not api agnostic)
  std::shared_ptr<DescriptorHeapDx12> imguiDescriptorHeap;
};
}  // namespace game_engine

#endif  // GAME_ENGINE_EDITOR_H
