#ifndef GAME_ENGINE_EDITOR_H
#define GAME_ENGINE_EDITOR_H

#include "config/config_manager.h"
#include "gfx/renderer/renderer.h"
#include "gfx/rhi/backends/dx12/command_buffer_dx12.h"
#include "gfx/rhi/backends/dx12/device_dx12.h"
#include "gfx/rhi/backends/dx12/texture_dx12.h"
#include "gfx/rhi/backends/vulkan/command_buffer_vk.h"
#include "gfx/rhi/backends/vulkan/device_vk.h"
#include "gfx/rhi/backends/vulkan/texture_vk.h"
#include "gfx/rhi/common/rhi_enums.h"
#include "gfx/rhi/interface/command_buffer.h"
#include "gfx/rhi/interface/descriptor.h"
#include "gfx/rhi/interface/device.h"
#include "gfx/rhi/interface/texture.h"
#include "utils/logger/global_logger.h"
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
  auto getRenderParams() -> const gfx::renderer::RenderSettings& { return m_renderParams; }

  void init(Window* window, gfx::rhi::RenderingApi renderingApi, gfx::rhi::Device* device) {
    m_window = window;

    m_renderingApi = renderingApi;

    m_device = device;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui::StyleColorsDark();

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      ImGuiStyle& style                 = ImGui::GetStyle();
      style.WindowRounding              = 0.0f;
      style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    if (m_renderingApi == gfx::rhi::RenderingApi::Vulkan) {
      if (!ImGui_ImplSDL2_InitForVulkan(window->getNativeWindowHandle())) {
        GlobalLogger::Log(LogLevel::Error, "Failed to initialize ImGui SDL2 backend for Vulkan.");
      }
    } else if (m_renderingApi == gfx::rhi::RenderingApi::Dx12) {
      if (!ImGui_ImplSDL2_InitForD3D(window->getNativeWindowHandle())) {
        GlobalLogger::Log(LogLevel::Error, "Failed to initialize ImGui SDL2 backend for D3D.");
      }
    }

    initRenderBackend(device);
  }

  void render(gfx::renderer::RenderContext& context) {
    if (m_renderingApi == gfx::rhi::RenderingApi::Vulkan) {
      ImGui_ImplVulkan_NewFrame();
    } else if (m_renderingApi == gfx::rhi::RenderingApi::Dx12) {
      ImGui_ImplDX12_NewFrame();
    }
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport();

    renderPerformanceWindow();
    renderViewportWindow(context);
    renderModeSelectionWindow();
    renderPostProcessWindow();
    renderApplicationModeWindow();

    ImGui::Render();

    renderImGuiDrawData(context);

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
    }
  }

  ~Editor() {
    if (m_renderingApi == gfx::rhi::RenderingApi::Vulkan) {
      ImGui_ImplVulkan_Shutdown();
    } else if (m_renderingApi == gfx::rhi::RenderingApi::Dx12) {
      ImGui_ImplDX12_Shutdown();
    }
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
  }

  private:
  void initRenderBackend(gfx::rhi::Device* device) {
    m_device       = device;
    m_renderingApi = device->getApiType();

    if (m_renderingApi == gfx::rhi::RenderingApi::Vulkan) {
      initImGuiVulkan();
    } else if (m_renderingApi == gfx::rhi::RenderingApi::Dx12) {
      initImGuiDx12();
    }
  }

  void initImGuiVulkan() {
    GlobalLogger::Log(LogLevel::Info, "Initializing ImGui Vulkan backend");

    auto deviceVk = static_cast<gfx::rhi::DeviceVk*>(m_device);

    // Create descriptor pool for ImGui
    VkDescriptorPoolSize pool_sizes[] = {
      {               VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
      {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
      {         VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
      {         VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
      {  VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
      {  VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
      {        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
      {        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
      {      VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets                    = 1000;
    pool_info.poolSizeCount              = std::size(pool_sizes);
    pool_info.pPoolSizes                 = pool_sizes;

    VkDescriptorPool imguiPool;
    auto             result = vkCreateDescriptorPool(deviceVk->getDevice(), &pool_info, nullptr, &imguiPool);
    if (result != VK_SUCCESS) {
      GlobalLogger::Log(LogLevel::Error, "Failed to create ImGui descriptor pool");
      return;
    }

    // Create render pass for ImGui
    VkRenderPass renderPass = createVulkanRenderPass();

    auto queueFamilyIndices = deviceVk->getQueueFamilyIndices().graphicsFamily.has_value()
                                ? deviceVk->getQueueFamilyIndices().graphicsFamily.value()
                                : 0;

    // Initialize ImGui Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance                  = deviceVk->getInstance();
    init_info.PhysicalDevice            = deviceVk->getPhysicalDevice();
    init_info.Device                    = deviceVk->getDevice();
    init_info.QueueFamily               = queueFamilyIndices;
    init_info.Queue                     = deviceVk->getGraphicsQueue();
    init_info.PipelineCache             = VK_NULL_HANDLE;
    init_info.DescriptorPool            = imguiPool;
    init_info.Allocator                 = nullptr;
    init_info.MinImageCount             = 2;
    init_info.ImageCount                = 2;
    init_info.CheckVkResultFn           = nullptr;
    init_info.RenderPass                = renderPass;

    ImGui_ImplVulkan_Init(&init_info);

    // Store ImGui resources
    m_imguiVulkanPool       = imguiPool;
    m_imguiVulkanRenderPass = renderPass;
  }

  void initImGuiDx12() {
    GlobalLogger::Log(LogLevel::Info, "Initializing ImGui DirectX12 backend");

    // Get DX12 device
    auto deviceDx12 = static_cast<gfx::rhi::DeviceDx12*>(m_device);
    auto dx12Device = deviceDx12->getDevice();

    // Create descriptor heap for ImGui
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type                       = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors             = 1000;
    desc.Flags                      = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    ID3D12DescriptorHeap* imguiHeap;
    auto                  hr = dx12Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&imguiHeap));
    if (FAILED(hr)) {
      GlobalLogger::Log(LogLevel::Error, "Failed to create ImGui descriptor heap");
      return;
    }

    // Initialize ImGui DX12
    ImGui_ImplDX12_InitInfo init_info      = {};
    init_info.Device                       = dx12Device;
    init_info.NumFramesInFlight            = 2;
    init_info.RTVFormat                    = DXGI_FORMAT_B8G8R8A8_UNORM;
    init_info.SrvDescriptorHeap            = imguiHeap;
    init_info.LegacySingleSrvCpuDescriptor = imguiHeap->GetCPUDescriptorHandleForHeapStart();
    init_info.LegacySingleSrvGpuDescriptor = imguiHeap->GetGPUDescriptorHandleForHeapStart();

    ImGui_ImplDX12_Init(&init_info);
    ImGui_ImplDX12_CreateDeviceObjects();

    // Store ImGui resources
    m_imguiDx12Heap = imguiHeap;
  }

  VkRenderPass createVulkanRenderPass() {
    VkAttachmentDescription attachment = {};
    attachment.format                  = VK_FORMAT_B8G8R8A8_UNORM;    // Common swapchain format
    attachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_LOAD;  // We want to draw over existing content
    attachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout           = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout             = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachment = {};
    colorAttachment.attachment            = 0;
    colorAttachment.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorAttachment;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass          = 0;
    dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask       = 0;
    dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo info = {};
    info.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount        = 1;
    info.pAttachments           = &attachment;
    info.subpassCount           = 1;
    info.pSubpasses             = &subpass;
    info.dependencyCount        = 1;
    info.pDependencies          = &dependency;

    auto deviceVk = static_cast<gfx::rhi::DeviceVk*>(m_device);

    VkRenderPass renderPass;
    auto         result = vkCreateRenderPass(deviceVk->getDevice(), &info, nullptr, &renderPass);
    if (result != VK_SUCCESS) {
      GlobalLogger::Log(LogLevel::Error, "Failed to create ImGui render pass");
      return VK_NULL_HANDLE;
    }

    return renderPass;
  }

  void renderPerformanceWindow() {
    auto timingManager = ServiceLocator::s_get<TimingManager>();
    if (timingManager) {
      auto fps       = timingManager->getFPS();
      auto frameTime = timingManager->getFrameTime();

      ImGui::Begin("Performance");
      ImGui::Text("FPS: %.1f", fps);
      ImGui::Text("Frame Time: %.1f ms", frameTime);

      // Frame time history graph
      static const int historyCount                   = 300;
      static float     frameTimeHistory[historyCount] = {0};
      static int       historyIndex                   = 0;

      frameTimeHistory[historyIndex] = frameTime;
      historyIndex                   = (historyIndex + 1) % historyCount;

      ImGui::PlotLines(
          "Frame Time (ms)", frameTimeHistory, historyCount, historyIndex, nullptr, 0.0f, FLT_MAX, ImVec2(0, 80));

      ImGui::End();
    }
  }

  void renderViewportWindow(gfx::renderer::RenderContext& context) {
    ImGui::Begin("Render Window");

    ImVec2 renderWindow = ImGui::GetContentRegionAvail();

    if (renderWindow.x <= 0.0f) {
      renderWindow.x = 1.0f;
    }
    if (renderWindow.y <= 0.0f) {
      renderWindow.y = 1.0f;
    }

    m_renderParams.renderViewportDimension = math::Dimension2Di(renderWindow.x, renderWindow.y);

    // Update texture ID for ImGui
    updateImGuiTexture(context);

    // Display the rendered scene
    ImGui::Image(m_imguiTextureID, renderWindow);

    ImGui::End();
  }

  void renderModeSelectionWindow() {
    ImGui::Begin("Render Mode");

    if (ImGui::RadioButton("Solid", m_renderParams.renderMode == gfx::renderer::RenderMode::Solid)) {
      m_renderParams.renderMode = gfx::renderer::RenderMode::Solid;
    }
    if (ImGui::RadioButton("Wireframe", m_renderParams.renderMode == gfx::renderer::RenderMode::Wireframe)) {
      m_renderParams.renderMode = gfx::renderer::RenderMode::Wireframe;
    }
    if (ImGui::RadioButton("Normal Map Visualization",
                           m_renderParams.renderMode == gfx::renderer::RenderMode::NormalMapVisualization)) {
      m_renderParams.renderMode = gfx::renderer::RenderMode::NormalMapVisualization;
    }
    if (ImGui::RadioButton("Vertex Normal Visualization",
                           m_renderParams.renderMode == gfx::renderer::RenderMode::VertexNormalVisualization)) {
      m_renderParams.renderMode = gfx::renderer::RenderMode::VertexNormalVisualization;
    }
    if (ImGui::RadioButton("Shader Overdraw", m_renderParams.renderMode == gfx::renderer::RenderMode::ShaderOverdraw)) {
      m_renderParams.renderMode = gfx::renderer::RenderMode::ShaderOverdraw;
    }

    ImGui::End();
  }

  void renderPostProcessWindow() {
    ImGui::Begin("Post Process Mode");

    if (ImGui::RadioButton("None", m_renderParams.postProcessMode == gfx::renderer::PostProcessMode::None)) {
      m_renderParams.postProcessMode = gfx::renderer::PostProcessMode::None;
    }
    if (ImGui::RadioButton("Grayscale", m_renderParams.postProcessMode == gfx::renderer::PostProcessMode::Grayscale)) {
      m_renderParams.postProcessMode = gfx::renderer::PostProcessMode::Grayscale;
    }
    if (ImGui::RadioButton("Color Inversion",
                           m_renderParams.postProcessMode == gfx::renderer::PostProcessMode::ColorInversion)) {
      m_renderParams.postProcessMode = gfx::renderer::PostProcessMode::ColorInversion;
    }

    ImGui::End();
  }

  void renderApplicationModeWindow() {
    ImGui::Begin("Application Mode");

    if (ImGui::RadioButton("Editor Mode", m_renderParams.appMode == gfx::renderer::ApplicationRenderMode::Editor)) {
      m_renderParams.appMode = gfx::renderer::ApplicationRenderMode::Editor;
    }
    if (ImGui::RadioButton("Game Mode", m_renderParams.appMode == gfx::renderer::ApplicationRenderMode::Game)) {
      m_renderParams.appMode = gfx::renderer::ApplicationRenderMode::Game;
    }

    ImGui::End();
  }

  void updateImGuiTexture(gfx::renderer::RenderContext& context) {
    // Get the color buffer from the render context
    auto colorTexture = context.renderTarget->colorBuffer.get();

    if (!colorTexture) {
      GlobalLogger::Log(LogLevel::Error, "Color texture is null in updateImGuiTexture");
      return;
    }

    if (m_renderingApi == gfx::rhi::RenderingApi::Vulkan) {
      // Get the Vulkan texture and image view
      auto textureVk   = static_cast<gfx::rhi::TextureVk*>(colorTexture);
      auto vkImageView = textureVk->getImageView();

      // Create or update descriptor set
      if (m_imguiTextureID) {
        auto            deviceVk = static_cast<gfx::rhi::DeviceVk*>(m_device);
        // Free old descriptor set if exists
        VkDescriptorSet oldDescSet = reinterpret_cast<VkDescriptorSet>(m_imguiTextureID);
        vkFreeDescriptorSets(deviceVk->getDevice(), m_imguiVulkanPool, 1, &oldDescSet);
        m_imguiTextureID = 0;
      }

      // Get a sampler (could create a static one or get from a sampler cache)
      VkSampler sampler = getVulkanSampler();

      // Add the texture to ImGui
      VkDescriptorSet descriptorSet
          = ImGui_ImplVulkan_AddTexture(sampler, vkImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

      m_imguiTextureID = reinterpret_cast<ImTextureID>(descriptorSet);

    } else if (m_renderingApi == gfx::rhi::RenderingApi::Dx12) {
      auto deviceDx12  = static_cast<gfx::rhi::DeviceDx12*>(m_device);
      auto textureDx12 = static_cast<gfx::rhi::TextureDx12*>(colorTexture);

      // Get the D3D12 resource (ID3D12Resource) for the texture
      ID3D12Resource* textureResource = textureDx12->getResource();

      // Get the texture format from its descriptor
      DXGI_FORMAT textureFormat = textureResource->GetDesc().Format;

      // Allocate or use existing descriptor in the ImGui heap
      D3D12_CPU_DESCRIPTOR_HANDLE dstCpuHandle;
      D3D12_GPU_DESCRIPTOR_HANDLE dstGpuHandle;

      if (!m_imguiDescriptorAllocated) {
        // Get handles from the heap
        UINT descriptorSize
            = deviceDx12->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        dstCpuHandle = m_imguiDx12Heap->GetCPUDescriptorHandleForHeapStart();
        dstGpuHandle = m_imguiDx12Heap->GetGPUDescriptorHandleForHeapStart();

        // Offset to an unused descriptor (index 1, as 0 is used by ImGui font)
        dstCpuHandle.ptr += descriptorSize;
        dstGpuHandle.ptr += descriptorSize;

        m_imguiDescriptorAllocated = true;
        m_imguiDx12CpuHandle       = dstCpuHandle;
        m_imguiDx12GpuHandle       = dstGpuHandle;
      } else {
        dstCpuHandle = m_imguiDx12CpuHandle;
        dstGpuHandle = m_imguiDx12GpuHandle;
      }

      // Create SRV description
      D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
      srvDesc.Format                          = textureFormat;
      srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_TEXTURE2D;
      srvDesc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      srvDesc.Texture2D.MipLevels             = 1;
      srvDesc.Texture2D.MostDetailedMip       = 0;
      srvDesc.Texture2D.PlaneSlice            = 0;
      srvDesc.Texture2D.ResourceMinLODClamp   = 0.0f;

      // Create SRV directly in the ImGui heap instead of copying
      deviceDx12->getDevice()->CreateShaderResourceView(textureResource, &srvDesc, dstCpuHandle);

      // Store GPU handle for ImGui
      m_imguiTextureID = (ImTextureID)dstGpuHandle.ptr;
    }
  }

  VkSampler getVulkanSampler() {
    // If we already have a sampler, return it
    if (m_vulkanSampler != VK_NULL_HANDLE) {
      return m_vulkanSampler;
    }

    // Create a new sampler
    VkSamplerCreateInfo samplerInfo     = {};
    samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter               = VK_FILTER_LINEAR;
    samplerInfo.minFilter               = VK_FILTER_LINEAR;
    samplerInfo.addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable        = VK_TRUE;
    samplerInfo.maxAnisotropy           = 16;
    samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable           = VK_FALSE;
    samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias              = 0.0f;
    samplerInfo.minLod                  = 0.0f;
    samplerInfo.maxLod                  = 0.0f;

    auto deviceVk = static_cast<gfx::rhi::DeviceVk*>(m_device);

    VkSampler sampler;
    auto      result = vkCreateSampler(deviceVk->getDevice(), &samplerInfo, nullptr, &sampler);
    if (result != VK_SUCCESS) {
      GlobalLogger::Log(LogLevel::Error, "Failed to create texture sampler");
      return VK_NULL_HANDLE;
    }

    m_vulkanSampler = sampler;
    return sampler;
  }

  void renderImGuiDrawData(gfx::renderer::RenderContext& context) {
    auto        cmdBuffer = context.commandBuffer.get();
    ImDrawData* drawData  = ImGui::GetDrawData();

    if (!drawData || !cmdBuffer) {
      return;
    }

    if (m_renderingApi == gfx::rhi::RenderingApi::Vulkan) {
      // For Vulkan, we need the native handle
      auto vkCmdBuffer = static_cast<gfx::rhi::CommandBufferVk*>(cmdBuffer)->getCommandBuffer();

      // Begin render pass for ImGui rendering
      VkRenderPassBeginInfo info = {};
      info.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      info.renderPass            = m_imguiVulkanRenderPass;

      // Create a framebuffer for this specific render
      VkFramebuffer framebuffer = createVulkanFramebuffer(context.renderTarget->backBuffer, context.viewportDimension);
      info.framebuffer          = framebuffer;

      info.renderArea.extent.width  = context.viewportDimension.width();
      info.renderArea.extent.height = context.viewportDimension.height();

      vkCmdBeginRenderPass(vkCmdBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);

      // Render ImGui
      ImGui_ImplVulkan_RenderDrawData(drawData, vkCmdBuffer);

      vkCmdEndRenderPass(vkCmdBuffer);

      auto deviceVk = static_cast<gfx::rhi::DeviceVk*>(m_device);

      // Clean up temporary framebuffer
      vkDestroyFramebuffer(static_cast<VkDevice>(deviceVk->getDevice()), framebuffer, nullptr);

    } else if (m_renderingApi == gfx::rhi::RenderingApi::Dx12) {
      // For DX12, we need the native command list and set the descriptor heap
      auto dx12CmdList = static_cast<gfx::rhi::CommandBufferDx12*>(cmdBuffer)->getCommandList();


      auto renderTarget = context.renderTarget->backBuffer;


      auto textureDx12  = static_cast<gfx::rhi::TextureDx12*>(renderTarget);
      auto resource     = textureDx12->getResource();

      GlobalLogger::Log(LogLevel::Info,
                        "Resource: " + std::to_string(reinterpret_cast<uintptr_t>(resource))
                            + ", State: " + std::to_string(static_cast<int>(textureDx12->getResourceState())));

      // ѕереход состо€ни€ из COPY_SOURCE в RENDER_TARGET
      gfx::rhi::ResourceBarrierDesc barrierDesc;
      barrierDesc.texture   = renderTarget;
      barrierDesc.oldLayout = renderTarget->getCurrentLayoutType();      // или текущее состо€ние
      barrierDesc.newLayout = gfx::rhi::ResourceLayout::ColorAttachment;  // это соответствует RENDER_TARGET

      // ѕримен€ем барьер ресурса
      cmdBuffer->resourceBarrier(barrierDesc);

      // ”становка дескрипторных куч дл€ ImGui
      ID3D12DescriptorHeap* heaps[] = {m_imguiDx12Heap};
      dx12CmdList->SetDescriptorHeaps(1, heaps);

      // –ендеринг ImGui
      ImGui_ImplDX12_RenderDrawData(drawData, dx12CmdList);

      // ѕосле рендеринга ImGui, возможно, нужно вернуть ресурс в нужное состо€ние
      barrierDesc.oldLayout = gfx::rhi::ResourceLayout::ColorAttachment;
      barrierDesc.newLayout = gfx::rhi::ResourceLayout::PresentSrc;  // или нужное вам состо€ние
      cmdBuffer->resourceBarrier(barrierDesc);

    }
  }

  VkFramebuffer createVulkanFramebuffer(gfx::rhi::Texture* backBuffer, const math::Dimension2Di& dimensions) {
    auto vkTexture = static_cast<gfx::rhi::TextureVk*>(backBuffer);

    auto imageView = vkTexture->getImageView();

    VkFramebufferCreateInfo info = {};
    info.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.renderPass              = m_imguiVulkanRenderPass;
    info.attachmentCount         = 1;
    info.pAttachments            = &imageView;
    info.width                   = dimensions.width();
    info.height                  = dimensions.height();
    info.layers                  = 1;

    auto deviceVk = static_cast<gfx::rhi::DeviceVk*>(m_device);

    VkFramebuffer framebuffer;
    auto          result = vkCreateFramebuffer(deviceVk->getDevice(), &info, nullptr, &framebuffer);
    if (result != VK_SUCCESS) {
      GlobalLogger::Log(LogLevel::Error, "Failed to create ImGui framebuffer");
      return VK_NULL_HANDLE;
    }

    return framebuffer;
  }

  private:
  // Core references
  Window*                m_window       = nullptr;
  gfx::rhi::Device*      m_device       = nullptr;
  gfx::rhi::RenderingApi m_renderingApi = gfx::rhi::RenderingApi::Vulkan;

  // Editor state
  gfx::renderer::RenderSettings m_renderParams;
  ImTextureID                       m_imguiTextureID;

  // Vulkan specific members
  VkDescriptorPool m_imguiVulkanPool       = VK_NULL_HANDLE;
  VkRenderPass     m_imguiVulkanRenderPass = VK_NULL_HANDLE;
  VkSampler        m_vulkanSampler         = VK_NULL_HANDLE;

  // DX12 specific members
  ID3D12DescriptorHeap*       m_imguiDx12Heap            = nullptr;
  D3D12_CPU_DESCRIPTOR_HANDLE m_imguiDx12CpuHandle       = {};
  D3D12_GPU_DESCRIPTOR_HANDLE m_imguiDx12GpuHandle       = {};
  bool                        m_imguiDescriptorAllocated = false;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_EDITOR_H