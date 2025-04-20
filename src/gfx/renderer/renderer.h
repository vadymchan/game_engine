#ifndef GAME_ENGINE_RENDERER_H
#define GAME_ENGINE_RENDERER_H

// TODO:
// - add debug pass for rendering (bounding boxes, light sources, normal vectors
// etc.)
// - shadow pass for the future

#include "ecs/components/camera.h"
#include "ecs/components/light.h"
#include "ecs/components/render_model.h"
#include "ecs/components/transform.h"
#include "ecs/components/vertex.h"
#include "gfx/renderer/render_resource_manager.h"
#include "gfx/rhi/common/rhi_creators.h"
#include "gfx/rhi/common/rhi_enums.h"
#include "gfx/rhi/common/rhi_types.h"
#include "gfx/rhi/interface/command_buffer.h"
#include "gfx/rhi/interface/descriptor.h"
#include "gfx/rhi/interface/device.h"
#include "gfx/rhi/interface/framebuffer.h"
#include "gfx/rhi/interface/pipeline.h"
#include "gfx/rhi/interface/render_pass.h"
#include "gfx/rhi/interface/swap_chain.h"
#include "gfx/rhi/interface/synchronization.h"
#include "gfx/rhi/interface/texture.h"
#include "gfx/rhi/shader_manager.h"
#include "platform/common/window.h"
#include "scene/scene.h"
#include "utils/hot_reload/hot_reload_manager.h"
#include "utils/path_manager/path_manager.h"
#include "utils/service/service_locator.h"

namespace game_engine {
namespace gfx {
namespace renderer {

enum class ApplicationRenderMode {
  Editor,
  Game
};

enum class RenderMode {
  Solid,
  Wireframe,
  NormalMapVisualization,
  VertexNormalVisualization,
  ShaderOverdraw,
};

enum class PostProcessMode {
  None,
  Grayscale,
  ColorInversion
};

struct RenderSettings {
  RenderMode            renderMode              = RenderMode::Solid;
  PostProcessMode       postProcessMode         = PostProcessMode::None;
  math::Dimension2Di    renderViewportDimension = math::Dimension2Di(1, 1);
  ApplicationRenderMode appMode                 = ApplicationRenderMode::Game;
};

// TODO: Temp solution, consider aligning constant buffer under the hood
inline uint64_t alignConstantBufferSize(uint64_t size) {
  return (size + 255) & ~255;
}

class RenderPass;
class DebugDrawStrategy;

// Class to hold render targets for a frame
class SceneRenderTarget {
  public:
  SceneRenderTarget()  = default;
  ~SceneRenderTarget() = default;

  std::unique_ptr<rhi::Texture> colorBuffer;
  std::unique_ptr<rhi::Texture> depthBuffer;
  rhi::Texture*                 backBuffer = nullptr;
};

struct RenderContext {
  Scene*                              scene = nullptr;
  std::unique_ptr<rhi::CommandBuffer> commandBuffer;
  std::unique_ptr<SceneRenderTarget>  renderTarget;
  math::Dimension2Di                  viewportDimension;
  RenderSettings                      renderSettings;

  rhi::Device* device = nullptr;

  uint32_t        frameIndex      = 0;
  rhi::Semaphore* waitSemaphore   = nullptr;
  rhi::Semaphore* signalSemaphore = nullptr;
};

class RenderPass {
  public:
  virtual ~RenderPass() = default;

  virtual void initialize(RenderContext& context) = 0;

  virtual void execute(RenderContext& context) = 0;

  virtual void finalize() = 0;

  virtual bool isExclusive() const { return false; }
};

class BasePass : public RenderPass {
  public:
  BasePass(rhi::Device* device, rhi::ShaderManager* shaderManager)
      : m_device(device)
      , m_shaderManager(shaderManager) {
    m_resourceManager = ServiceLocator::s_get<RenderResourceManager>();

    if (!m_resourceManager) {
      ServiceLocator::s_provide<RenderResourceManager>();
      m_resourceManager = ServiceLocator::s_get<RenderResourceManager>();
    }
  }

  ~BasePass() override { finalize(); }

  void initialize(RenderContext& context) override {
    m_context = &context;

    setupPipelineStates_();
    setupRenderPass_();
    preparePerFrameResources_();
    prepareDefaultSamplerDescriptorSet_();
    prepareDrawCalls_();
  }

  void execute(RenderContext& context) override {
    auto commandBuffer = context.commandBuffer.get();
    if (!commandBuffer || !m_renderPass) {
      return;
    }

    rhi::ResourceBarrierDesc colorBarrier;
    colorBarrier.texture   = context.renderTarget->colorBuffer.get();
    colorBarrier.oldLayout = context.renderTarget->colorBuffer->getCurrentLayoutType();
    colorBarrier.newLayout = rhi::ResourceLayout::ColorAttachment;
    commandBuffer->resourceBarrier(colorBarrier);

    rhi::ResourceBarrierDesc depthBarrier;
    depthBarrier.texture   = context.renderTarget->depthBuffer.get();
    depthBarrier.oldLayout = context.renderTarget->depthBuffer->getCurrentLayoutType();
    depthBarrier.newLayout = context.renderTarget->depthBuffer->getFormat() == rhi::TextureFormat::D32
                               ? rhi::ResourceLayout::DepthAttachment
                               : rhi::ResourceLayout::DepthStencilAttachment;
    commandBuffer->resourceBarrier(depthBarrier);

    std::vector<rhi::ClearValue> clearValues;

    rhi::ClearValue colorClear;
    colorClear.color[0] = 0.0f;
    colorClear.color[1] = 0.0f;
    colorClear.color[2] = 0.0f;
    colorClear.color[3] = 1.0f;
    clearValues.push_back(colorClear);

    rhi::ClearValue depthClear;
    depthClear.depthStencil.depth   = 1.0f;
    depthClear.depthStencil.stencil = 0;
    clearValues.push_back(depthClear);

    commandBuffer->beginRenderPass(m_renderPass, m_framebuffer, clearValues);

    for (const auto& drawData : m_drawDataList) {
      commandBuffer->setPipeline(drawData.pipeline);

      commandBuffer->setViewport(m_viewport);
      commandBuffer->setScissor(m_scissor);

      if (m_viewDescriptorSet) {
        commandBuffer->bindDescriptorSet(0, m_viewDescriptorSet);
      }

      if (m_directionalLightDescriptorSet) {
        commandBuffer->bindDescriptorSet(1, m_directionalLightDescriptorSet);
      }

      if (m_pointLightDescriptorSet) {
        commandBuffer->bindDescriptorSet(2, m_pointLightDescriptorSet);
      }

      if (m_spotLightDescriptorSet) {
        commandBuffer->bindDescriptorSet(3, m_spotLightDescriptorSet);
      }

      if (drawData.materialDescriptorSet) {
        commandBuffer->bindDescriptorSet(4, drawData.materialDescriptorSet);
      }

      if (m_defaultSamplerDescriptorSet) {
        commandBuffer->bindDescriptorSet(5, m_defaultSamplerDescriptorSet);
      }

      commandBuffer->bindVertexBuffer(0, drawData.vertexBuffer);
      commandBuffer->bindVertexBuffer(1, drawData.instanceBuffer);
      commandBuffer->bindIndexBuffer(drawData.indexBuffer, 0, true);

      commandBuffer->drawIndexedInstanced(drawData.indexCount, drawData.instanceCount, 0, 0, 0);
    }

    commandBuffer->endRenderPass();
  }

  void finalize() override {
    m_drawDataList.clear();

    m_viewDescriptorSet             = nullptr;
    m_directionalLightDescriptorSet = nullptr;
    m_pointLightDescriptorSet       = nullptr;
    m_spotLightDescriptorSet        = nullptr;
    m_defaultSamplerDescriptorSet   = nullptr;
    m_renderPass                    = nullptr;
    m_framebuffer                   = nullptr;
    m_defaultSampler                = nullptr;
  }

  private:
  struct DrawData {
    rhi::GraphicsPipeline* pipeline              = nullptr;
    rhi::DescriptorSet*    materialDescriptorSet = nullptr;
    rhi::Buffer*           vertexBuffer          = nullptr;
    rhi::Buffer*           indexBuffer           = nullptr;
    rhi::Buffer*           instanceBuffer        = nullptr;
    uint32_t               indexCount            = 0;
    uint32_t               instanceCount         = 0;
  };

  void setupPipelineStates_() {
    auto viewportDimension = m_context->renderSettings.renderViewportDimension;

    m_viewport.x        = 0.0f;
    m_viewport.y        = 0.0f;
    m_viewport.width    = static_cast<float>(viewportDimension.width());
    m_viewport.height   = static_cast<float>(viewportDimension.height());
    m_viewport.minDepth = 0.0f;
    m_viewport.maxDepth = 1.0f;

    m_scissor.x      = 0;
    m_scissor.y      = 0;
    m_scissor.width  = viewportDimension.width();
    m_scissor.height = viewportDimension.height();

    m_vertexShader = m_shaderManager->getShader("assets/shaders/base_pass/shader_instancing.vs.hlsl");
    m_pixelShader  = m_shaderManager->getShader("assets/shaders/base_pass/shader.ps.hlsl");
  }

  void setupRenderPass_() {
    auto viewportDimension = m_context->renderSettings.renderViewportDimension;

    rhi::RenderPassDesc renderPassDesc;

    rhi::RenderPassAttachmentDesc colorAttachmentDesc;
    colorAttachmentDesc.format        = m_context->renderTarget->colorBuffer->getFormat();
    colorAttachmentDesc.samples       = rhi::MSAASamples::Count1;
    colorAttachmentDesc.loadStoreOp   = rhi::AttachmentLoadStoreOp::ClearStore;
    colorAttachmentDesc.initialLayout = rhi::ResourceLayout::Undefined;
    colorAttachmentDesc.finalLayout   = rhi::ResourceLayout::ShaderReadOnly;
    renderPassDesc.colorAttachments.push_back(colorAttachmentDesc);

    rhi::RenderPassAttachmentDesc depthAttachmentDesc;
    depthAttachmentDesc.format             = m_context->renderTarget->depthBuffer->getFormat();
    depthAttachmentDesc.samples            = rhi::MSAASamples::Count1;
    depthAttachmentDesc.loadStoreOp        = rhi::AttachmentLoadStoreOp::ClearStore;
    depthAttachmentDesc.stencilLoadStoreOp = rhi::AttachmentLoadStoreOp::DontcareDontcare;
    depthAttachmentDesc.initialLayout      = rhi::ResourceLayout::Undefined;
    depthAttachmentDesc.finalLayout        = rhi::ResourceLayout::DepthStencilAttachment;
    renderPassDesc.depthStencilAttachment  = depthAttachmentDesc;
    renderPassDesc.hasDepthStencil         = true;

    auto renderPass = m_device->createRenderPass(renderPassDesc);
    m_renderPass    = m_resourceManager->addRenderPass(std::move(renderPass), "base_pass_render_pass");

    rhi::FramebufferDesc framebufferDesc;
    framebufferDesc.width  = viewportDimension.width();
    framebufferDesc.height = viewportDimension.height();
    framebufferDesc.colorAttachments.push_back(m_context->renderTarget->colorBuffer.get());
    framebufferDesc.depthStencilAttachment = m_context->renderTarget->depthBuffer.get();
    framebufferDesc.hasDepthStencil        = true;
    framebufferDesc.renderPass             = m_renderPass;

    auto framebuffer = m_device->createFramebuffer(framebufferDesc);
    m_framebuffer    = m_resourceManager->addFramebuffer(std::move(framebuffer), "base_pass_framebuffer");
  }

  void preparePerFrameResources_() {
    auto& registry = m_context->scene->getEntityRegistry();
    auto  view     = registry.view<Transform, Camera, CameraMatrices>();

    if (view.begin() == view.end()) {
      GlobalLogger::Log(LogLevel::Warning, "No main Camera exists!");
      return;
    }

    auto  entity       = *view.begin();
    auto& transform    = view.get<Transform>(entity);
    auto& cameraMatrix = view.get<CameraMatrices>(entity);

    rhi::BufferDesc viewUboDesc;
    viewUboDesc.size = alignConstantBufferSize(sizeof(math::Matrix4f<>) * 3 + sizeof(math::Vector3Df) + sizeof(float));
    viewUboDesc.createFlags = rhi::BufferCreateFlag::CpuAccess | rhi::BufferCreateFlag::ConstantBuffer;
    viewUboDesc.type        = rhi::BufferType::Dynamic;

    auto viewUniformBuffer = m_device->createBuffer(viewUboDesc);

    struct ViewData {
      math::Matrix4f<> view;
      math::Matrix4f<> projection;
      math::Matrix4f<> viewProjection;
      math::Vector3Df  eyePosition;
      float            padding;
    } viewData;

    viewData.view           = cameraMatrix.view;
    viewData.projection     = cameraMatrix.projection;
    viewData.viewProjection = cameraMatrix.view * cameraMatrix.projection;
    viewData.eyePosition    = transform.translation;
    viewData.padding        = 0.0f;

    m_device->updateBuffer(viewUniformBuffer.get(), &viewData, sizeof(viewData));

    rhi::DescriptorSetLayoutDesc        viewLayoutDesc;
    rhi::DescriptorSetLayoutBindingDesc viewBindingDesc;
    viewBindingDesc.binding    = 0;
    viewBindingDesc.type       = rhi::ShaderBindingType::Uniformbuffer;
    viewBindingDesc.stageFlags = rhi::ShaderStageFlag::Vertex | rhi::ShaderStageFlag::Fragment;
    viewLayoutDesc.bindings.push_back(viewBindingDesc);

    auto viewSetLayout = m_device->createDescriptorSetLayout(viewLayoutDesc);

    auto viewDescriptorSet = m_device->createDescriptorSet(viewSetLayout.get());
    viewDescriptorSet->setUniformBuffer(0, viewUniformBuffer.get());

    auto bufferPtr      = m_resourceManager->addBuffer(std::move(viewUniformBuffer), "view_buffer");
    auto layoutPtr      = m_resourceManager->addDescriptorSetLayout(std::move(viewSetLayout), "view_set_layout");
    m_viewDescriptorSet = m_resourceManager->addDescriptorSet(std::move(viewDescriptorSet), "view_descriptor_set");

    createLightResources_();
  }

  void createLightResources_() {
    auto& registry = m_context->scene->getEntityRegistry();

    // DirectionalLight
    {
      auto dirLightView = registry.view<Light, DirectionalLight>();
      if (dirLightView.begin() != dirLightView.end()) {
        rhi::DescriptorSetLayoutDesc        lightLayoutDesc;
        rhi::DescriptorSetLayoutBindingDesc lightBindingDesc;
        lightBindingDesc.binding    = 0;
        lightBindingDesc.type       = rhi::ShaderBindingType::Uniformbuffer;
        lightBindingDesc.stageFlags = rhi::ShaderStageFlag::Fragment;
        lightLayoutDesc.bindings.push_back(lightBindingDesc);

        auto lightSetLayout = m_device->createDescriptorSetLayout(lightLayoutDesc);

        rhi::BufferDesc lightUboDesc;
        lightUboDesc.size        = alignConstantBufferSize(sizeof(float) * 8);
        lightUboDesc.createFlags = rhi::BufferCreateFlag::CpuAccess | rhi::BufferCreateFlag::ConstantBuffer;
        lightUboDesc.type        = rhi::BufferType::Dynamic;

        auto lightUniformBuffer = m_device->createBuffer(lightUboDesc);

        auto  entity   = *dirLightView.begin();
        auto& light    = dirLightView.get<Light>(entity);
        auto& dirLight = dirLightView.get<DirectionalLight>(entity);

        struct DirectionalLightData {
          float color[3];
          float intensity;
          float direction[3];
          float padding;
        } lightData;

        lightData.color[0]     = light.color.x();
        lightData.color[1]     = light.color.y();
        lightData.color[2]     = light.color.z();
        lightData.intensity    = light.intensity;
        lightData.direction[0] = dirLight.direction.x();
        lightData.direction[1] = dirLight.direction.y();
        lightData.direction[2] = dirLight.direction.z();
        lightData.padding      = 0.0f;

        m_device->updateBuffer(lightUniformBuffer.get(), &lightData, sizeof(lightData));

        auto lightDescriptorSet = m_device->createDescriptorSet(lightSetLayout.get());
        lightDescriptorSet->setUniformBuffer(0, lightUniformBuffer.get());

        auto bufferPtr = m_resourceManager->addBuffer(std::move(lightUniformBuffer), "directional_light_buffer");
        auto layoutPtr
            = m_resourceManager->addDescriptorSetLayout(std::move(lightSetLayout), "directional_light_layout");
        m_directionalLightDescriptorSet
            = m_resourceManager->addDescriptorSet(std::move(lightDescriptorSet), "directional_light_descriptor");
      }
    }

    // PointLight
    {
      auto pointLightView = registry.view<Light, PointLight, Transform>();
      if (pointLightView.begin() != pointLightView.end()) {
        rhi::DescriptorSetLayoutDesc        lightLayoutDesc;
        rhi::DescriptorSetLayoutBindingDesc lightBindingDesc;
        lightBindingDesc.binding    = 0;
        lightBindingDesc.type       = rhi::ShaderBindingType::Uniformbuffer;
        lightBindingDesc.stageFlags = rhi::ShaderStageFlag::Fragment;
        lightLayoutDesc.bindings.push_back(lightBindingDesc);

        auto lightSetLayout = m_device->createDescriptorSetLayout(lightLayoutDesc);

        rhi::BufferDesc lightUboDesc;
        lightUboDesc.size        = alignConstantBufferSize(sizeof(float) * 8);
        lightUboDesc.createFlags = rhi::BufferCreateFlag::CpuAccess | rhi::BufferCreateFlag::ConstantBuffer;
        lightUboDesc.type        = rhi::BufferType::Dynamic;

        auto lightUniformBuffer = m_device->createBuffer(lightUboDesc);

        auto  entity     = *pointLightView.begin();
        auto& light      = pointLightView.get<Light>(entity);
        auto& pointLight = pointLightView.get<PointLight>(entity);
        auto& transform  = pointLightView.get<Transform>(entity);

        struct PointLightData {
          float color[3];
          float intensity;
          float range;
          float position[3];
        } lightData;

        lightData.color[0]    = light.color.x();
        lightData.color[1]    = light.color.y();
        lightData.color[2]    = light.color.z();
        lightData.intensity   = light.intensity;
        lightData.range       = pointLight.range;
        lightData.position[0] = transform.translation.x();
        lightData.position[1] = transform.translation.y();
        lightData.position[2] = transform.translation.z();

        m_device->updateBuffer(lightUniformBuffer.get(), &lightData, sizeof(lightData));

        auto lightDescriptorSet = m_device->createDescriptorSet(lightSetLayout.get());
        lightDescriptorSet->setUniformBuffer(0, lightUniformBuffer.get());

        auto bufferPtr = m_resourceManager->addBuffer(std::move(lightUniformBuffer), "point_light_buffer");
        auto layoutPtr = m_resourceManager->addDescriptorSetLayout(std::move(lightSetLayout), "point_light_layout");
        m_pointLightDescriptorSet
            = m_resourceManager->addDescriptorSet(std::move(lightDescriptorSet), "point_light_descriptor");
      }
    }

    // SpotLight
    {
      auto spotLightView = registry.view<Light, SpotLight, Transform>();
      if (spotLightView.begin() != spotLightView.end()) {
        rhi::DescriptorSetLayoutDesc        lightLayoutDesc;
        rhi::DescriptorSetLayoutBindingDesc lightBindingDesc;
        lightBindingDesc.binding    = 0;
        lightBindingDesc.type       = rhi::ShaderBindingType::Uniformbuffer;
        lightBindingDesc.stageFlags = rhi::ShaderStageFlag::Fragment;
        lightLayoutDesc.bindings.push_back(lightBindingDesc);

        auto lightSetLayout = m_device->createDescriptorSetLayout(lightLayoutDesc);

        rhi::BufferDesc lightUboDesc;
        lightUboDesc.size        = alignConstantBufferSize(sizeof(float) * 16);
        lightUboDesc.createFlags = rhi::BufferCreateFlag::CpuAccess | rhi::BufferCreateFlag::ConstantBuffer;
        lightUboDesc.type        = rhi::BufferType::Dynamic;

        auto lightUniformBuffer = m_device->createBuffer(lightUboDesc);

        auto  entity    = *spotLightView.begin();
        auto& light     = spotLightView.get<Light>(entity);
        auto& spotLight = spotLightView.get<SpotLight>(entity);
        auto& transform = spotLightView.get<Transform>(entity);

        struct SpotLightData {
          float color[3];
          float intensity;
          float range;
          float innerConeAngle;
          float outerConeAngle;
          float padding1;
          float position[3];
          float padding2;
          float direction[3];
          float padding3;
        } lightData;

        lightData.color[0]       = light.color.x();
        lightData.color[1]       = light.color.y();
        lightData.color[2]       = light.color.z();
        lightData.intensity      = light.intensity;
        lightData.range          = spotLight.range;
        lightData.innerConeAngle = spotLight.innerConeAngle;
        lightData.outerConeAngle = spotLight.outerConeAngle;
        lightData.padding1       = 0.0f;
        lightData.position[0]    = transform.translation.x();
        lightData.position[1]    = transform.translation.y();
        lightData.position[2]    = transform.translation.z();
        lightData.padding2       = 0.0f;

        lightData.direction[0] = transform.rotation.x();
        lightData.direction[1] = transform.rotation.y();
        lightData.direction[2] = transform.rotation.z();
        lightData.padding3     = 0.0f;

        m_device->updateBuffer(lightUniformBuffer.get(), &lightData, sizeof(lightData));

        auto lightDescriptorSet = m_device->createDescriptorSet(lightSetLayout.get());
        lightDescriptorSet->setUniformBuffer(0, lightUniformBuffer.get());

        auto bufferPtr = m_resourceManager->addBuffer(std::move(lightUniformBuffer), "spot_light_buffer");
        auto layoutPtr = m_resourceManager->addDescriptorSetLayout(std::move(lightSetLayout), "spot_light_layout");
        m_spotLightDescriptorSet
            = m_resourceManager->addDescriptorSet(std::move(lightDescriptorSet), "spot_light_descriptor");
      }
    }
  }

  void prepareDefaultSamplerDescriptorSet_() {
    rhi::DescriptorSetLayoutDesc        samplerLayoutDesc;
    rhi::DescriptorSetLayoutBindingDesc samplerBindingDesc;
    samplerBindingDesc.binding    = 0;
    samplerBindingDesc.type       = rhi::ShaderBindingType::Sampler;
    samplerBindingDesc.stageFlags = rhi::ShaderStageFlag::Fragment;
    samplerLayoutDesc.bindings.push_back(samplerBindingDesc);

    std::string layoutKey        = "default_sampler_layout";
    auto        samplerSetLayout = m_resourceManager->getDescriptorSetLayout(layoutKey);

    if (!samplerSetLayout) {
      auto newLayout   = m_device->createDescriptorSetLayout(samplerLayoutDesc);
      samplerSetLayout = m_resourceManager->addDescriptorSetLayout(std::move(newLayout), layoutKey);
    }

    if (!m_defaultSampler) {
      m_defaultSampler = m_resourceManager->getSampler("default_sampler");

      if (!m_defaultSampler) {
        rhi::SamplerDesc samplerDesc;
        samplerDesc.minFilter     = rhi::TextureFilter::Linear;
        samplerDesc.magFilter     = rhi::TextureFilter::Linear;
        samplerDesc.addressModeU  = rhi::TextureAddressMode::Repeat;
        samplerDesc.addressModeV  = rhi::TextureAddressMode::Repeat;
        samplerDesc.addressModeW  = rhi::TextureAddressMode::Repeat;
        samplerDesc.maxAnisotropy = 16.0f;

        auto sampler     = m_device->createSampler(samplerDesc);
        m_defaultSampler = m_resourceManager->addSampler(std::move(sampler), "default_sampler");
      }
    }

    auto samplerDescriptorSet = m_device->createDescriptorSet(samplerSetLayout);
    samplerDescriptorSet->setSampler(0, m_defaultSampler);

    m_defaultSamplerDescriptorSet
        = m_resourceManager->addDescriptorSet(std::move(samplerDescriptorSet), "default_sampler_descriptor_set");
  }

  void prepareDrawCalls_() {
    auto& registry = m_context->scene->getEntityRegistry();
    auto  view     = registry.view<Transform, RenderModel*>();

    std::unordered_map<RenderModel*, std::vector<math::Matrix4f<>>> modelInstances;

    for (auto entity : view) {
      auto& transform   = view.get<Transform>(entity);
      auto& renderModel = view.get<RenderModel*>(entity);

      math::Matrix4f<> modelMatrix = calculateTransformMatrix(transform);
      modelInstances[renderModel].push_back(modelMatrix);
    }

    for (const auto& [renderModel, modelMatrices] : modelInstances) {
      auto instanceBuffer = createInstanceBuffer_(modelMatrices);

      auto instanceBufferPtr = m_resourceManager->addBuffer(std::move(instanceBuffer));

      for (const auto& renderMesh : renderModel->renderMeshes) {
        auto materialDescriptorSet = createMaterialDescriptorSet_(renderMesh->material);

        rhi::DescriptorSet* materialSetPtr = nullptr;
        if (materialDescriptorSet) {
          std::string materialKey = "material_" + std::to_string(reinterpret_cast<uintptr_t>(renderMesh->material));
          materialSetPtr          = m_resourceManager->addDescriptorSet(std::move(materialDescriptorSet), materialKey);
        }

        auto pipeline = createPipeline_(renderMesh->gpuMesh->vertexBuffer, instanceBufferPtr);

        std::string pipelineKey = "pipeline_"
                                + std::to_string(reinterpret_cast<uintptr_t>(renderMesh->gpuMesh->vertexBuffer)) + "_"
                                + std::to_string(reinterpret_cast<uintptr_t>(instanceBufferPtr));
        auto pipelinePtr = m_resourceManager->addPipeline(std::move(pipeline), pipelineKey);

        DrawData drawData;
        drawData.pipeline              = pipelinePtr;
        drawData.materialDescriptorSet = materialSetPtr;
        drawData.vertexBuffer          = renderMesh->gpuMesh->vertexBuffer;
        drawData.indexBuffer           = renderMesh->gpuMesh->indexBuffer;
        drawData.instanceBuffer        = instanceBufferPtr;
        drawData.indexCount            = renderMesh->gpuMesh->indexBuffer->getDesc().size / sizeof(uint32_t);
        drawData.instanceCount         = static_cast<uint32_t>(modelMatrices.size());

        m_drawDataList.push_back(drawData);
      }
    }
  }

  std::unique_ptr<rhi::Buffer> createInstanceBuffer_(const std::vector<math::Matrix4f<>>& matrices) {
    rhi::BufferDesc bufferDesc;
    bufferDesc.size        = matrices.size() * sizeof(math::Matrix4f<>);
    bufferDesc.createFlags = rhi::BufferCreateFlag::VertexBuffer;
    bufferDesc.type        = rhi::BufferType::Dynamic;

    auto buffer = m_device->createBuffer(bufferDesc);

    m_device->updateBuffer(buffer.get(), matrices.data(), bufferDesc.size);

    return buffer;
  }

  std::unique_ptr<rhi::DescriptorSet> createMaterialDescriptorSet_(Material* material) {
    if (!material) {
      return nullptr;
    }

    rhi::DescriptorSetLayoutDesc materialLayoutDesc;

    std::vector<std::string> textureNames = {"albedo", "normal_map", "roughness", "metalness"};
    uint32_t                 binding      = 0;

    for (const auto& textureName : textureNames) {
      rhi::DescriptorSetLayoutBindingDesc textureBindingDesc;
      textureBindingDesc.binding    = binding++;
      textureBindingDesc.type       = rhi::ShaderBindingType::TextureSrv;
      textureBindingDesc.stageFlags = rhi::ShaderStageFlag::Fragment;
      materialLayoutDesc.bindings.push_back(textureBindingDesc);
    }

    std::string layoutKey         = "material_layout";
    auto        materialSetLayout = m_resourceManager->getDescriptorSetLayout(layoutKey);

    if (!materialSetLayout) {
      auto newLayout    = m_device->createDescriptorSetLayout(materialLayoutDesc);
      materialSetLayout = m_resourceManager->addDescriptorSetLayout(std::move(newLayout), layoutKey);
    }

    auto descriptorSet = m_device->createDescriptorSet(materialSetLayout);

    binding = 0;
    for (const auto& textureName : textureNames) {
      auto it = material->textures.find(textureName);

      if (it != material->textures.end() && it->second) {
        auto texture = it->second;
        descriptorSet->setTexture(binding, texture);
      } else {
        GlobalLogger::Log(LogLevel::Warning,
                          "Material texture not found: " + textureName + " for material: " + material->materialName);
      }

      binding++;
    }

    return descriptorSet;
  }

  std::unique_ptr<rhi::GraphicsPipeline> createPipeline_(rhi::Buffer* vertexBuffer, rhi::Buffer* instanceBuffer) {
    rhi::GraphicsPipelineDesc pipelineDesc;

    pipelineDesc.shaders.push_back(m_vertexShader);
    pipelineDesc.shaders.push_back(m_pixelShader);

    rhi::VertexInputBindingDesc vertexBinding;
    vertexBinding.binding   = 0;
    vertexBinding.stride    = sizeof(Vertex);
    vertexBinding.inputRate = rhi::VertexInputRate::Vertex;
    pipelineDesc.vertexBindings.push_back(vertexBinding);

    rhi::VertexInputBindingDesc instanceBinding;
    instanceBinding.binding   = 1;
    instanceBinding.stride    = sizeof(math::Matrix4f<>);
    instanceBinding.inputRate = rhi::VertexInputRate::Instance;
    pipelineDesc.vertexBindings.push_back(instanceBinding);

    rhi::VertexInputAttributeDesc positionAttr;
    positionAttr.location     = 0;
    positionAttr.binding      = 0;
    positionAttr.format       = rhi::TextureFormat::Rgb32f;
    positionAttr.offset       = offsetof(Vertex, position);
    positionAttr.semanticName = "POSITION";
    pipelineDesc.vertexAttributes.push_back(positionAttr);

    rhi::VertexInputAttributeDesc normalAttr;
    normalAttr.location     = 1;
    normalAttr.binding      = 0;
    normalAttr.format       = rhi::TextureFormat::Rgb32f;
    normalAttr.offset       = offsetof(Vertex, normal);
    normalAttr.semanticName = "NORMAL";
    pipelineDesc.vertexAttributes.push_back(normalAttr);

    rhi::VertexInputAttributeDesc uvAttr;
    uvAttr.location     = 2;
    uvAttr.binding      = 0;
    uvAttr.format       = rhi::TextureFormat::Rg32f;
    uvAttr.offset       = offsetof(Vertex, texCoords);
    uvAttr.semanticName = "TEXCOORD";
    pipelineDesc.vertexAttributes.push_back(uvAttr);

    rhi::VertexInputAttributeDesc tangentAttr;
    tangentAttr.location     = 3;
    tangentAttr.binding      = 0;
    tangentAttr.format       = rhi::TextureFormat::Rgb32f;
    tangentAttr.offset       = offsetof(Vertex, tangent);
    tangentAttr.semanticName = "TANGENT";
    pipelineDesc.vertexAttributes.push_back(tangentAttr);

    rhi::VertexInputAttributeDesc bitangentAttr;
    bitangentAttr.location     = 4;
    bitangentAttr.binding      = 0;
    bitangentAttr.format       = rhi::TextureFormat::Rgb32f;
    bitangentAttr.offset       = offsetof(Vertex, bitangent);
    bitangentAttr.semanticName = "BITANGENT";
    pipelineDesc.vertexAttributes.push_back(bitangentAttr);

    rhi::VertexInputAttributeDesc colorAttr;
    colorAttr.location     = 5;
    colorAttr.binding      = 0;
    colorAttr.format       = rhi::TextureFormat::Rgba32f;
    colorAttr.offset       = offsetof(Vertex, color);
    colorAttr.semanticName = "COLOR";
    pipelineDesc.vertexAttributes.push_back(colorAttr);

    for (uint32_t i = 0; i < 4; i++) {
      rhi::VertexInputAttributeDesc matrixCol;
      matrixCol.location     = 6 + i;
      matrixCol.binding      = 1;
      matrixCol.format       = rhi::TextureFormat::Rgba32f;
      matrixCol.offset       = i * 16;
      matrixCol.semanticName = "INSTANCE";
      pipelineDesc.vertexAttributes.push_back(matrixCol);
    }

    pipelineDesc.inputAssembly.topology               = rhi::PrimitiveType::Triangles;
    pipelineDesc.inputAssembly.primitiveRestartEnable = false;

    pipelineDesc.rasterization.polygonMode     = rhi::PolygonMode::Fill;
    pipelineDesc.rasterization.cullMode        = rhi::CullMode::Back;
    pipelineDesc.rasterization.frontFace       = rhi::FrontFace::Ccw;
    pipelineDesc.rasterization.depthBiasEnable = false;
    pipelineDesc.rasterization.lineWidth       = 1.0f;

    pipelineDesc.depthStencil.depthTestEnable   = true;
    pipelineDesc.depthStencil.depthWriteEnable  = true;
    pipelineDesc.depthStencil.depthCompareOp    = rhi::CompareOp::Less;
    pipelineDesc.depthStencil.stencilTestEnable = false;

    rhi::ColorBlendAttachmentDesc blendAttachment;
    blendAttachment.blendEnable    = false;
    blendAttachment.colorWriteMask = rhi::ColorMask::All;
    pipelineDesc.colorBlend.attachments.push_back(blendAttachment);

    pipelineDesc.multisample.rasterizationSamples = rhi::MSAASamples::Count1;

    pipelineDesc.setLayouts.push_back(m_viewDescriptorSet->getLayout());
    pipelineDesc.setLayouts.push_back(m_directionalLightDescriptorSet->getLayout());
    pipelineDesc.setLayouts.push_back(m_pointLightDescriptorSet->getLayout());
    pipelineDesc.setLayouts.push_back(m_spotLightDescriptorSet->getLayout());
    auto materialDescriptorSetLayout = m_resourceManager->getDescriptorSetLayout("material_layout");
    pipelineDesc.setLayouts.push_back(materialDescriptorSetLayout);
    pipelineDesc.setLayouts.push_back(m_defaultSamplerDescriptorSet->getLayout());

    pipelineDesc.renderPass = m_renderPass;

    return m_device->createGraphicsPipeline(pipelineDesc);
  }

  rhi::Device*        m_device        = nullptr;
  rhi::ShaderManager* m_shaderManager = nullptr;
  RenderContext*      m_context       = nullptr;

  RenderResourceManager* m_resourceManager = nullptr;

  rhi::Shader* m_vertexShader = nullptr;
  rhi::Shader* m_pixelShader  = nullptr;

  rhi::Viewport    m_viewport;
  rhi::ScissorRect m_scissor;

  rhi::RenderPass*    m_renderPass                    = nullptr;
  rhi::Framebuffer*   m_framebuffer                   = nullptr;
  rhi::DescriptorSet* m_viewDescriptorSet             = nullptr;
  rhi::DescriptorSet* m_directionalLightDescriptorSet = nullptr;
  rhi::DescriptorSet* m_pointLightDescriptorSet       = nullptr;
  rhi::DescriptorSet* m_spotLightDescriptorSet        = nullptr;
  rhi::Sampler*       m_defaultSampler                = nullptr;
  rhi::DescriptorSet* m_defaultSamplerDescriptorSet   = nullptr;

  std::vector<DrawData> m_drawDataList;
};

class FinalPass : public RenderPass {
  public:
  FinalPass(rhi::Device* device)
      : m_device(device) {}

  ~FinalPass() override { finalize(); }

  void initialize(RenderContext& context) override {}

  void execute(RenderContext& context) override {
    auto commandBuffer = context.commandBuffer.get();

    if (!commandBuffer || !context.renderTarget) {
      return;
    }



    commandBuffer->copyTexture(context.renderTarget->colorBuffer.get(), context.renderTarget->backBuffer);


  }

  void finalize() override {}

  private:
  rhi::Device* m_device = nullptr;
};

class Renderer {
  public:
  Renderer() = default;

  ~Renderer() {
    waitForAllFrames_();

    m_basePass.reset();
    // m_debugPass.reset();
    m_finalPass.reset();
    // m_postProcessPass.reset();

    m_shaderManager.reset();
    m_swapChain.reset();
    m_device.reset();
  }

  bool initialize(Window* window, rhi::RenderingApi api) {
    m_window = window;

    rhi::DeviceDesc deviceDesc;
    deviceDesc.window = window;
    m_device          = g_createDevice(api, deviceDesc);

    if (!m_device) {
      GlobalLogger::Log(LogLevel::Error, "Failed to create device");
      return false;
    }

    rhi::SwapchainDesc swapchainDesc;
    swapchainDesc.width       = window->getSize().width();
    swapchainDesc.height      = window->getSize().height();
    swapchainDesc.format      = rhi::TextureFormat::Bgra8;
    swapchainDesc.bufferCount = MAX_FRAMES_IN_FLIGHT;

    m_swapChain = m_device->createSwapChain(swapchainDesc);
    if (!m_swapChain) {
      GlobalLogger::Log(LogLevel::Error, "Failed to create swap chain");
      return false;
    }

    m_shaderManager = std::make_unique<rhi::ShaderManager>(m_device.get(), true);

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
      rhi::FenceDesc fenceDesc;
      fenceDesc.signaled = true;
      m_frameFences[i]   = m_device->createFence(fenceDesc);

      m_imageAvailableSemaphores[i] = m_device->createSemaphore();
      m_renderFinishedSemaphores[i] = m_device->createSemaphore();

      m_commandBufferPools[i].clear();
      m_commandBufferPools[i].reserve(COMMAND_BUFFERS_PER_FRAME);

      for (uint32_t j = 0; j < INITIAL_COMMAND_BUFFERS; ++j) {
        rhi::CommandBufferDesc cmdDesc;
        auto                   cmdBuffer = m_device->createCommandBuffer(cmdDesc);
        if (cmdBuffer) {
          m_commandBufferPools[i].emplace_back(std::move(cmdBuffer));
        }
      }
    }

    setupRenderStages_();

    m_initialized = true;

    GlobalLogger::Log(LogLevel::Info, "Renderer initialized successfully");
    return true;
  }

  RenderContext beginFrame(Scene* scene, const RenderSettings& renderSettings) {
    GlobalLogger::Log(LogLevel::Info, "New Frame");

    if (!m_initialized) {
      GlobalLogger::Log(LogLevel::Error, "Renderer not initialized");
      return RenderContext();
    }

    m_currentFrame                = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    auto& fence                   = m_frameFences[m_currentFrame];
    auto& imageAvailableSemaphore = m_imageAvailableSemaphores[m_currentFrame];
    auto& renderFinishedSemaphore = m_renderFinishedSemaphores[m_currentFrame];

    fence->wait();
    fence->reset();

    if (!m_swapChain->acquireNextImage()) {
      GlobalLogger::Log(LogLevel::Error, "Failed to acquire next swapchain image");
      return RenderContext();
    }

    auto commandBuffer = acquireCommandBuffer_();

    commandBuffer->begin();

    math::Dimension2Di viewportDimension;
    switch (renderSettings.appMode) {
      case ApplicationRenderMode::Game:
        viewportDimension = m_window->getSize();
        break;
      case ApplicationRenderMode::Editor:
        viewportDimension = renderSettings.renderViewportDimension;
        break;
      default:
        GlobalLogger::Log(LogLevel::Error, "Invalid application mode");
        return RenderContext();
    }

    auto swapchainImage = m_swapChain->getCurrentImage();
    auto renderTarget   = createRenderTarget_(viewportDimension, swapchainImage);

    RenderContext context;
    context.scene             = scene;
    context.commandBuffer     = std::move(commandBuffer);
    context.renderTarget      = std::move(renderTarget);
    context.viewportDimension = viewportDimension;
    context.renderSettings    = renderSettings;
    context.device            = m_device.get();
    context.frameIndex        = m_frameIndex;
    context.waitSemaphore     = imageAvailableSemaphore.get();
    context.signalSemaphore   = renderFinishedSemaphore.get();

    return context;
  }

  void renderFrame(RenderContext& context) {
    if (!context.commandBuffer || !context.renderTarget) {
      GlobalLogger::Log(LogLevel::Error, "Invalid render context");
      return;
    }
    if (m_basePass) {
      m_basePass->initialize(context);
    }

    // TODO: currently not using it - first check whether base pass works
    // if (m_debugPass) {
    //  m_debugPass->initialize(context);
    //}

    // if (m_postProcessPass && context.renderSettings.postProcessMode != PostProcessMode::None) {
    //   m_postProcessPass->initialize(context);
    // }

    if (m_finalPass) {
      m_finalPass->initialize(context);
    }

    bool exclusiveMode = false;

    // if (context.renderSettings.renderMode != RenderMode::Solid && m_debugPass) {
    //   exclusiveMode = m_debugPass->isExclusive();
    //   m_debugPass->execute(context);
    // }

    if (!exclusiveMode && m_basePass) {
      m_basePass->execute(context);
    }

    //
    // if (context.renderSettings.postProcessMode != PostProcessMode::None && m_postProcessPass) {
    //  m_postProcessPass->execute(context);
    //}

    if (m_finalPass && context.renderSettings.appMode == ApplicationRenderMode::Game) {
      m_finalPass->execute(context);
    }

    if (m_basePass) {
      m_basePass->finalize();
    }

    // if (m_debugPass) {
    //   m_debugPass->finalize();
    // }

    // if (m_postProcessPass && context.renderSettings.postProcessMode != PostProcessMode::None) {
    //   m_postProcessPass->finalize();
    // }

    if (m_finalPass) {
      m_finalPass->finalize();
    }
  }

  void endFrame(RenderContext& context) {
    context.commandBuffer->end();

    std::vector<rhi::Semaphore*> waitSemaphores;
    if (context.waitSemaphore) {
      waitSemaphores.push_back(context.waitSemaphore);
    }

    std::vector<rhi::Semaphore*> signalSemaphores;
    if (context.signalSemaphore) {
      signalSemaphores.push_back(context.signalSemaphore);
    }

    m_device->submitCommandBuffer(
        context.commandBuffer.get(), m_frameFences[m_currentFrame].get(), waitSemaphores, signalSemaphores);

    m_device->present(m_swapChain.get(), context.signalSemaphore);

    recycleCommandBuffer_(std::move(context.commandBuffer));

    m_frameIndex++;
  }

  bool onWindowResize(uint32_t width, uint32_t height) {
    waitForAllFrames_();

    if (m_swapChain) {
      if (!m_swapChain->resize(width, height)) {
        GlobalLogger::Log(LogLevel::Error, "Failed to resize swap chain");
        return false;
      }
    }

    return true;
  }

  rhi::Device* getDevice() const { return m_device.get(); }

  uint32_t getFrameIndex() const { return m_frameIndex; }

  rhi::ShaderManager* getShaderManager() const { return m_shaderManager.get(); }

  private:
  static constexpr uint32_t MAX_FRAMES_IN_FLIGHT      = 2;
  static constexpr uint32_t COMMAND_BUFFERS_PER_FRAME = 8;
  static constexpr uint32_t INITIAL_COMMAND_BUFFERS   = 2;

  std::unique_ptr<rhi::CommandBuffer> acquireCommandBuffer_() {
    auto& pool = m_commandBufferPools[m_currentFrame];

    if (!pool.empty()) {
      auto cmdBuffer = std::move(pool.back());
      pool.pop_back();

      cmdBuffer->reset();

      return cmdBuffer;
    }

    rhi::CommandBufferDesc cmdDesc;
    return m_device->createCommandBuffer(cmdDesc);
  }

  void recycleCommandBuffer_(std::unique_ptr<rhi::CommandBuffer> cmdBuffer) {
    if (cmdBuffer) {
      auto& pool = m_commandBufferPools[m_currentFrame];

      if (pool.size() < COMMAND_BUFFERS_PER_FRAME) {
        pool.push_back(std::move(cmdBuffer));
      }
    }
  }

  void waitForAllFrames_() {
    if (m_device) {
      m_device->waitIdle();

      // for (auto& fence : m_frameFences) {
      //   if (fence) {
      //     fence->reset();
      //   }
      // }
    }
  }

  std::unique_ptr<SceneRenderTarget> createRenderTarget_(const math::Dimension2Di& viewportDimension,
                                                         rhi::Texture*             swapchainImage) {
    auto result = std::make_unique<SceneRenderTarget>();

    rhi::TextureDesc colorDesc;
    colorDesc.width  = viewportDimension.width();
    colorDesc.height = viewportDimension.height();
    colorDesc.format = swapchainImage->getFormat();
    colorDesc.createFlags
        = rhi::TextureCreateFlag::Rtv | rhi::TextureCreateFlag::TransferSrc | rhi::TextureCreateFlag::TransferDst;

    result->colorBuffer = m_device->createTexture(colorDesc);

    rhi::TextureDesc depthDesc;
    depthDesc.width       = viewportDimension.width();
    depthDesc.height      = viewportDimension.height();
    depthDesc.format      = rhi::TextureFormat::D24S8;
    depthDesc.createFlags = rhi::TextureCreateFlag::Dsv;

    result->depthBuffer = m_device->createTexture(depthDesc);

    result->backBuffer = swapchainImage;

    return result;
  }

  void setupRenderStages_() {
    m_basePass = std::make_unique<BasePass>(m_device.get(), m_shaderManager.get());

    // m_debugPass = std::make_unique<DebugPass>(m_device.get(), m_shaderManager.get());

    m_finalPass = std::make_unique<FinalPass>(m_device.get());
  }

  Window*                             m_window = nullptr;
  std::unique_ptr<rhi::Device>        m_device;
  std::unique_ptr<rhi::SwapChain>     m_swapChain;
  std::unique_ptr<rhi::ShaderManager> m_shaderManager;

  std::unique_ptr<BasePass>  m_basePass;
  // std::unique_ptr<DebugPass>  m_debugPass;
  // std::unique_ptr<RenderPass> m_postProcessPass;
  std::unique_ptr<FinalPass> m_finalPass;

  // Command buffer pools - one pool per frame in flight
  std::array<std::vector<std::unique_ptr<rhi::CommandBuffer>>, MAX_FRAMES_IN_FLIGHT> m_commandBufferPools;

  // Synchronization primitives - one set per frame in flight
  std::array<std::unique_ptr<rhi::Fence>, MAX_FRAMES_IN_FLIGHT>     m_frameFences;
  std::array<std::unique_ptr<rhi::Semaphore>, MAX_FRAMES_IN_FLIGHT> m_imageAvailableSemaphores;
  std::array<std::unique_ptr<rhi::Semaphore>, MAX_FRAMES_IN_FLIGHT> m_renderFinishedSemaphores;

  uint32_t m_currentFrame = 0;  // Which frame in the pipeline we're working on
  uint32_t m_frameIndex   = 0;  // Total frames rendered
  bool     m_initialized  = false;
};

}  // namespace renderer
}  // namespace gfx
}  // namespace game_engine

#endif  // GAME_ENGINE_RENDERER_H
