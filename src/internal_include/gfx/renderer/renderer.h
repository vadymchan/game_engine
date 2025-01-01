#ifndef GAME_ENGINE_RENDERER_H
#define GAME_ENGINE_RENDERER_H

// TODO:
// - implement abstraction layer for rhi (rendering api)
// - add debug pass for rendering (bounding boxes, light sources, normal vectors
// etc.)
// - shadow pass for the future

#include "ecs/components/camera.h"
#include "ecs/components/light.h"
#include "ecs/components/render_model.h"
#include "ecs/components/transform.h"
#include "gfx/rhi/rhi.h"
#include "scene/scene.h"
#include "utils/hot_reload/hot_reload_manager.h"
#include "utils/path_manager/path_manager.h"
#include "utils/service/service_locator.h"

namespace game_engine {

struct RenderContext {
  std::shared_ptr<Scene>              scene;
  // initialized in Renderer
  std::shared_ptr<RenderFrameContext> renderFrameContext;
  // TODO: for now I use RenderFrameContext but will change soon
  // std::shared_ptr<CommandBuffer> commandBuffer = nullptr;
  // RenderTarget* renderTarget = nullptr;
  int                                 viewportWidth  = 0;
  int                                 viewportHeight = 0;
};

// This is RenderPass (Currently named so since it conflicts with RenderPass in
// RHI)
class RenderStage {
  public:
  virtual ~RenderStage() = default;
  virtual void initialize(const std::shared_ptr<RenderContext>& renderContext)
      = 0;
  virtual void execute()  = 0;
  virtual void finalize() = 0;
};

class BasePass : public RenderStage {
  public:
  BasePass() = default;

  void initialize(
      const std::shared_ptr<RenderContext>& renderContext) override {
    m_renderContext_ = renderContext;
    setupPipelineStates();
    setupRenderPass();
    preparePerFrameResources();
    prepareDrawCalls();
  }

  void execute() override {
    // Transition Resource Layouts
    g_rhi->transitionLayout(
        m_renderContext_->renderFrameContext->getActiveCommandBuffer(),
        m_renderContext_->renderFrameContext->m_sceneRenderTargetPtr_
            ->m_finalColorPtr_->getTexture(),
        EResourceLayout::COLOR_ATTACHMENT);

    auto newLayout
        = m_renderContext_->renderFrameContext->m_sceneRenderTargetPtr_
                  ->m_depthPtr_->getTexture()
                  ->isDepthOnlyFormat()
            ? EResourceLayout::DEPTH_ATTACHMENT
            : EResourceLayout::DEPTH_STENCIL_ATTACHMENT;

    g_rhi->transitionLayout(
        m_renderContext_->renderFrameContext->getActiveCommandBuffer(),
        m_renderContext_->renderFrameContext->m_sceneRenderTargetPtr_
            ->m_depthPtr_->getTexture(),
        newLayout);

    if (renderPass_->beginRenderPass(
            m_renderContext_->renderFrameContext->getActiveCommandBuffer())) {
      for (const auto& drawData : drawDataList_) {
        drawData.pipelineStateInfo_->bind(m_renderContext_->renderFrameContext);

        ShaderBindingInstanceArray shaderBindingInstanceArray;
        shaderBindingInstanceArray.add(viewShaderBindingInstance_.get());

        if (directionalLightShaderBindingInstance_) {
          shaderBindingInstanceArray.add(
              directionalLightShaderBindingInstance_.get());
        }
        if (pointLightShaderBindingInstance_) {
          shaderBindingInstanceArray.add(
              pointLightShaderBindingInstance_.get());
        }
        if (spotLightShaderBindingInstance_) {
          shaderBindingInstanceArray.add(spotLightShaderBindingInstance_.get());
        }

        if (drawData.materialShaderBindingInstance_) {
          shaderBindingInstanceArray.add(
              drawData.materialShaderBindingInstance_.get());
        }

        ShaderBindingInstanceCombiner shaderBindingInstanceCombiner;
        for (int32_t i = 0; i < shaderBindingInstanceArray.m_numOfData_; ++i) {
          // shaderBindingLayoutArray.add(
          //    shaderBindingInstanceArray[i]->m_shaderBindingsLayouts_);

          // Add shaderBindingInstanceCombiner data : DescriptorSets,
          // DynamicOffsets
          shaderBindingInstanceCombiner.m_descriptorSetHandles_.add(
              shaderBindingInstanceArray[i]->getHandle());
          const std::vector<uint32_t>* pDynamicOffsetTest
              = shaderBindingInstanceArray[i]->getDynamicOffsets();
          if (pDynamicOffsetTest && pDynamicOffsetTest->size()) {
            shaderBindingInstanceCombiner.m_dynamicOffsets_.add(
                (void*)pDynamicOffsetTest->data(),
                (int32_t)pDynamicOffsetTest->size());
          }
        }
        shaderBindingInstanceCombiner.m_shaderBindingInstanceArray
            = &shaderBindingInstanceArray;

        // TODO: move upper inside loop
        // shaderBindingInstanceCombiner.m_descriptorSetHandles_.add(
        //     viewShaderBindingInstance_->getHandle());
        // if (drawData.materialShaderBindingInstance_) {
        //   // shaderBindingInstanceCombiner.m_descriptorSetHandles_.add(
        //   //     drawData.materialShaderBindingInstance_->getHandle());
        // }

        g_rhi->bindGraphicsShaderBindingInstances(
            m_renderContext_->renderFrameContext->getActiveCommandBuffer(),
            drawData.pipelineStateInfo_,
            shaderBindingInstanceCombiner,
            0);

        const auto& gpuMesh = drawData.renderGeometryMesh_;
        gpuMesh->vertexBuffer->bind(m_renderContext_->renderFrameContext);
        drawData.instanceBuffer_->bind(m_renderContext_->renderFrameContext);

        gpuMesh->indexBuffer->bind(m_renderContext_->renderFrameContext);

        g_rhi->drawElementsInstanced(m_renderContext_->renderFrameContext,
                                     0,
                                     0,
                                     gpuMesh->indexBuffer->getElementCount(),
                                     drawData.instanceCount);
      }

      renderPass_->endRenderPass();
    }
  }

  void finalize() override {
    // TODO:
    // PipelineStateInfo*                     pipelineStateInfo_
    // all ShaderBindingInstance
    // RenderPass* renderPass_
    // IUniformBufferBlock (add correct ~ in vulkan)
    // shaders_.m_vertexShader_ delete

    // delete renderPass_;
    drawDataList_.clear();
  }

  private:
  void setupPipelineStates() {
    auto rasterizationState
        = TRasterizationStateInfo<EPolygonMode::LINE,
                                  ECullMode::BACK,
                                  EFrontFace::CCW,
                                  false,
                                  0.0f,
                                  0.0f,
                                  0.0f,
                                  1.0f,
                                  false,
                                  false,
                                  // TODO: currently not support multisampling
                                  // but consider in the future
                                  EMSAASamples::COUNT_1,
                                  true,
                                  0.2f,
                                  false,
                                  false>::s_create();

    auto depthStencilState = TDepthStencilStateInfo<true,
                                                    true,
                                                    ECompareOp::LESS,
                                                    false,
                                                    false,
                                                    0.0f,
                                                    1.0f>::s_create();

    auto blendingState = TBlendingStateInfo<false,
                                            EBlendFactor::ONE,
                                            EBlendFactor::ZERO,
                                            EBlendOp::ADD,
                                            EBlendFactor::ONE,
                                            EBlendFactor::ZERO,
                                            EBlendOp::ADD,
                                            EColorMask::ALL>::s_create();

    pipelineStateFixed_ = PipelineStateFixedInfo(
        rasterizationState,
        depthStencilState,
        blendingState,
        Viewport(0.0f,
                 0.0f,
                 static_cast<float>(m_renderContext_->viewportWidth),
                 static_cast<float>(m_renderContext_->viewportHeight)),
        Scissor(0,
                0,
                m_renderContext_->viewportWidth,
                m_renderContext_->viewportHeight),
        false /*gOptions.UseVRS*/);

    static bool isShaderInitialized = false;

    // TODO: use config in future
    if (!isShaderInitialized) {
      ShaderInfo vertexShaderInfo;
      vertexShaderInfo.setName(NameStatic("BasePassVS"));
      vertexShaderInfo.setShaderFilepath(
          NameStatic("assets/shaders/base_pass/shader_instancing.vs.hlsl"));
      vertexShaderInfo.setShaderType(EShaderAccessStageFlag::VERTEX);
      shaders_.m_vertexShader_ = g_rhi->createShader(vertexShaderInfo);

      ShaderInfo pixelShaderInfo;
      pixelShaderInfo.setName(NameStatic("BasePassPS"));
      pixelShaderInfo.setShaderFilepath(
          NameStatic("assets/shaders/base_pass/shader.ps.hlsl"));
      pixelShaderInfo.setShaderType(EShaderAccessStageFlag::FRAGMENT);
      shaders_.m_pixelShader_ = g_rhi->createShader(pixelShaderInfo);

      // hot reload for shaders
      {
        auto fileModificationHandler = [&shaders
                                        = shaders_](const wtr::event& e) {
          if (e.path_name
              == "assets/shaders/base_pass/shader_instancing.vs.hlsl") {
            ShaderInfo vertexShaderInfo;
            vertexShaderInfo.setName(NameStatic("BasePassVS"));
            vertexShaderInfo.setShaderFilepath(
                NameStatic(e.path_name.string()));
            vertexShaderInfo.setShaderType(EShaderAccessStageFlag::VERTEX);
            delete shaders.m_vertexShader_;
            shaders.m_vertexShader_ = g_rhi->createShader(vertexShaderInfo);
          }

          if (e.path_name == "assets/shaders/base_pass/shader.ps.hlsl") {
            ShaderInfo pixelShaderInfo;
            pixelShaderInfo.setName(NameStatic("BasePassPS"));
            pixelShaderInfo.setShaderFilepath(NameStatic(e.path_name.string()));
            pixelShaderInfo.setShaderType(EShaderAccessStageFlag::FRAGMENT);
            delete shaders.m_pixelShader_;
            shaders.m_pixelShader_ = g_rhi->createShader(pixelShaderInfo);
          }
        };

        auto hotReloadManager = ServiceLocator::s_get<HotReloadManager>();
        hotReloadManager->watchFileModifications(
            PathManager::s_getShaderPath() / "base_pass",
            fileModificationHandler);
      }
      isShaderInitialized = true;
    }
  }

  void setupRenderPass() {
    const RtClearValue clearColor = RtClearValue(0.0f, 0.0f, 0.0f, 1.0f);
    const RtClearValue clearDepth = RtClearValue(1.0f, 0);

    Attachment colorAttachment(m_renderContext_->renderFrameContext
                                   ->m_sceneRenderTargetPtr_->m_finalColorPtr_,
                               EAttachmentLoadStoreOp::CLEAR_STORE,
                               EAttachmentLoadStoreOp::DONTCARE_DONTCARE,
                               clearColor,
                               EResourceLayout::UNDEFINED,
                               EResourceLayout::COLOR_ATTACHMENT);

    Attachment depthAttachment(m_renderContext_->renderFrameContext
                                   ->m_sceneRenderTargetPtr_->m_depthPtr_,
                               EAttachmentLoadStoreOp::CLEAR_STORE,
                               EAttachmentLoadStoreOp::DONTCARE_DONTCARE,
                               clearDepth,
                               EResourceLayout::UNDEFINED,
                               EResourceLayout::DEPTH_STENCIL_ATTACHMENT);

    RenderPassInfo renderPassInfo;
    renderPassInfo.m_attachments_.push_back(colorAttachment);
    renderPassInfo.m_attachments_.push_back(depthAttachment);

    Subpass subpass;
    subpass.initialize(0, /*sourceSubpassIndex*/
                       1, /*destSubpassIndex*/
                       EPipelineStageMask::COLOR_ATTACHMENT_OUTPUT_BIT,
                       EPipelineStageMask::FRAGMENT_SHADER_BIT);

    subpass.m_outputColorAttachments_.push_back(0);
    subpass.m_outputDepthAttachment_ = 1;

    renderPassInfo.m_subpasses_.push_back(subpass);

    // Создание Render Pass
    renderPass_ = g_rhi->getOrCreateRenderPass(
        renderPassInfo,
        {0, 0},
        {m_renderContext_->viewportWidth, m_renderContext_->viewportHeight});
  }

  void preparePerFrameResources() {
    // View
    {
      auto view = m_renderContext_->scene->getEntityRegistry()
                      .view<Transform, Camera, CameraMatrices>();

      if (view.begin() == view.end()) {
        // TODO: use loagger
        std::cout << "No main Camera exists!\n";
      }

      // TODO: in future consider using tags instead of taking forst entity
      entt::entity firstEntity = view.front();

      auto& transform    = view.get<Transform>(firstEntity);
      auto& cameraMatrix = view.get<CameraMatrices>(firstEntity);

      struct ViewUniformBuffer {
        math::Matrix4f<> view;
        math::Matrix4f<> projection;
        math::Matrix4f<> viewProjection;
        math::Vector3Df  eyeWorld;
        float            padding0;
      };

      ViewUniformBuffer ubo;
      ubo.view            = cameraMatrix.view;
      ubo.projection      = cameraMatrix.projection;
      ubo.viewProjection  = cameraMatrix.view;
      ubo.viewProjection *= cameraMatrix.projection;
      ubo.eyeWorld        = transform.translation;

      viewUniformBuffer_ = std::shared_ptr<IUniformBufferBlock>(
          g_rhi->createUniformBufferBlock(Name("ViewUniformParameters"),
                                          LifeTimeType::MultiFrame,
                                          sizeof(ubo)));
      viewUniformBuffer_->updateBufferData(&ubo, sizeof(ubo));

      ShaderBindingArray                   shaderBindingArray;
      ShaderBindingResourceInlineAllocator resourceAllocator;

      shaderBindingArray.add(
          ShaderBinding(0,
                        1,
                        EShaderBindingType::UNIFORMBUFFER,
                        false,
                        EShaderAccessStageFlag::ALL_GRAPHICS,
                        resourceAllocator.alloc<UniformBufferResource>(
                            viewUniformBuffer_.get())));

      viewShaderBindingInstance_ = g_rhi->createShaderBindingInstance(
          shaderBindingArray, ShaderBindingInstanceType::MultiFrame);
    }

    // Directional Light
    {
      auto dirLightView = m_renderContext_->scene->getEntityRegistry()
                              .view<Light, DirectionalLight>();
      if (dirLightView.begin() != dirLightView.end()) {
        auto  entity   = dirLightView.front();
        auto& light    = dirLightView.get<Light>(entity);
        auto& dirLight = dirLightView.get<DirectionalLight>(entity);

        struct DirectionalLightData {
          float color[3];
          float intensity;
          float direction[3];
          float padding;
        };

        DirectionalLightData dld;
        dld.color[0]  = light.color.x();
        dld.color[1]  = light.color.y();
        dld.color[2]  = light.color.z();
        dld.intensity = light.intensity;
        // dld.direction = dirLight.direction.normalized();
        //  TODO: remove the line below (debug only)
        dld.direction[0] = dirLight.direction.x();
        dld.direction[1] = dirLight.direction.y();
        dld.direction[2] = dirLight.direction.z();
        dld.padding      = 0.0f;

        directionalLightUniformBuffer_ = std::shared_ptr<IUniformBufferBlock>(
            g_rhi->createUniformBufferBlock(Name("DirectionalLightBuffer"),
                                            LifeTimeType::MultiFrame,
                                            sizeof(dld)));
        directionalLightUniformBuffer_->updateBufferData(&dld, sizeof(dld));

        ShaderBindingArray                   directionalLightSBA;
        ShaderBindingResourceInlineAllocator dirLightResAlloc;
        directionalLightSBA.add(
            ShaderBinding(0,
                          1,
                          EShaderBindingType::UNIFORMBUFFER,
                          false,
                          EShaderAccessStageFlag::ALL_GRAPHICS,
                          dirLightResAlloc.alloc<UniformBufferResource>(
                              directionalLightUniformBuffer_.get())));

        directionalLightShaderBindingInstance_
            = g_rhi->createShaderBindingInstance(
                directionalLightSBA, ShaderBindingInstanceType::MultiFrame);
      } else {
        directionalLightShaderBindingInstance_ = nullptr;
      }
    }

    // Point Light
    {
      auto pointLightView = m_renderContext_->scene->getEntityRegistry()
                                .view<Light, PointLight, Transform>();
      if (pointLightView.begin() != pointLightView.end()) {
        auto  entity     = pointLightView.front();
        auto& light      = pointLightView.get<Light>(entity);
        auto& pointLight = pointLightView.get<PointLight>(entity);
        auto& transform  = pointLightView.get<Transform>(entity);

        struct PointLightData {
          float color[3];
          float intensity;
          float range;
          float position[3];
        };

        PointLightData pld;
        pld.color[0]    = light.color.x();
        pld.color[1]    = light.color.y();
        pld.color[2]    = light.color.z();
        pld.intensity   = light.intensity;
        pld.range       = pointLight.range;
        pld.position[0] = transform.translation.x();
        pld.position[1] = transform.translation.y();
        pld.position[2] = transform.translation.z();

        pointLightUniformBuffer_ = std::shared_ptr<IUniformBufferBlock>(
            g_rhi->createUniformBufferBlock(Name("PointLightBuffer"),
                                            LifeTimeType::MultiFrame,
                                            sizeof(pld)));
        pointLightUniformBuffer_->updateBufferData(&pld, sizeof(pld));

        ShaderBindingArray                   pointLightSBA;
        ShaderBindingResourceInlineAllocator pointLightResAlloc;
        pointLightSBA.add(
            ShaderBinding(0,
                          1,
                          EShaderBindingType::UNIFORMBUFFER,
                          false,
                          EShaderAccessStageFlag::ALL_GRAPHICS,
                          pointLightResAlloc.alloc<UniformBufferResource>(
                              pointLightUniformBuffer_.get())));

        pointLightShaderBindingInstance_ = g_rhi->createShaderBindingInstance(
            pointLightSBA, ShaderBindingInstanceType::MultiFrame);
      } else {
        pointLightShaderBindingInstance_ = nullptr;
      }
    }

    // Spot Light
    {
      auto spotLightView = m_renderContext_->scene->getEntityRegistry()
                               .view<Light, SpotLight, Transform>();
      if (spotLightView.begin() != spotLightView.end()) {
        auto  entity    = spotLightView.front();
        auto& light     = spotLightView.get<Light>(entity);
        auto& spotLight = spotLightView.get<SpotLight>(entity);
        auto& transform = spotLightView.get<Transform>(entity);

        struct SpotLightData {
          float color[3];
          float intensity;
          float range;
          float innerConeAngle;
          float outerConeAngle;
          float position[3];
          float direction[3];
          float padding[3];
        };

        SpotLightData sld;
        sld.color[0]       = light.color.x();
        sld.color[1]       = light.color.y();
        sld.color[2]       = light.color.z();
        sld.intensity      = light.intensity;
        sld.range          = spotLight.range;
        sld.innerConeAngle = spotLight.innerConeAngle;
        sld.outerConeAngle = spotLight.outerConeAngle;
        sld.position[0]    = transform.translation.x();
        sld.position[1]    = transform.translation.y();
        sld.position[2]    = transform.translation.z();

        // Compute direction from transform rotation (assuming forward is Z)
        auto pitch = math::g_degreeToRadian(transform.rotation.x());
        auto yaw   = math::g_degreeToRadian(transform.rotation.y());
        auto roll  = math::g_degreeToRadian(transform.rotation.z());
        auto q     = math::Quaternionf::fromEulerAngles(
            roll, pitch, yaw, math::EulerRotationOrder::ZXY);
        // sld.direction = q.rotateVector(math::g_forwardVector<float, 3>());
        // TODO: remove that line (debut only)
        sld.direction[0] = transform.rotation.x();
        sld.direction[1] = transform.rotation.y();
        sld.direction[2] = transform.rotation.z();
        sld.padding[0]   = 0.0f;
        sld.padding[1]   = 0.0f;
        sld.padding[2]   = 0.0f;

        spotLightUniformBuffer_ = std::shared_ptr<IUniformBufferBlock>(
            g_rhi->createUniformBufferBlock(Name("SpotLightBuffer"),
                                            LifeTimeType::MultiFrame,
                                            sizeof(sld)));
        spotLightUniformBuffer_->updateBufferData(&sld, sizeof(sld));

        ShaderBindingArray                   spotLightSBA;
        ShaderBindingResourceInlineAllocator spotLightResAlloc;
        spotLightSBA.add(
            ShaderBinding(0,
                          1,
                          EShaderBindingType::UNIFORMBUFFER,
                          false,
                          EShaderAccessStageFlag::ALL_GRAPHICS,
                          spotLightResAlloc.alloc<UniformBufferResource>(
                              spotLightUniformBuffer_.get())));

        spotLightShaderBindingInstance_ = g_rhi->createShaderBindingInstance(
            spotLightSBA, ShaderBindingInstanceType::MultiFrame);
      } else {
        spotLightShaderBindingInstance_ = nullptr;
      }
    }
  }

  void prepareDrawCalls() {
    auto& registry = m_renderContext_->scene->getEntityRegistry();
    auto  view     = registry.view<Transform, std::shared_ptr<RenderModel>>();

    std::unordered_map<std::shared_ptr<RenderModel>,
                       std::vector<math::Matrix4f<>>>
        modelInstances;

    for (auto entity : view) {
      auto& transform   = view.get<Transform>(entity);
      auto& renderModel = view.get<std::shared_ptr<RenderModel>>(entity);

      math::Matrix4f modelMatrix = calculateTransformMatrix(transform);

      modelInstances[renderModel].push_back(modelMatrix);
    }

    for (const auto& pair : modelInstances) {
      const std::shared_ptr<RenderModel>&  renderModel   = pair.first;
      const std::vector<math::Matrix4f<>>& modelMatrices = pair.second;

      std::shared_ptr<VertexBuffer> instanceBuffer;
      createInstanceBuffer(modelMatrices, instanceBuffer);

      for (const auto& renderMesh : renderModel->renderMeshes) {
        DrawData drawData;
        drawData.renderGeometryMesh_ = renderMesh->gpuMesh;
        drawData.material_           = renderMesh->material;
        drawData.instanceBuffer_     = instanceBuffer;
        drawData.instanceCount = static_cast<uint32_t>(modelMatrices.size());

        ShaderBindingInstanceArray shaderBindingInstanceArray;
        shaderBindingInstanceArray.add(viewShaderBindingInstance_.get());

        if (directionalLightShaderBindingInstance_) {
          shaderBindingInstanceArray.add(
              directionalLightShaderBindingInstance_.get());
        }

        if (pointLightShaderBindingInstance_) {
          shaderBindingInstanceArray.add(
              pointLightShaderBindingInstance_.get());
        }

        if (spotLightShaderBindingInstance_) {
          shaderBindingInstanceArray.add(spotLightShaderBindingInstance_.get());
        }

        if (drawData.material_) {
          drawData.materialShaderBindingInstance_
              = createMaterialShaderBindingInstance(drawData.material_);
          shaderBindingInstanceArray.add(
              drawData.materialShaderBindingInstance_.get());
        }

        ShaderBindingLayoutArray shaderBindingLayoutArray;
        for (int32_t i = 0; i < shaderBindingInstanceArray.m_numOfData_; ++i) {
          shaderBindingLayoutArray.add(
              shaderBindingInstanceArray[i]->m_shaderBindingsLayouts_);
        }

        VertexBufferArray vertexBufferArray;
        vertexBufferArray.add(
            drawData.renderGeometryMesh_->vertexBuffer.get());  // Binding 0
        vertexBufferArray.add(drawData.instanceBuffer_.get());  // Binding 1

        // TODO: investigate that (currently seems unoptimized to create
        // Pipeline state per each drawCall (lots of state switching))
        drawData.pipelineStateInfo_
            = g_rhi->createPipelineStateInfo(&pipelineStateFixed_,
                                             shaders_,
                                             vertexBufferArray,
                                             renderPass_,
                                             shaderBindingLayoutArray,
                                             nullptr,
                                             0 /*subpassIndex*/);

        drawDataList_.push_back(std::move(drawData));
      }
    }
  }

  void createInstanceBuffer(const std::vector<math::Matrix4f<>>& modelMatrices,
                            std::shared_ptr<VertexBuffer>& instanceBuffer) {
    constexpr size_t kMatrixSize = 16;

    std::vector<float> instanceData;

    instanceData.reserve(modelMatrices.size() * kMatrixSize);
    for (const auto& matrix : modelMatrices) {
      instanceData.insert(
          instanceData.end(), matrix.data(), matrix.data() + kMatrixSize);
    }

    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("INSTANCE"),
        EBufferType::Static,
        math::Matrix4f<>::GetDataSize(),
        std::vector<IBufferAttribute::Attribute>{
          // IBufferAttribute::Attribute(
          //    EBufferElementType::FLOAT, 0, math::Matrix4f<>::GetDataSize())
          // Before
          IBufferAttribute::Attribute(EBufferElementType::FLOAT,
                                      0,
                                      sizeof(float) * 4),  // Column 0
          IBufferAttribute::Attribute(EBufferElementType::FLOAT,
                                      sizeof(float) * 4,
                                      sizeof(float) * 4),  // Column 1
          IBufferAttribute::Attribute(EBufferElementType::FLOAT,
                                      sizeof(float) * 8,
                                      sizeof(float) * 4),  // Column 2
          IBufferAttribute::Attribute(EBufferElementType::FLOAT,
                                      sizeof(float) * 12,
                                      sizeof(float) * 4)   // Column 3
        },
        std::move(instanceData));

    auto instanceStreamData = std::make_shared<VertexStreamData>();
    instanceStreamData->m_streams_.push_back(streamParam);

    instanceStreamData->m_primitiveType_ = EPrimitiveType::TRIANGLES;
    instanceStreamData->m_elementCount_
        = static_cast<int32_t>(modelMatrices.size());
    // instanceStreamData->m_startLocation_
    //     = 6;  // TODO: fix this (take the input slot)
    instanceStreamData->m_bindingIndex_ = 6;

    instanceStreamData->m_vertexInputRate_ = EVertexInputRate::INSTANCE;

    instanceBuffer = g_rhi->createVertexBuffer(instanceStreamData);
  }

  std::shared_ptr<ShaderBindingInstance> createMaterialShaderBindingInstance(
      const std::shared_ptr<Material>& material) {
    // TODO: in future consider shader reflection

    ShaderBindingArray                   shaderBindingArray;
    ShaderBindingResourceInlineAllocator resourceAllocator;

    std::vector<std::string> textureNames
        = {"albedo", "normal_map", "roughness", "metalness"};

    auto bindingPoint = 0;
    for (const auto& textureName : textureNames) {
      std::shared_ptr<Texture> texture;

      auto it = material->textures.find(textureName);
      if (it != material->textures.end() && it->second) {
        texture = it->second;
      } else {
        assert(false && "Texture not found in material");
      }

      shaderBindingArray.add(ShaderBinding(
          bindingPoint++,
          1,
          EShaderBindingType::TEXTURE_SAMPLER_SRV,
          false,
          EShaderAccessStageFlag::FRAGMENT,
          resourceAllocator.alloc<TextureResource>(texture.get(), nullptr)));
    }

    return g_rhi->createShaderBindingInstance(
        shaderBindingArray, ShaderBindingInstanceType::MultiFrame);
  }

  std::shared_ptr<RenderContext> m_renderContext_;

  PipelineStateFixedInfo pipelineStateFixed_;

  RenderPass* renderPass_ = nullptr;

  GraphicsPipelineShader shaders_;

  int32_t bindingPoint = 0;

  std::shared_ptr<IUniformBufferBlock>   viewUniformBuffer_;
  std::shared_ptr<ShaderBindingInstance> viewShaderBindingInstance_;

  std::shared_ptr<IUniformBufferBlock>   directionalLightUniformBuffer_;
  std::shared_ptr<ShaderBindingInstance> directionalLightShaderBindingInstance_;

  std::shared_ptr<IUniformBufferBlock>   pointLightUniformBuffer_;
  std::shared_ptr<ShaderBindingInstance> pointLightShaderBindingInstance_;

  std::shared_ptr<IUniformBufferBlock>   spotLightUniformBuffer_;
  std::shared_ptr<ShaderBindingInstance> spotLightShaderBindingInstance_;

  struct DrawData {
    PipelineStateInfo*                     pipelineStateInfo_ = nullptr;
    std::shared_ptr<RenderGeometryMesh>    renderGeometryMesh_;
    std::shared_ptr<Material>              material_;
    std::shared_ptr<VertexBuffer>          instanceBuffer_;
    uint32_t                               instanceCount = 0;
    std::shared_ptr<ShaderBindingInstance> materialShaderBindingInstance_;
  };

  std::vector<DrawData> drawDataList_;
};

class Renderer {
  public:
  Renderer() = default;

  void renderFrame(const std::shared_ptr<RenderContext>& renderContext) {
    // TODO: this is temporary solution, incorrect to call it
    renderContext->renderFrameContext = g_rhi->beginRenderFrame();

    initialize(renderContext);

    // m_renderContext_->renderFrameContext = g_rhi->beginRenderFrame();

    m_renderContext_->renderFrameContext->getActiveCommandBuffer()->begin();

    basePass_->execute();

    m_renderContext_->renderFrameContext->getActiveCommandBuffer()->end();

    m_renderContext_->renderFrameContext->submitCurrentActiveCommandBuffer(
        RenderFrameContext::BasePass);

    g_rhi->endRenderFrame(m_renderContext_->renderFrameContext);

    g_rhi->incrementFrameNumber();

    finalize();
  }

  private:
  void initialize(const std::shared_ptr<RenderContext>& renderContext) {
    m_renderContext_ = renderContext;
    if (basePass_ == nullptr) {
      basePass_ = std::make_unique<BasePass>();
    }
    basePass_->initialize(renderContext);
  }

  void finalize() { basePass_->finalize(); }

  // TODO: currently not the best architectural approach, consider changing
  std::shared_ptr<RenderContext> m_renderContext_;
  std::unique_ptr<BasePass>      basePass_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDERER_H
