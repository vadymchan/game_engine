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
#include "gfx/rhi/render_frame_context.h"
#include "gfx/rhi/rhi.h"
#include "scene/scene.h"
#include "utils/hot_reload/hot_reload_manager.h"
#include "utils/path_manager/path_manager.h"
#include "utils/service/service_locator.h"

namespace game_engine {

// This is RenderPass (Currently named so since it conflicts with RenderPass in
// RHI)
class RenderStage {
  public:
  virtual ~RenderStage() = default;
  virtual void initialize(const std::shared_ptr<RenderContext>& renderContext,
                          const EditorRenderParams& editorRenderParams)
      = 0;
  virtual void execute()  = 0;
  virtual void finalize() = 0;
};

class BasePass : public RenderStage {
  public:
  BasePass() = default;

  void initialize(const std::shared_ptr<RenderContext>& renderContext,
                  const EditorRenderParams& editorRenderParams) override {
    m_renderContext_      = renderContext;
    m_editorRenderParams_ = editorRenderParams;
    setupPipelineStates();
    setupRenderPass();
    preparePerFrameResources();
    prepareDrawCalls();
  }

  void execute() override {
    auto commandBuffer
        = m_renderContext_->renderFrameContext->getActiveCommandBuffer();

    // Transition Resource Layouts
    g_rhi->transitionLayout(
        commandBuffer,
        m_renderContext_->renderFrameContext->m_sceneRenderTarget_
            ->m_colorBuffer_->getTexture(),
        EResourceLayout::COLOR_ATTACHMENT);

    auto newLayout = m_renderContext_->renderFrameContext->m_sceneRenderTarget_
                             ->m_depthBuffer_->getTexture()
                             ->isDepthOnlyFormat()
                       ? EResourceLayout::DEPTH_ATTACHMENT
                       : EResourceLayout::DEPTH_STENCIL_ATTACHMENT;

    g_rhi->transitionLayout(
        commandBuffer,
        m_renderContext_->renderFrameContext->m_sceneRenderTarget_
            ->m_depthBuffer_->getTexture(),
        newLayout);

    if (renderPass_->beginRenderPass(commandBuffer)) {
      for (const auto& drawData : drawDataList_) {
        drawData.pipelineStateInfo_->bind(commandBuffer);

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
          /*const std::vector<uint32_t>* pDynamicOffsetTest
              = shaderBindingInstanceArray[i]->getDynamicOffsets();
          if (pDynamicOffsetTest && pDynamicOffsetTest->size()) {
            shaderBindingInstanceCombiner.m_dynamicOffsets_.add(
                (void*)pDynamicOffsetTest->data(),
                (int32_t)pDynamicOffsetTest->size());
          }*/
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

        g_rhi->bindGraphicsShaderBindingInstances(commandBuffer,
                                                  drawData.pipelineStateInfo_,
                                                  shaderBindingInstanceCombiner,
                                                  0);

        const auto& gpuMesh = drawData.renderGeometryMesh_;
        gpuMesh->vertexBuffer->bind(commandBuffer);
        drawData.instanceBuffer_->bind(commandBuffer);

        gpuMesh->indexBuffer->bind(commandBuffer);

        g_rhi->drawElementsInstanced(commandBuffer,
                                     0,
                                     gpuMesh->indexBuffer->getElementCount(),
                                     drawData.instanceCount);
      }

      renderPass_->endRenderPass();
    }

    // TODO: actually it's better to copy from color / post process buffer to
    // the back buffer in the final pass
    // g_rhi->transitionLayout(
    //    commandBuffer,
    //    m_renderContext_->renderFrameContext->m_sceneRenderTarget_
    //        ->m_colorBuffer_->getTexture(),
    //    EResourceLayout::PRESENT_SRC);
  }

  void finalize() override {
    // TODO:
    // PipelineStateInfo*                     pipelineStateInfo_
    // all ShaderBindingInstance
    // RenderPass* renderPass_
    // IUniformBufferBlock (add correct ~ in vulkan)
    // shaders_.m_vertexShader_ delete

    // delete renderPass_;

    // viewShaderBindingInstance_.reset();
    // directionalLightShaderBindingInstance_.reset();
    // pointLightShaderBindingInstance_.reset();
    // spotLightShaderBindingInstance_.reset();

    drawDataList_.clear();
  }

  private:
  void setupPipelineStates() {
    auto viewportDimension = m_editorRenderParams_.editorViewportDimension;
    // auto viewportDimension = m_renderContext_->viewportDimension;

    auto rasterizationState
        = TRasterizationStateInfo<EPolygonMode::FILL,
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
                 static_cast<float>(viewportDimension.width()),
                 static_cast<float>(viewportDimension.height())),
        Scissor(0, 0, viewportDimension.width(), viewportDimension.height()),
        false /*gOptions.UseVRS*/);

    static bool isShaderInitialized = false;

    // TODO: use config in future
    if (!isShaderInitialized) {
      ShaderInfo vertexShaderInfo;
      vertexShaderInfo.setName(NameStatic("BasePassVS"));
      vertexShaderInfo.setShaderFilepath(
          NameStatic("assets/shaders/base_pass/shader_instancing.vs.hlsl"));
      vertexShaderInfo.setShaderType(EShaderAccessStageFlag::VERTEX);

      ShaderInfo pixelShaderInfo;
      pixelShaderInfo.setName(NameStatic("BasePassPS"));
      pixelShaderInfo.setShaderFilepath(
          NameStatic("assets/shaders/base_pass/shader.ps.hlsl"));
      pixelShaderInfo.setShaderType(EShaderAccessStageFlag::FRAGMENT);

      auto newVertexShader = g_rhi->createShader(vertexShaderInfo);
      auto newPixelShader  = g_rhi->createShader(pixelShaderInfo);

      {
        std::lock_guard<std::mutex> lock(shaders_mutex_);
        shaders_.m_vertexShader_ = newVertexShader;
        shaders_.m_pixelShader_  = newPixelShader;
      }

      // hot reload for shaders
      {
        auto fileModificationHandler = [this](const wtr::event& e) {
          std::lock_guard<std::mutex> lock(shaders_mutex_);

          const std::string vertexShaderPath
              = "assets/shaders/base_pass/shader_instancing.vs.hlsl";
          const std::string pixelShaderPath
              = "assets/shaders/base_pass/shader.ps.hlsl";

          const std::string absolutePath = e.path_name.generic_string();

          if (absolutePath.find(vertexShaderPath) != std::string::npos) {
            ShaderInfo vertexShaderInfo;
            vertexShaderInfo.setName(NameStatic("BasePassVS"));
            vertexShaderInfo.setShaderFilepath(
                NameStatic(e.path_name.string()));
            vertexShaderInfo.setShaderType(EShaderAccessStageFlag::VERTEX);
            shaders_.m_vertexShader_ = g_rhi->createShader(vertexShaderInfo);
          }

          if (absolutePath.find(pixelShaderPath) != std::string::npos) {
            ShaderInfo pixelShaderInfo;
            pixelShaderInfo.setName(NameStatic("BasePassPS"));
            pixelShaderInfo.setShaderFilepath(NameStatic(e.path_name.string()));
            pixelShaderInfo.setShaderType(EShaderAccessStageFlag::FRAGMENT);
            shaders_.m_pixelShader_ = g_rhi->createShader(pixelShaderInfo);
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
    auto viewportDimension = m_editorRenderParams_.editorViewportDimension;
    // auto viewportDimension = m_renderContext_->viewportDimension;

    const RtClearValue clearColor = RtClearValue(0.0f, 0.0f, 0.0f, 1.0f);
    const RtClearValue clearDepth = RtClearValue(1.0f, 0);

    Attachment colorAttachment(m_renderContext_->renderFrameContext
                                   ->m_sceneRenderTarget_->m_colorBuffer_,
                               EAttachmentLoadStoreOp::CLEAR_STORE,
                               EAttachmentLoadStoreOp::DONTCARE_DONTCARE,
                               clearColor,
                               EResourceLayout::UNDEFINED,
                               EResourceLayout::COLOR_ATTACHMENT);

    Attachment depthAttachment(m_renderContext_->renderFrameContext
                                   ->m_sceneRenderTarget_->m_depthBuffer_,
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
        {viewportDimension.width(), viewportDimension.height()});
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
                                          LifeTimeType::OneFrame,
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

      if (viewShaderBindingInstance_) {
        viewShaderBindingInstance_->free();
      }

      viewShaderBindingInstance_ = g_rhi->createShaderBindingInstance(
          shaderBindingArray, ShaderBindingInstanceType::SingleFrame);
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
                                            LifeTimeType::OneFrame,
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

        if (directionalLightShaderBindingInstance_) {
          directionalLightShaderBindingInstance_->free();
        }

        directionalLightShaderBindingInstance_
            = g_rhi->createShaderBindingInstance(
                directionalLightSBA, ShaderBindingInstanceType::SingleFrame);
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
            g_rhi->createUniformBufferBlock(
                Name("PointLightBuffer"), LifeTimeType::OneFrame, sizeof(pld)));
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

        if (pointLightShaderBindingInstance_) {
          pointLightShaderBindingInstance_->free();
        }

        pointLightShaderBindingInstance_ = g_rhi->createShaderBindingInstance(
            pointLightSBA, ShaderBindingInstanceType::SingleFrame);
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
            g_rhi->createUniformBufferBlock(
                Name("SpotLightBuffer"), LifeTimeType::OneFrame, sizeof(sld)));
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

        if (spotLightShaderBindingInstance_) {
          spotLightShaderBindingInstance_->free();
        }

        spotLightShaderBindingInstance_ = g_rhi->createShaderBindingInstance(
            spotLightSBA, ShaderBindingInstanceType::SingleFrame);
      } else {
        spotLightShaderBindingInstance_ = nullptr;
      }
    }
  }

  void prepareDrawCalls() {
    GraphicsPipelineShader currentShaders;
    {
      std::lock_guard<std::mutex> lock(shaders_mutex_);
      currentShaders = shaders_;
    }

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
              shaderBindingInstanceArray[i]->m_shaderBindingsLayouts_.get());
        }

        VertexBufferArray vertexBufferArray;
        vertexBufferArray.add(
            drawData.renderGeometryMesh_->vertexBuffer.get());  // Binding 0
        vertexBufferArray.add(drawData.instanceBuffer_.get());  // Binding 1

        // TODO: investigate that (currently seems unoptimized to create
        // Pipeline state per each drawCall (lots of state switching))
        drawData.pipelineStateInfo_
            = g_rhi->createPipelineStateInfo(&pipelineStateFixed_,
                                             currentShaders,
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
        shaderBindingArray, ShaderBindingInstanceType::SingleFrame);
  }

  std::shared_ptr<RenderContext> m_renderContext_;
  EditorRenderParams             m_editorRenderParams_;

  PipelineStateFixedInfo pipelineStateFixed_;

  RenderPass* renderPass_ = nullptr;

  GraphicsPipelineShader shaders_;
  std::mutex             shaders_mutex_;

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

    ~DrawData() {
      // if (materialShaderBindingInstance_) {
      //   materialShaderBindingInstance_->free();
      // }
    }
  };

  std::vector<DrawData> drawDataList_;
};

class FinalPass : public RenderStage {
  public:
  void initialize(const std::shared_ptr<RenderContext>& renderContext,
                  const EditorRenderParams& editorRenderParams) override {
    m_renderContext_ = renderContext;
  }

  void execute() override {
    // here we only copy from color buffer to the back buffer
    auto commandBuffer
        = m_renderContext_->renderFrameContext->getActiveCommandBuffer();

    auto colorBuffer = m_renderContext_->renderFrameContext
                           ->m_sceneRenderTarget_->m_colorBuffer_->getTexture();

    auto backBuffer = m_renderContext_->renderFrameContext->m_sceneRenderTarget_
                          ->m_backBuffer_->getTexture();

    g_rhi->copyTexture(commandBuffer, colorBuffer, backBuffer);
  }

  void finalize() override {}

  private:
  std::shared_ptr<RenderContext> m_renderContext_;
};

class IDebugDrawStrategy {
  public:
  virtual ~IDebugDrawStrategy() = default;

  virtual void initialize(const std::shared_ptr<RenderContext>&,
                          const EditorRenderParams&)
      = 0;

  virtual void execute() = 0;

  virtual void finalize() = 0;

  virtual bool isExclusive() const { return false; }
};

// heatmap
class ShaderOverdrawStrategy : public IDebugDrawStrategy {
  public:
  bool isExclusive() const override { return true; }

  void initialize(const std::shared_ptr<RenderContext>& renderContext,
                  const EditorRenderParams& editorRenderParams) override {
    m_renderContext_      = renderContext;
    m_editorRenderParams_ = editorRenderParams;
    setupPipelineStates();
    setupRenderPass();
    preparePerFrameResources();
    prepareDrawCalls();
  }

  void execute() override {
    auto commandBuffer
        = m_renderContext_->renderFrameContext->getActiveCommandBuffer();

    g_rhi->transitionLayout(
        commandBuffer,
        m_renderContext_->renderFrameContext->m_sceneRenderTarget_
            ->m_colorBuffer_->getTexture(),
        EResourceLayout::COLOR_ATTACHMENT);

    // auto newLayout =
    // m_renderContext_->renderFrameContext->m_sceneRenderTarget_
    //                          ->m_depthBuffer_->getTexture()
    //                          ->isDepthOnlyFormat()
    //                    ? EResourceLayout::DEPTH_ATTACHMENT
    //                    : EResourceLayout::DEPTH_STENCIL_ATTACHMENT;

    // g_rhi->transitionLayout(
    //     commandBuffer,
    //     m_renderContext_->renderFrameContext->m_sceneRenderTarget_
    //         ->m_depthBuffer_->getTexture(),
    //     newLayout);

    if (renderPass_->beginRenderPass(commandBuffer)) {
      for (const auto& drawData : drawDataList_) {
        drawData.pipelineStateInfo_->bind(commandBuffer);

        ShaderBindingInstanceArray bindingArray;
        bindingArray.add(viewShaderBindingInstance_.get());

        ShaderBindingInstanceCombiner combiner;
        for (int i = 0; i < bindingArray.m_numOfData_; ++i) {
          combiner.m_descriptorSetHandles_.add(bindingArray[i]->getHandle());
        }
        combiner.m_shaderBindingInstanceArray = &bindingArray;

        g_rhi->bindGraphicsShaderBindingInstances(
            commandBuffer, drawData.pipelineStateInfo_, combiner, 0);

        drawData.renderGeometryMesh_->vertexBuffer->bind(commandBuffer);
        drawData.instanceBuffer_->bind(commandBuffer);
        drawData.renderGeometryMesh_->indexBuffer->bind(commandBuffer);

        g_rhi->drawElementsInstanced(
            commandBuffer,
            0,
            drawData.renderGeometryMesh_->indexBuffer->getElementCount(),
            drawData.instanceCount);
      }

      renderPass_->endRenderPass();
    }
  }

  void finalize() override {
    drawDataList_.clear();

    // viewShaderBindingInstance_.reset();
  }

  private:
  void setupPipelineStates() {
    auto viewportDimension = m_editorRenderParams_.editorViewportDimension;

    auto rasterizationState = TRasterizationStateInfo<EPolygonMode::FILL,
                                                      ECullMode::BACK,
                                                      EFrontFace::CCW,
                                                      false,
                                                      0.0f,
                                                      0.0f,
                                                      0.0f,
                                                      1.0f,
                                                      false,
                                                      false,
                                                      EMSAASamples::COUNT_1,
                                                      false,
                                                      0.0f,
                                                      false,
                                                      false>::s_create();

    // TODO: turn off depth
    // auto depthStencilState = TDepthStencilStateInfo<true,
    //                                                true,
    //                                                ECompareOp::LESS,
    //                                                false,
    //                                                false,
    //                                                0.0f,
    //                                                1.0f>::s_create();

    auto blendingState = TBlendingStateInfo<true,
                                            EBlendFactor::SRC_ALPHA,
                                            EBlendFactor::ONE,
                                            EBlendOp::ADD,
                                            EBlendFactor::ONE,
                                            EBlendFactor::ONE,
                                            EBlendOp::ADD,
                                            EColorMask::ALL>::s_create();

    pipelineStateFixed_ = PipelineStateFixedInfo(
        rasterizationState,
        // depthStencilState,
        nullptr,
        blendingState,
        Viewport(0.0f,
                 0.0f,
                 static_cast<float>(viewportDimension.width()),
                 static_cast<float>(viewportDimension.height())),
        Scissor(0, 0, viewportDimension.width(), viewportDimension.height()),
        false /*gOptions.UseVRS*/);

    ShaderInfo vsInfo;
    vsInfo.setName(NameStatic("OverdrawVS"));
    vsInfo.setShaderFilepath(
        NameStatic("assets/shaders/debug/overdraw/shader_instancing.vs.hlsl"));
    vsInfo.setShaderType(EShaderAccessStageFlag::VERTEX);

    ShaderInfo psInfo;
    psInfo.setName(NameStatic("OverdrawPS"));
    psInfo.setShaderFilepath(
        NameStatic("assets/shaders/debug/overdraw/shader.ps.hlsl"));
    psInfo.setShaderType(EShaderAccessStageFlag::FRAGMENT);

    auto vs = g_rhi->createShader(vsInfo);
    auto ps = g_rhi->createShader(psInfo);

    {
      std::lock_guard<std::mutex> lock(shaders_mutex_);
      shaders_.m_vertexShader_ = vs;
      shaders_.m_pixelShader_  = ps;
    }

    // TODO: hot reload for shaders
  }

  void setupRenderPass() {
    auto viewportDimension = m_editorRenderParams_.editorViewportDimension;
    const RtClearValue clearColor(0.0f, 0.0f, 0.0f, 1.0f);
    const RtClearValue clearDepth(1.0f, 0);

    // colorAttachment:
    //  В ShaderOverdraw часто хотим НЕ обнулять буфер (чтобы видеть
    //  накопление), но здесь можно сделать так же, как у вас. Или заменить
    //  EAttachmentLoadStoreOp::LOAD_STORE чтобы не затирать цвет, если хотите
    //  наложение оверлея поверх.
    Attachment colorAttachment(m_renderContext_->renderFrameContext
                                   ->m_sceneRenderTarget_->m_colorBuffer_,
                               EAttachmentLoadStoreOp::CLEAR_STORE,
                               EAttachmentLoadStoreOp::DONTCARE_DONTCARE,
                               clearColor,
                               EResourceLayout::UNDEFINED,
                               EResourceLayout::COLOR_ATTACHMENT);

    // Attachment depthAttachment(m_renderContext_->renderFrameContext
    //                                ->m_sceneRenderTarget_->m_depthBuffer_,
    //                            EAttachmentLoadStoreOp::CLEAR_STORE,
    //                            EAttachmentLoadStoreOp::DONTCARE_DONTCARE,
    //                            clearDepth,
    //                            EResourceLayout::UNDEFINED,
    //                            EResourceLayout::DEPTH_STENCIL_ATTACHMENT);

    RenderPassInfo passInfo;
    passInfo.m_attachments_.push_back(colorAttachment);
    // passInfo.m_attachments_.push_back(depthAttachment);

    Subpass sp;
    sp.initialize(0,
                  1,
                  EPipelineStageMask::COLOR_ATTACHMENT_OUTPUT_BIT,
                  EPipelineStageMask::FRAGMENT_SHADER_BIT);
    sp.m_outputColorAttachments_.push_back(0);
    // sp.m_outputDepthAttachment_ = 1;

    passInfo.m_subpasses_.push_back(sp);

    renderPass_ = g_rhi->getOrCreateRenderPass(
        passInfo,
        {0, 0},
        {viewportDimension.width(), viewportDimension.height()});
  }

  void preparePerFrameResources() {
    auto view = m_renderContext_->scene->getEntityRegistry()
                    .view<Transform, Camera, CameraMatrices>();
    if (view.begin() == view.end()) {
      // TODO: add loagger
      return;
    }
    auto  entity    = view.front();
    auto& transform = view.get<Transform>(entity);
    auto& camMat    = view.get<CameraMatrices>(entity);

    struct ViewUbo {
      math::Matrix4f<> view;
      math::Matrix4f<> proj;
      math::Matrix4f<> viewProj;
      math::Vector3Df  eye;
      float            pad;
    } ubo{};

    ubo.view     = camMat.view;
    ubo.proj     = camMat.projection;
    ubo.viewProj = camMat.view * camMat.projection;
    ubo.eye      = transform.translation;

    viewUboBlock_
        = std::shared_ptr<IUniformBufferBlock>(g_rhi->createUniformBufferBlock(
            Name("OverdrawViewUBO"), LifeTimeType::OneFrame, sizeof(ubo)));
    viewUboBlock_->updateBufferData(&ubo, sizeof(ubo));

    ShaderBindingArray                   sba;
    ShaderBindingResourceInlineAllocator alloc;
    sba.add(
        ShaderBinding(0,
                      1,
                      EShaderBindingType::UNIFORMBUFFER,
                      false,
                      EShaderAccessStageFlag::ALL_GRAPHICS,
                      alloc.alloc<UniformBufferResource>(viewUboBlock_.get())));

    viewShaderBindingInstance_ = g_rhi->createShaderBindingInstance(
        sba, ShaderBindingInstanceType::SingleFrame);
  }

  void prepareDrawCalls() {
    GraphicsPipelineShader localShaders;
    {
      std::lock_guard<std::mutex> lock(shaders_mutex_);
      localShaders = shaders_;
    }

    auto& registry = m_renderContext_->scene->getEntityRegistry();
    auto  view     = registry.view<Transform, std::shared_ptr<RenderModel>>();

    std::unordered_map<std::shared_ptr<RenderModel>,
                       std::vector<math::Matrix4f<>>>
        modelInstances;

    for (auto entity : view) {
      auto& transform   = view.get<Transform>(entity);
      auto& renderModel = view.get<std::shared_ptr<RenderModel>>(entity);

      math::Matrix4f<> mtx = calculateTransformMatrix(transform);
      modelInstances[renderModel].push_back(mtx);
    }

    for (auto& [modelPtr, matrices] : modelInstances) {
      std::shared_ptr<VertexBuffer> instanceBuf;
      createInstanceBuffer(matrices, instanceBuf);

      for (auto& renderMesh : modelPtr->renderMeshes) {
        DrawData drawData{};
        drawData.renderGeometryMesh_ = renderMesh->gpuMesh;
        drawData.instanceBuffer_     = instanceBuf;
        drawData.instanceCount       = static_cast<uint32_t>(matrices.size());

        ShaderBindingInstanceArray bindingArr;
        bindingArr.add(viewShaderBindingInstance_.get());

        ShaderBindingLayoutArray layoutArr;
        for (int i = 0; i < bindingArr.m_numOfData_; ++i) {
          layoutArr.add(bindingArr[i]->m_shaderBindingsLayouts_.get());
        }

        VertexBufferArray vbArray;
        vbArray.add(
            drawData.renderGeometryMesh_->vertexBuffer.get());  // binding=0
        vbArray.add(drawData.instanceBuffer_.get());            // binding=1

        drawData.pipelineStateInfo_
            = g_rhi->createPipelineStateInfo(&pipelineStateFixed_,
                                             localShaders,
                                             vbArray,
                                             renderPass_,
                                             layoutArr,
                                             nullptr,
                                             0);

        drawDataList_.push_back(drawData);
      }
    }
  }

  void createInstanceBuffer(const std::vector<math::Matrix4f<>>& matrices,
                            std::shared_ptr<VertexBuffer>& instanceBuffer) {
    constexpr int      kMatSize = 16;
    std::vector<float> data;
    data.reserve(matrices.size() * kMatSize);

    for (auto& mat : matrices) {
      data.insert(data.end(), mat.data(), mat.data() + kMatSize);
    }

    auto stream = std::make_shared<BufferAttributeStream<float>>(
        Name("INSTANCE"),
        EBufferType::Static,
        math::Matrix4f<>::GetDataSize(),
        std::vector<IBufferAttribute::Attribute>{
          {EBufferElementType::FLOAT,                  0, sizeof(float) * 4},
          {EBufferElementType::FLOAT,  sizeof(float) * 4, sizeof(float) * 4},
          {EBufferElementType::FLOAT,  sizeof(float) * 8, sizeof(float) * 4},
          {EBufferElementType::FLOAT, sizeof(float) * 12, sizeof(float) * 4},
    },
        std::move(data));

    auto instanceStreamData = std::make_shared<VertexStreamData>();
    instanceStreamData->m_streams_.push_back(stream);
    instanceStreamData->m_primitiveType_ = EPrimitiveType::TRIANGLES;
    instanceStreamData->m_elementCount_ = static_cast<int32_t>(matrices.size());
    instanceStreamData->m_bindingIndex_ = 6;
    instanceStreamData->m_vertexInputRate_ = EVertexInputRate::INSTANCE;

    instanceBuffer = g_rhi->createVertexBuffer(instanceStreamData);
  }

  private:
  std::shared_ptr<RenderContext> m_renderContext_;
  EditorRenderParams             m_editorRenderParams_;

  PipelineStateFixedInfo pipelineStateFixed_;
  GraphicsPipelineShader shaders_;
  std::mutex             shaders_mutex_;

  RenderPass* renderPass_ = nullptr;

  std::shared_ptr<IUniformBufferBlock>   viewUboBlock_;
  std::shared_ptr<ShaderBindingInstance> viewShaderBindingInstance_;

  struct DrawData {
    PipelineStateInfo*                  pipelineStateInfo_ = nullptr;
    std::shared_ptr<RenderGeometryMesh> renderGeometryMesh_;
    std::shared_ptr<VertexBuffer>       instanceBuffer_;
    uint32_t                            instanceCount = 0;
  };

  std::vector<DrawData> drawDataList_;
};

class NormalMapVisualizationStrategy : public IDebugDrawStrategy {
  public:
  bool isExclusive() const override { return true; }

  void initialize(const std::shared_ptr<RenderContext>& renderContext,
                  const EditorRenderParams& editorRenderParams) override {
    m_renderContext_      = renderContext;
    m_editorRenderParams_ = editorRenderParams;
    setupPipelineStates();
    setupRenderPass();
    preparePerFrameResources();
    prepareDrawCalls();
  }

  void execute() override {
    auto commandBuffer
        = m_renderContext_->renderFrameContext->getActiveCommandBuffer();

    g_rhi->transitionLayout(
        commandBuffer,
        m_renderContext_->renderFrameContext->m_sceneRenderTarget_
            ->m_colorBuffer_->getTexture(),
        EResourceLayout::COLOR_ATTACHMENT);

    auto newLayout = m_renderContext_->renderFrameContext->m_sceneRenderTarget_
                             ->m_depthBuffer_->getTexture()
                             ->isDepthOnlyFormat()
                       ? EResourceLayout::DEPTH_ATTACHMENT
                       : EResourceLayout::DEPTH_STENCIL_ATTACHMENT;

    g_rhi->transitionLayout(
        commandBuffer,
        m_renderContext_->renderFrameContext->m_sceneRenderTarget_
            ->m_depthBuffer_->getTexture(),
        newLayout);

    if (renderPass_->beginRenderPass(commandBuffer)) {
      for (const auto& drawData : drawDataList_) {
        drawData.pipelineStateInfo_->bind(commandBuffer);

        ShaderBindingInstanceArray shaderBindingInstanceArray;
        shaderBindingInstanceArray.add(viewShaderBindingInstance_.get());

        if (drawData.materialShaderBindingInstance_) {
          shaderBindingInstanceArray.add(
              drawData.materialShaderBindingInstance_.get());
        }

        ShaderBindingInstanceCombiner shaderBindingInstanceCombiner;
        for (int32_t i = 0; i < shaderBindingInstanceArray.m_numOfData_; ++i) {
          shaderBindingInstanceCombiner.m_descriptorSetHandles_.add(
              shaderBindingInstanceArray[i]->getHandle());
        }
        shaderBindingInstanceCombiner.m_shaderBindingInstanceArray
            = &shaderBindingInstanceArray;

        g_rhi->bindGraphicsShaderBindingInstances(commandBuffer,
                                                  drawData.pipelineStateInfo_,
                                                  shaderBindingInstanceCombiner,
                                                  0);

        const auto& gpuMesh = drawData.renderGeometryMesh_;
        gpuMesh->vertexBuffer->bind(commandBuffer);
        drawData.instanceBuffer_->bind(commandBuffer);

        gpuMesh->indexBuffer->bind(commandBuffer);

        g_rhi->drawElementsInstanced(commandBuffer,
                                     0,
                                     gpuMesh->indexBuffer->getElementCount(),
                                     drawData.instanceCount);
      }

      renderPass_->endRenderPass();
    }
  }

  void finalize() override {
    // viewShaderBindingInstance_.reset();

    drawDataList_.clear();
  }

  private:
  void setupPipelineStates() {
    auto viewportDimension = m_editorRenderParams_.editorViewportDimension;
    // auto viewportDimension = m_renderContext_->viewportDimension;

    auto rasterizationState
        = TRasterizationStateInfo<EPolygonMode::FILL,
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
                 static_cast<float>(viewportDimension.width()),
                 static_cast<float>(viewportDimension.height())),
        Scissor(0, 0, viewportDimension.width(), viewportDimension.height()),
        false /*gOptions.UseVRS*/);

    ShaderInfo vsInfo;
    vsInfo.setName(NameStatic("NormalMapVisVS"));
    vsInfo.setShaderFilepath(
        NameStatic("assets/shaders/debug/normal_map_visualization/"
                   "shader_instancing.vs.hlsl"));
    vsInfo.setShaderType(EShaderAccessStageFlag::VERTEX);

    ShaderInfo psInfo;
    psInfo.setName(NameStatic("NormalMapVisPS"));
    psInfo.setShaderFilepath(NameStatic(
        "assets/shaders/debug/normal_map_visualization/shader.ps.hlsl"));
    psInfo.setShaderType(EShaderAccessStageFlag::FRAGMENT);

    auto vs = g_rhi->createShader(vsInfo);
    auto ps = g_rhi->createShader(psInfo);

    {
      std::lock_guard<std::mutex> lock(shaders_mutex_);
      shaders_.m_vertexShader_ = vs;
      shaders_.m_pixelShader_  = ps;
    }
  }

  void setupRenderPass() {
    auto viewportDimension = m_editorRenderParams_.editorViewportDimension;
    // auto viewportDimension = m_renderContext_->viewportDimension;

    const RtClearValue clearColor = RtClearValue(0.0f, 0.0f, 0.0f, 1.0f);
    const RtClearValue clearDepth = RtClearValue(1.0f, 0);

    Attachment colorAttachment(m_renderContext_->renderFrameContext
                                   ->m_sceneRenderTarget_->m_colorBuffer_,
                               EAttachmentLoadStoreOp::CLEAR_STORE,
                               EAttachmentLoadStoreOp::DONTCARE_DONTCARE,
                               clearColor,
                               EResourceLayout::UNDEFINED,
                               EResourceLayout::COLOR_ATTACHMENT);

    Attachment depthAttachment(m_renderContext_->renderFrameContext
                                   ->m_sceneRenderTarget_->m_depthBuffer_,
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
        {viewportDimension.width(), viewportDimension.height()});
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
                                          LifeTimeType::OneFrame,
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

      if (viewShaderBindingInstance_) {
        viewShaderBindingInstance_->free();
      }

      viewShaderBindingInstance_ = g_rhi->createShaderBindingInstance(
          shaderBindingArray, ShaderBindingInstanceType::SingleFrame);
    }
  }

  void prepareDrawCalls() {
    GraphicsPipelineShader currentShaders;
    {
      std::lock_guard<std::mutex> lock(shaders_mutex_);
      currentShaders = shaders_;
    }

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

        if (drawData.material_) {
          drawData.materialShaderBindingInstance_
              = createMaterialShaderBindingInstance(drawData.material_);
          shaderBindingInstanceArray.add(
              drawData.materialShaderBindingInstance_.get());
        }

        ShaderBindingLayoutArray shaderBindingLayoutArray;
        for (int32_t i = 0; i < shaderBindingInstanceArray.m_numOfData_; ++i) {
          shaderBindingLayoutArray.add(
              shaderBindingInstanceArray[i]->m_shaderBindingsLayouts_.get());
        }

        VertexBufferArray vertexBufferArray;
        vertexBufferArray.add(
            drawData.renderGeometryMesh_->vertexBuffer.get());  // Binding 0
        vertexBufferArray.add(drawData.instanceBuffer_.get());  // Binding 1

        // TODO: investigate that (currently seems unoptimized to create
        // Pipeline state per each drawCall (lots of state switching))
        drawData.pipelineStateInfo_
            = g_rhi->createPipelineStateInfo(&pipelineStateFixed_,
                                             currentShaders,
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

    std::vector<std::string> textureNames = {"normal_map"};

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
        shaderBindingArray, ShaderBindingInstanceType::SingleFrame);
  }

  std::shared_ptr<ShaderBindingInstance> createMaterialShaderBinding(
      const std::shared_ptr<Material>& material);

  private:
  std::shared_ptr<RenderContext> m_renderContext_;
  EditorRenderParams             m_editorRenderParams_;

  PipelineStateFixedInfo pipelineStateFixed_;
  RenderPass*            renderPass_ = nullptr;

  GraphicsPipelineShader shaders_;
  std::mutex             shaders_mutex_;

  std::shared_ptr<IUniformBufferBlock>   viewUniformBuffer_;
  std::shared_ptr<ShaderBindingInstance> viewShaderBindingInstance_;

  struct DrawData {
    PipelineStateInfo*                     pipelineStateInfo_ = nullptr;
    std::shared_ptr<RenderGeometryMesh>    renderGeometryMesh_;
    std::shared_ptr<Material>              material_;
    std::shared_ptr<VertexBuffer>          instanceBuffer_;
    uint32_t                               instanceCount = 0;
    std::shared_ptr<ShaderBindingInstance> materialShaderBindingInstance_;

    ~DrawData() {
      // if (materialShaderBindingInstance_) {
      //   materialShaderBindingInstance_->free();
      // }
    }
  };

  std::vector<DrawData> drawDataList_;
};

class VertexNormalVisualizationStrategy : public IDebugDrawStrategy {
  public:
  bool isExclusive() const override { return false; }

  void initialize(const std::shared_ptr<RenderContext>& renderContext,
                  const EditorRenderParams& editorRenderParams) override {
    m_renderContext_      = renderContext;
    m_editorRenderParams_ = editorRenderParams;

    setupPipelineStates();
    setupRenderPass();
    preparePerFrameResources();
    prepareDrawCalls();
  }

  void execute() override {
    auto commandBuffer
        = m_renderContext_->renderFrameContext->getActiveCommandBuffer();

    g_rhi->transitionLayout(
        commandBuffer,
        m_renderContext_->renderFrameContext->m_sceneRenderTarget_
            ->m_colorBuffer_->getTexture(),
        EResourceLayout::COLOR_ATTACHMENT);

    auto newLayout = EResourceLayout::DEPTH_STENCIL_ATTACHMENT;
    if (!m_renderContext_->renderFrameContext->m_sceneRenderTarget_
             ->m_depthBuffer_->getTexture()
             ->isDepthOnlyFormat()) {
      newLayout = EResourceLayout::DEPTH_STENCIL_ATTACHMENT;
    }

    g_rhi->transitionLayout(
        commandBuffer,
        m_renderContext_->renderFrameContext->m_sceneRenderTarget_
            ->m_depthBuffer_->getTexture(),
        newLayout);

    if (renderPass_->beginRenderPass(commandBuffer)) {
      for (auto& drawData : m_drawDataList) {
        drawData.pipelineStateInfo_->bind(commandBuffer);

        ShaderBindingInstanceArray bindingArray;
        bindingArray.add(viewShaderBindingInstance_.get());

        ShaderBindingInstanceCombiner combiner;
        for (int i = 0; i < bindingArray.m_numOfData_; ++i) {
          combiner.m_descriptorSetHandles_.add(bindingArray[i]->getHandle());
        }
        combiner.m_shaderBindingInstanceArray = &bindingArray;

        g_rhi->bindGraphicsShaderBindingInstances(
            commandBuffer, drawData.pipelineStateInfo_, combiner, 0);

        drawData.renderMesh_->vertexBuffer->bind(commandBuffer);
        drawData.instanceBuffer_->bind(commandBuffer);
        drawData.renderMesh_->indexBuffer->bind(commandBuffer);

        // draw
        // ВАЖНО: geometry shader сам генерирует линии,
        // но input "primitive type" всё ещё TRIANGLES
        g_rhi->drawElementsInstanced(
            commandBuffer,
            0,
            drawData.renderMesh_->indexBuffer->getElementCount(),
            drawData.instanceCount);
      }

      renderPass_->endRenderPass();
    }
  }

  void finalize() override {
    m_drawDataList.clear();
    /*viewShaderBindingInstance_.reset();
    viewUniformBuffer_.reset();

    {
      std::lock_guard<std::mutex> lock(m_shadersMutex);
      m_shaders.m_vertexShader_.reset();
      m_shaders.m_geometryShader_.reset();
      m_shaders.m_pixelShader_.reset();
    }*/
  }

  private:
  struct DrawData {
    PipelineStateInfo*                  pipelineStateInfo_ = nullptr;
    std::shared_ptr<RenderGeometryMesh> renderMesh_;
    std::shared_ptr<VertexBuffer>       instanceBuffer_;
    uint32_t                            instanceCount = 0;
  };

  std::shared_ptr<RenderContext> m_renderContext_;
  EditorRenderParams             m_editorRenderParams_;

  PipelineStateFixedInfo m_pipelineStateFixed;
  GraphicsPipelineShader m_shaders;
  std::mutex             m_shadersMutex;

  RenderPass*           renderPass_ = nullptr;
  std::vector<DrawData> m_drawDataList;

  std::shared_ptr<IUniformBufferBlock>   viewUniformBuffer_;
  std::shared_ptr<ShaderBindingInstance> viewShaderBindingInstance_;

  private:
  void setupPipelineStates() {
    auto vpDim = m_editorRenderParams_.editorViewportDimension;

    // Растеризация: можно FILL,
    // но мы фактически выводим линии geometry shader'ом
    // (он создаёт line primitives).
    auto rasterState = TRasterizationStateInfo<EPolygonMode::FILL,
                                               ECullMode::BACK,
                                               EFrontFace::CCW,
                                               false,
                                               0.f,
                                               0.f,
                                               0.f,
                                               1.f,
                                               false,
                                               false,
                                               EMSAASamples::COUNT_1,
                                               false,
                                               0.f,
                                               false,
                                               false>::s_create();

    auto depthStencilState = TDepthStencilStateInfo<true,
                                                    false,
                                                    ECompareOp::LESS,
                                                    false,
                                                    false,
                                                    0.f,
                                                    1.f>::s_create();

    auto blendingState = TBlendingStateInfo<false,
                                            EBlendFactor::ONE,
                                            EBlendFactor::ZERO,
                                            EBlendOp::ADD,
                                            EBlendFactor::ONE,
                                            EBlendFactor::ZERO,
                                            EBlendOp::ADD,
                                            EColorMask::ALL>::s_create();

    m_pipelineStateFixed
        = PipelineStateFixedInfo(rasterState,
                                 depthStencilState,
                                 blendingState,
                                 Viewport(0.f,
                                          0.f,
                                          static_cast<float>(vpDim.width()),
                                          static_cast<float>(vpDim.height())),
                                 Scissor(0, 0, vpDim.width(), vpDim.height()),
                                 false);

    {
      ShaderInfo vsInfo;
      vsInfo.setName(NameStatic("VertexNormalVisVS"));
      vsInfo.setShaderFilepath(
          NameStatic("assets/shaders/debug/geometry_normal_visualization/"
                     "shader_instancing.vs.hlsl"));
      vsInfo.setShaderType(EShaderAccessStageFlag::VERTEX);

      ShaderInfo gsInfo;
      gsInfo.setName(NameStatic("VertexNormalVisGS"));
      gsInfo.setShaderFilepath(NameStatic(
          "assets/shaders/debug/geometry_normal_visualization/shader.gs.hlsl"));
      gsInfo.setShaderType(EShaderAccessStageFlag::GEOMETRY);

      ShaderInfo psInfo;
      psInfo.setName(NameStatic("VertexNormalVisPS"));
      psInfo.setShaderFilepath(NameStatic(
          "assets/shaders/debug/geometry_normal_visualization/shader.ps.hlsl"));
      psInfo.setShaderType(EShaderAccessStageFlag::FRAGMENT);

      auto vs = g_rhi->createShader(vsInfo);
      auto gs = g_rhi->createShader(gsInfo);
      auto ps = g_rhi->createShader(psInfo);

      std::lock_guard<std::mutex> lock(m_shadersMutex);
      m_shaders.m_vertexShader_   = vs;
      m_shaders.m_geometryShader_ = gs;
      m_shaders.m_pixelShader_    = ps;
    }
  }

  void setupRenderPass() {
    auto vpDim = m_editorRenderParams_.editorViewportDimension;
    // Так как это оверлей, хотим не очищать color буфер (LOAD_STORE)
    // а depth можно очистить или тоже загрузить.
    // Но если у нас уже есть сцена, мы не хотим сбросить глубину.
    // Значит тоже LOAD_STORE, если ваше RHI позволяет.

    const RtClearValue dummyColor(0, 0, 0, 0);
    const RtClearValue dummyDepth(1.f, 0);

    Attachment colorAttachment(m_renderContext_->renderFrameContext
                                   ->m_sceneRenderTarget_->m_colorBuffer_,
                               EAttachmentLoadStoreOp::LOAD_STORE,
                               EAttachmentLoadStoreOp::DONTCARE_DONTCARE,
                               dummyColor,
                               EResourceLayout::COLOR_ATTACHMENT,
                               EResourceLayout::COLOR_ATTACHMENT);

    Attachment depthAttachment(m_renderContext_->renderFrameContext
                                   ->m_sceneRenderTarget_->m_depthBuffer_,
                               EAttachmentLoadStoreOp::LOAD_STORE,
                               EAttachmentLoadStoreOp::DONTCARE_DONTCARE,
                               dummyDepth,
                               EResourceLayout::DEPTH_STENCIL_ATTACHMENT,
                               EResourceLayout::DEPTH_STENCIL_ATTACHMENT);

    RenderPassInfo rpass;
    rpass.m_attachments_.push_back(colorAttachment);
    rpass.m_attachments_.push_back(depthAttachment);

    Subpass sp;
    sp.initialize(0,
                  1,
                  EPipelineStageMask::COLOR_ATTACHMENT_OUTPUT_BIT,
                  EPipelineStageMask::FRAGMENT_SHADER_BIT);
    sp.m_outputColorAttachments_.push_back(0);
    sp.m_outputDepthAttachment_ = 1;

    rpass.m_subpasses_.push_back(sp);

    renderPass_ = g_rhi->getOrCreateRenderPass(
        rpass, {0, 0}, {vpDim.width(), vpDim.height()});
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
                                          LifeTimeType::OneFrame,
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

      if (viewShaderBindingInstance_) {
        viewShaderBindingInstance_->free();
      }

      viewShaderBindingInstance_ = g_rhi->createShaderBindingInstance(
          shaderBindingArray, ShaderBindingInstanceType::SingleFrame);
    }
  }

  void prepareDrawCalls() {
    GraphicsPipelineShader localShaders;
    {
      std::lock_guard<std::mutex> lock(m_shadersMutex);
      localShaders = m_shaders;
    }

    auto& registry = m_renderContext_->scene->getEntityRegistry();
    auto  view     = registry.view<Transform, std::shared_ptr<RenderModel>>();

    std::unordered_map<std::shared_ptr<RenderModel>,
                       std::vector<math::Matrix4f<>>>
        modelInstances;

    for (auto entity : view) {
      auto& transform   = view.get<Transform>(entity);
      auto& renderModel = view.get<std::shared_ptr<RenderModel>>(entity);

      math::Matrix4f<> mtx = calculateTransformMatrix(transform);
      modelInstances[renderModel].push_back(mtx);
    }

    for (auto& [modelPtr, matrices] : modelInstances) {
      std::shared_ptr<VertexBuffer> instanceBuf;
      createInstanceBuffer(matrices, instanceBuf);

      for (auto& renderMesh : modelPtr->renderMeshes) {
        DrawData drawData{};
        drawData.renderMesh_     = renderMesh->gpuMesh;
        drawData.instanceBuffer_ = instanceBuf;
        drawData.instanceCount   = static_cast<uint32_t>(matrices.size());

        ShaderBindingInstanceArray bindingArr;
        bindingArr.add(viewShaderBindingInstance_.get());

        ShaderBindingLayoutArray layoutArr;
        for (int i = 0; i < bindingArr.m_numOfData_; ++i) {
          layoutArr.add(bindingArr[i]->m_shaderBindingsLayouts_.get());
        }

        VertexBufferArray vbArray;
        vbArray.add(drawData.renderMesh_->vertexBuffer.get());  // binding=0
        vbArray.add(drawData.instanceBuffer_.get());            // binding=1

        drawData.pipelineStateInfo_
            = g_rhi->createPipelineStateInfo(&m_pipelineStateFixed,
                                             localShaders,
                                             vbArray,
                                             renderPass_,
                                             layoutArr,
                                             nullptr,
                                             0);

        m_drawDataList.push_back(drawData);
      }
    }
  }

  void createInstanceBuffer(const std::vector<math::Matrix4f<>>& mats,
                            std::shared_ptr<VertexBuffer>&       instanceBuf) {
    constexpr int      kMatSize = 16;
    std::vector<float> data;
    data.reserve(mats.size() * kMatSize);

    for (auto& mat : mats) {
      data.insert(data.end(), mat.data(), mat.data() + kMatSize);
    }

    auto stream = std::make_shared<BufferAttributeStream<float>>(
        Name("INSTANCE"),
        EBufferType::Static,
        math::Matrix4f<>::GetDataSize(),
        std::vector<IBufferAttribute::Attribute>{
          {EBufferElementType::FLOAT,                  0, sizeof(float) * 4},
          {EBufferElementType::FLOAT,  sizeof(float) * 4, sizeof(float) * 4},
          {EBufferElementType::FLOAT,  sizeof(float) * 8, sizeof(float) * 4},
          {EBufferElementType::FLOAT, sizeof(float) * 12, sizeof(float) * 4}
    },
        std::move(data));

    auto vsd = std::make_shared<VertexStreamData>();
    vsd->m_streams_.push_back(stream);
    vsd->m_primitiveType_   = EPrimitiveType::TRIANGLES;
    vsd->m_elementCount_    = static_cast<int32_t>(mats.size());
    vsd->m_bindingIndex_    = 6;
    vsd->m_vertexInputRate_ = EVertexInputRate::INSTANCE;

    instanceBuf = g_rhi->createVertexBuffer(vsd);
  }
};

class WireframeStrategy : public IDebugDrawStrategy {
  public:
  bool isExclusive() const override { return true; }

  void initialize(const std::shared_ptr<RenderContext>& renderContext,
                  const EditorRenderParams& editorRenderParams) override {
    m_renderContext_      = renderContext;
    m_editorRenderParams_ = editorRenderParams;
    setupPipelineStates();
    setupRenderPass();
    preparePerFrameResources();
    prepareDrawCalls();
  }

  void execute() override {
    auto commandBuffer
        = m_renderContext_->renderFrameContext->getActiveCommandBuffer();

    g_rhi->transitionLayout(
        commandBuffer,
        m_renderContext_->renderFrameContext->m_sceneRenderTarget_
            ->m_colorBuffer_->getTexture(),
        EResourceLayout::COLOR_ATTACHMENT);

    // auto newLayout =
    // m_renderContext_->renderFrameContext->m_sceneRenderTarget_
    //                          ->m_depthBuffer_->getTexture()
    //                          ->isDepthOnlyFormat()
    //                    ? EResourceLayout::DEPTH_ATTACHMENT
    //                    : EResourceLayout::DEPTH_STENCIL_ATTACHMENT;

    // g_rhi->transitionLayout(
    //     commandBuffer,
    //     m_renderContext_->renderFrameContext->m_sceneRenderTarget_
    //         ->m_depthBuffer_->getTexture(),
    //     newLayout);

    if (renderPass_->beginRenderPass(commandBuffer)) {
      for (const auto& drawData : drawDataList_) {
        drawData.pipelineStateInfo_->bind(commandBuffer);

        ShaderBindingInstanceArray shaderBindingInstanceArray;
        shaderBindingInstanceArray.add(viewShaderBindingInstance_.get());

        ShaderBindingInstanceCombiner shaderBindingInstanceCombiner;
        for (int32_t i = 0; i < shaderBindingInstanceArray.m_numOfData_; ++i) {
          shaderBindingInstanceCombiner.m_descriptorSetHandles_.add(
              shaderBindingInstanceArray[i]->getHandle());
        }
        shaderBindingInstanceCombiner.m_shaderBindingInstanceArray
            = &shaderBindingInstanceArray;

        g_rhi->bindGraphicsShaderBindingInstances(commandBuffer,
                                                  drawData.pipelineStateInfo_,
                                                  shaderBindingInstanceCombiner,
                                                  0);

        const auto& gpuMesh = drawData.renderGeometryMesh_;
        gpuMesh->vertexBuffer->bind(commandBuffer);
        drawData.instanceBuffer_->bind(commandBuffer);

        gpuMesh->indexBuffer->bind(commandBuffer);

        g_rhi->drawElementsInstanced(commandBuffer,
                                     0,
                                     gpuMesh->indexBuffer->getElementCount(),
                                     drawData.instanceCount);
      }

      renderPass_->endRenderPass();
    }
  }

  void finalize() override {
    drawDataList_.clear();

    shaders_.m_vertexShader_.reset();
    shaders_.m_geometryShader_.reset();
    shaders_.m_pixelShader_.reset();

    // viewShaderBindingInstance_.reset();
  }

  private:
  void setupPipelineStates() {
    auto viewportDimension = m_editorRenderParams_.editorViewportDimension;
    // auto viewportDimension = m_renderContext_->viewportDimension;

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

    // auto depthStencilState = TDepthStencilStateInfo<true,
    //                                                 true,
    //                                                 ECompareOp::LESS,
    //                                                 false,
    //                                                 false,
    //                                                 0.0f,
    //                                                 1.0f>::s_create();

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
        // depthStencilState,
        nullptr,
        blendingState,
        Viewport(0.0f,
                 0.0f,
                 static_cast<float>(viewportDimension.width()),
                 static_cast<float>(viewportDimension.height())),
        Scissor(0, 0, viewportDimension.width(), viewportDimension.height()),
        false /*gOptions.UseVRS*/);

    // static bool isShaderInitialized = false;

    // TODO: use config in future
    // if (!isShaderInitialized) {
    ShaderInfo vertexShaderInfo;
    vertexShaderInfo.setName(NameStatic("WireframeVS"));
    vertexShaderInfo.setShaderFilepath(
        NameStatic("assets/shaders/debug/wireframe/shader_instancing.vs.hlsl"));
    vertexShaderInfo.setShaderType(EShaderAccessStageFlag::VERTEX);

    ShaderInfo pixelShaderInfo;
    pixelShaderInfo.setName(NameStatic("WireframePS"));
    pixelShaderInfo.setShaderFilepath(
        NameStatic("assets/shaders/debug/wireframe/shader.ps.hlsl"));
    pixelShaderInfo.setShaderType(EShaderAccessStageFlag::FRAGMENT);

    auto newVertexShader = g_rhi->createShader(vertexShaderInfo);
    auto newPixelShader  = g_rhi->createShader(pixelShaderInfo);

    {
      std::lock_guard<std::mutex> lock(shaders_mutex_);
      shaders_.m_vertexShader_ = newVertexShader;
      shaders_.m_pixelShader_  = newPixelShader;
    }

    // hot reload for shaders
    //{
    //  auto fileModificationHandler = [this](const wtr::event& e) {
    //    std::lock_guard<std::mutex> lock(shaders_mutex_);

    //    const std::string vertexShaderPath
    //        = "assets/shaders/debug/wireframe/shader_instancing.vs.hlsl";
    //    const std::string pixelShaderPath
    //        = "assets/shaders/debug/wireframe/shader.ps.hlsl";

    //    const std::string absolutePath = e.path_name.generic_string();

    //    if (absolutePath.find(vertexShaderPath) != std::string::npos) {
    //      ShaderInfo vertexShaderInfo;
    //      vertexShaderInfo.setName(NameStatic("WireframeVS"));
    //      vertexShaderInfo.setShaderFilepath(
    //          NameStatic(e.path_name.string()));
    //      vertexShaderInfo.setShaderType(EShaderAccessStageFlag::VERTEX);
    //      shaders_.m_vertexShader_ = g_rhi->createShader(vertexShaderInfo);
    //    }

    //    if (absolutePath.find(pixelShaderPath) != std::string::npos) {
    //      ShaderInfo pixelShaderInfo;
    //      pixelShaderInfo.setName(NameStatic("WireframePS"));
    //      pixelShaderInfo.setShaderFilepath(NameStatic(e.path_name.string()));
    //      pixelShaderInfo.setShaderType(EShaderAccessStageFlag::FRAGMENT);
    //      shaders_.m_pixelShader_ = g_rhi->createShader(pixelShaderInfo);
    //    }
    //  };

    //  auto hotReloadManager = ServiceLocator::s_get<HotReloadManager>();
    //   hotReloadManager->watchFileModifications(
    //       PathManager::s_getShaderPath() / "debug/wireframe",
    //       fileModificationHandler);
    //}
    // isShaderInitialized = true;
    //}
  }

  void setupRenderPass() {
    auto viewportDimension = m_editorRenderParams_.editorViewportDimension;
    // auto viewportDimension = m_renderContext_->viewportDimension;

    const RtClearValue clearColor = RtClearValue(0.0f, 0.0f, 0.0f, 1.0f);
    // const RtClearValue clearDepth = RtClearValue(1.0f, 0);

    Attachment colorAttachment(m_renderContext_->renderFrameContext
                                   ->m_sceneRenderTarget_->m_colorBuffer_,
                               EAttachmentLoadStoreOp::CLEAR_STORE,
                               EAttachmentLoadStoreOp::DONTCARE_DONTCARE,
                               clearColor,
                               EResourceLayout::UNDEFINED,
                               EResourceLayout::COLOR_ATTACHMENT);

    // Attachment depthAttachment(m_renderContext_->renderFrameContext
    //                                ->m_sceneRenderTarget_->m_depthBuffer_,
    //                            EAttachmentLoadStoreOp::CLEAR_STORE,
    //                            EAttachmentLoadStoreOp::DONTCARE_DONTCARE,
    //                            clearDepth,
    //                            EResourceLayout::UNDEFINED,
    //                            EResourceLayout::DEPTH_STENCIL_ATTACHMENT);

    RenderPassInfo renderPassInfo;
    renderPassInfo.m_attachments_.push_back(colorAttachment);
    // renderPassInfo.m_attachments_.push_back(depthAttachment);

    Subpass subpass;
    subpass.initialize(0, /*sourceSubpassIndex*/
                       1, /*destSubpassIndex*/
                       EPipelineStageMask::COLOR_ATTACHMENT_OUTPUT_BIT,
                       EPipelineStageMask::FRAGMENT_SHADER_BIT);

    subpass.m_outputColorAttachments_.push_back(0);
    // subpass.m_outputDepthAttachment_ = 1;

    renderPassInfo.m_subpasses_.push_back(subpass);

    renderPass_ = g_rhi->getOrCreateRenderPass(
        renderPassInfo,
        {0, 0},
        {viewportDimension.width(), viewportDimension.height()});
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
                                          LifeTimeType::OneFrame,
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

      if (viewShaderBindingInstance_) {
        viewShaderBindingInstance_->free();
      }

      viewShaderBindingInstance_ = g_rhi->createShaderBindingInstance(
          shaderBindingArray, ShaderBindingInstanceType::SingleFrame);
    }
  }

  void prepareDrawCalls() {
    GraphicsPipelineShader currentShaders;
    {
      std::lock_guard<std::mutex> lock(shaders_mutex_);
      currentShaders = shaders_;
    }

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
        drawData.instanceBuffer_     = instanceBuffer;
        drawData.instanceCount = static_cast<uint32_t>(modelMatrices.size());

        ShaderBindingInstanceArray shaderBindingInstanceArray;
        shaderBindingInstanceArray.add(viewShaderBindingInstance_.get());

        ShaderBindingLayoutArray shaderBindingLayoutArray;
        for (int32_t i = 0; i < shaderBindingInstanceArray.m_numOfData_; ++i) {
          shaderBindingLayoutArray.add(
              shaderBindingInstanceArray[i]->m_shaderBindingsLayouts_.get());
        }

        VertexBufferArray vertexBufferArray;
        vertexBufferArray.add(
            drawData.renderGeometryMesh_->vertexBuffer.get());  // Binding 0
        vertexBufferArray.add(drawData.instanceBuffer_.get());  // Binding 1

        // TODO: investigate that (currently seems unoptimized to create
        // Pipeline state per each drawCall (lots of state switching))
        drawData.pipelineStateInfo_
            = g_rhi->createPipelineStateInfo(&pipelineStateFixed_,
                                             currentShaders,
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

  std::shared_ptr<RenderContext> m_renderContext_;
  EditorRenderParams             m_editorRenderParams_;

  struct DrawData {
    PipelineStateInfo*                  pipelineStateInfo_ = nullptr;
    std::shared_ptr<RenderGeometryMesh> renderGeometryMesh_;
    std::shared_ptr<VertexBuffer>       instanceBuffer_;
    uint32_t                            instanceCount = 0;

    ~DrawData() {
      // if (materialShaderBindingInstance_) {
      //   materialShaderBindingInstance_->free();
      // }
    }
  };

  std::vector<DrawData> drawDataList_;

  PipelineStateFixedInfo pipelineStateFixed_;

  RenderPass* renderPass_ = nullptr;

  GraphicsPipelineShader shaders_;
  std::mutex             shaders_mutex_;

  std::shared_ptr<IUniformBufferBlock>   viewUniformBuffer_;
  std::shared_ptr<ShaderBindingInstance> viewShaderBindingInstance_;
};

class DebugPass : public RenderStage {
  public:
  DebugPass() = default;

  void setStrategy(std::unique_ptr<IDebugDrawStrategy> strategy) {
    if (m_strategy_) {
      m_strategy_->finalize();
    }
    m_strategy_ = std::move(strategy);
  }

  void initialize(const std::shared_ptr<RenderContext>& renderContext,
                  const EditorRenderParams& editorRenderParams) override {
    m_renderContext_ = renderContext;
    if (m_strategy_) {
      m_strategy_->initialize(renderContext, editorRenderParams);
    }
  }

  void execute() override {
    if (m_strategy_) {
      m_strategy_->execute();
    }
  }

  void finalize() override {
    if (m_strategy_) {
      m_strategy_->finalize();
    }
  }

  bool isExclusiveMode() const {
    // TODO: add logger (if strategy is not set)
    return m_strategy_ ? m_strategy_->isExclusive() : false;
  }

  private:
  // TODO: m_renderContext_ not used
  std::shared_ptr<RenderContext>      m_renderContext_;
  std::unique_ptr<IDebugDrawStrategy> m_strategy_;
};

class Renderer {
  public:
  Renderer() = default;

  void renderFrame(const std::shared_ptr<RenderContext>& renderContext,
                   const EditorRenderParams&             editorRenderParams) {
    initialize(renderContext, editorRenderParams);

    std::unique_ptr<IDebugDrawStrategy> debugStrategy = nullptr;

    bool needBasePass = true;

    switch (editorRenderParams.renderMode) {
      case RenderMode::Solid:
        needBasePass = true;
        break;
      case RenderMode::Wireframe:
        debugStrategy = std::make_unique<WireframeStrategy>();
        break;
      case RenderMode::NormalMapVisualization:
        debugStrategy = std::make_unique<NormalMapVisualizationStrategy>();
        break;
      case RenderMode::VertexNormalVisualization:
        debugStrategy = std::make_unique<VertexNormalVisualizationStrategy>();
        break;
      case RenderMode::ShaderOverdraw:
        debugStrategy = std::make_unique<ShaderOverdrawStrategy>();
        break;
    }

    if (debugStrategy) {
      debugStrategy->initialize(renderContext, editorRenderParams);
    }

    debugPass_->setStrategy(std::move(debugStrategy));

    if (debugPass_->isExclusiveMode()) {
      needBasePass = false;
    }

    if (needBasePass) {
      basePass_->initialize(renderContext, editorRenderParams);
      basePass_->execute();
    }

    debugPass_->execute();

    // switch (editorRenderParams.renderMode) {
    //   case RenderMode::Solid:
    //     basePass_->execute();
    //     break;
    //   case RenderMode::Wireframe:
    //     // Or debug pass
    //     // wireframePass_->execute();
    //     break;
    //   case RenderMode::NormalMapVisualization:
    //     // Or debug pass
    //     // normalMapPass_->execute();
    //     break;
    //   case RenderMode::VertexNormalVisualization:
    //     // Or debug pass
    //     // vertexNormalPass_->execute();
    //     break;
    //   case RenderMode::ShaderOverdraw:
    //     // Or debug pass
    //     // shadowOverdrawPass_->execute();
    //     break;
    //   default:
    //     assert(0);
    //     // TODO: log error
    //     break;
    // }

    if (editorRenderParams.postProcessMode == PostProcessMode::Grayscale
        || editorRenderParams.postProcessMode
               == PostProcessMode::ColorInversion) {
      // postProcessPass_->execute();
    }

    // finalPass_->execute();

    finalize();
  }

  private:
  // TODO: temporal solution to pass renderContext and editorRenderParams on
  // each frame (not all passes need it)
  void initialize(const std::shared_ptr<RenderContext>& renderContext,
                  const EditorRenderParams&             editorRenderParams) {
    // m_renderContext_ = renderContext;
    if (basePass_ == nullptr) {
      basePass_ = std::make_unique<BasePass>();
    }

    if (debugPass_ == nullptr) {
      debugPass_ = std::make_unique<DebugPass>();
    }
    debugPass_->initialize(renderContext, editorRenderParams);

    // if (finalPass_ == nullptr) {
    //   finalPass_ = std::make_unique<FinalPass>();
    // }
    // finalPass_->initialize(renderContext, editorRenderParams);
  }

  void finalize() {
    basePass_->finalize();
    debugPass_->finalize();
    // finalPass_->finalize();
  }

  // TODO: currently not the best architectural approach, consider changing
  // std::shared_ptr<RenderContext> m_renderContext_;
  std::unique_ptr<BasePass>  basePass_;
  std::unique_ptr<DebugPass> debugPass_;
  // std::unique_ptr<FinalPass> finalPass_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDERER_H
