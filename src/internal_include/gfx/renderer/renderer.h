#ifndef GAME_ENGINE_RENDERER_H
#define GAME_ENGINE_RENDERER_H

#include "gfx/renderer/draw_command.h"
#include "gfx/renderer/material.h"
#include "gfx/rhi/feature_switch.h"
#include "gfx/rhi/mem_stack_allocator.h"
#include "gfx/rhi/rhi.h"
#include "gfx/scene/camera.h"
#include "gfx/scene/object.h"
#include "gfx/scene/render_object.h"

#include <future>
#include <memory>
#include <vector>

// TODO:
// - implement abstraction layer for rhi (rendering api)
// - add debug pass for rendering (bounding boxes, light sources, normal vectors
// etc.)
// - shadow pass for the future

namespace game_engine {

#define ASYNC_WITH_SETUP           0
#define PARALLELFOR_WITH_PASSSETUP 0

struct SimplePushConstant {
  math::Vector4Df m_color_ = math::Vector4Df(1.0f, 1.0f, 1.0f, 1.0f);

  // No need for now
  // bool ShowVRSArea = false;
  // bool Padding[3];

  // bool ShowGrid = true;
  // bool Padding2[3];
};

class Renderer {
  public:
  Renderer() = default;

  Renderer(const std::shared_ptr<RenderFrameContext>& renderFrameContextPtr,
           const View&                                view,
           std::shared_ptr<Window>                    window)
      : m_renderFrameContextPtr_(renderFrameContextPtr)
      , m_view_(view)
      , m_window_(std::move(window)) {}

  virtual ~Renderer() {}

  virtual void setup() {
    m_frameIndex_            = g_rhi->getCurrentFrameIndex();
    m_useForwardRenderer_ = m_renderFrameContextPtr_->m_useForwardRenderer_;

    // view.ShadowCasterLights.reserve(view.Lights.size());

    // for (int32_t i = 0; i < view.Lights.size(); ++i) {
    //   ViewLight& viewLight = view.Lights[i];

    //  if (viewLight.Light) {
    //    if (viewLight.Light->IsShadowCaster) {
    //      viewLight.ShadowMapPtr
    //          = RenderFrameContextPtr->SceneRenderTargetPtr->GetShadowMap(
    //              viewLight.Light);
    //    }

    //    viewLight.m_shaderBindingInstance
    //        = viewLight.Light->PrepareShaderBindingInstance(
    //            viewLight.ShadowMapPtr ? viewLight.ShadowMapPtr->getTexture()
    //                                   : nullptr);

    //    if (viewLight.Light->IsShadowCaster) {
    //      ViewLight NewViewLight = viewLight;
    //      NewViewLight.m_shaderBindingInstance
    //          = viewLight.Light->PrepareShaderBindingInstance(nullptr);
    //      view.ShadowCasterLights.push_back(NewViewLight);
    //    }
    //  }
    //}

#if ASYNC_WITH_SETUP
    ShadowPassSetupCompleteEvent
        = std::async(std::launch::async, &Renderer::SetupShadowPass, this);
#else
    // TODO: shadow pass setup before base pass
    Renderer::setupBasePass();
#endif
  }

  void setupBasePass() {
    const auto& screenWidth  = m_window_->getSize().width();
    const auto& screenHeight = m_window_->getSize().height();

    // Prepare basepass pipeline
    // TODO: refactor code to create RasterizationState
    RasterizationStateInfo* rasterizationState = nullptr;
    switch (g_rhi->getSelectedMSAASamples()) {
      case EMSAASamples::COUNT_1:
        rasterizationState = TRasterizationStateInfo<EPolygonMode::FILL,
                                                     ECullMode::BACK,
                                                     EFrontFace::CCW,
                                                     false,
                                                     0.0f,
                                                     0.0f,
                                                     0.0f,
                                                     1.0f,
                                                     false,
                                                     false,
                                                     (EMSAASamples)1,
                                                     true,
                                                     0.2f,
                                                     false,
                                                     false>::s_create();
        break;
      case EMSAASamples::COUNT_2:
        rasterizationState = TRasterizationStateInfo<EPolygonMode::FILL,
                                                     ECullMode::BACK,
                                                     EFrontFace::CCW,
                                                     false,
                                                     0.0f,
                                                     0.0f,
                                                     0.0f,
                                                     1.0f,
                                                     false,
                                                     false,
                                                     (EMSAASamples)2,
                                                     true,
                                                     0.2f,
                                                     false,
                                                     false>::s_create();
        break;
      case EMSAASamples::COUNT_4:
        rasterizationState = TRasterizationStateInfo<EPolygonMode::FILL,
                                                     ECullMode::BACK,
                                                     EFrontFace::CCW,
                                                     false,
                                                     0.0f,
                                                     0.0f,
                                                     0.0f,
                                                     1.0f,
                                                     false,
                                                     false,
                                                     (EMSAASamples)4,
                                                     true,
                                                     0.2f,
                                                     false,
                                                     false>::s_create();
        break;
      case EMSAASamples::COUNT_8:
        rasterizationState = TRasterizationStateInfo<EPolygonMode::FILL,
                                                     ECullMode::BACK,
                                                     EFrontFace::CCW,
                                                     false,
                                                     0.0f,
                                                     0.0f,
                                                     0.0f,
                                                     1.0f,
                                                     false,
                                                     false,
                                                     (EMSAASamples)8,
                                                     true,
                                                     0.2f,
                                                     false,
                                                     false>::s_create();
        break;
      default:
        assert(0);
        break;
    }

    auto depthStencilState = TDepthStencilStateInfo<true,
                                                    true,
                                                    ECompareOp::LESS,
                                                    false,
                                                    false,
                                                    0.0f,
                                                    1.0f>::s_create();
    auto blendingState     = TBlendingStateInfo<false,
                                            EBlendFactor::ONE,
                                            EBlendFactor::ZERO,
                                            EBlendOp::ADD,
                                            EBlendFactor::ONE,
                                            EBlendFactor::ZERO,
                                            EBlendOp::ADD,
                                            EColorMask::ALL>::s_create();

    PipelineStateFixedInfo basePassPipelineStateFixed = PipelineStateFixedInfo(
        rasterizationState,
        depthStencilState,
        blendingState,
        Viewport(0.0f, 0.0f, (float)screenWidth, (float)screenHeight),
        Scissor(0, 0, screenWidth, screenWidth),
        false /*gOptions.UseVRS*/);

    // TODO: remove this code (not needed)
    // -----------------------------------------------------
    // auto TranslucentBlendingState
    //    = TBlendingStateInfo<true,
    //                         EBlendFactor::SRC_ALPHA,
    //                         EBlendFactor::ONE_MINUS_SRC_ALPHA,
    //                         EBlendOp::ADD,
    //                         EBlendFactor::ONE,
    //                         EBlendFactor::ZERO,
    //                         EBlendOp::ADD,
    //                         EColorMask::ALL>::s_create();
    // PipelineStateFixedInfo TranslucentPassPipelineStateFixed
    //    = PipelineStateFixedInfo(
    //        rasterizationState,
    //        depthStencilState,
    //        TranslucentBlendingState,
    //        Viewport(0.0f, 0.0f, (float)screenWidth, (float)screenHeight),
    //        Scissor(0, 0, screenWidth, screenHeight),
    //        gOptions.UseVRS);
    // -----------------------------------------------------

    const RTClearValue kClearColor = RTClearValue(0.0f, 0.0f, 0.0f, 1.0f);
    const RTClearValue kClearDepth = RTClearValue(1.0f, 0);

    Attachment depth = Attachment(
        m_renderFrameContextPtr_->m_sceneRenderTargetPtr_->m_depthPtr_,
        EAttachmentLoadStoreOp::CLEAR_STORE,
        EAttachmentLoadStoreOp::CLEAR_STORE,
        kClearDepth,
        EResourceLayout::UNDEFINED,
        EResourceLayout::DEPTH_STENCIL_ATTACHMENT);
    Attachment resolve;

    if (m_useForwardRenderer_) {
      if ((int32_t)g_rhi->getSelectedMSAASamples() > 1) {
        resolve = Attachment(
            m_renderFrameContextPtr_->m_sceneRenderTargetPtr_->m_resolvePtr_,
            EAttachmentLoadStoreOp::DONTCARE_STORE,
            EAttachmentLoadStoreOp::DONTCARE_DONTCARE,
            kClearColor,
            EResourceLayout::UNDEFINED,
            EResourceLayout::COLOR_ATTACHMENT,
            true);
      }
    }

    // Setup attachment
    RenderPassInfo renderPassInfo;
    if (!m_useForwardRenderer_) {
      // TODO: not used for now
      /*for (int32_t i = 0;
         i < std::size(RenderFrameContextPtr->SceneRenderTargetPtr->GBuffer);
         ++i) {
      Attachment color = Attachment(
          RenderFrameContextPtr->SceneRenderTargetPtr->GBuffer[i],
          EAttachmentLoadStoreOp::CLEAR_STORE,
          EAttachmentLoadStoreOp::DONTCARE_DONTCARE,
          kClearColor,
          EResourceLayout::UNDEFINED,
          EResourceLayout::COLOR_ATTACHMENT);
      renderPassInfo.Attachments.push_back(color);
    }*/
    }

    const int32_t kLightPassAttachmentIndex
        = (int32_t)renderPassInfo.m_attachments_.size();

    // TODO: pay attention to UseSubpass
    if (m_useForwardRenderer_ /*|| gOptions.UseSubpass*/) {
      Attachment color
          = Attachment(/*RenderFrameContextPtr->SceneRenderTargetPtr->ColorPtr,*/
                       m_renderFrameContextPtr_->m_sceneRenderTargetPtr_
                           ->m_finalColorPtr_,
                       EAttachmentLoadStoreOp::CLEAR_STORE,
                       EAttachmentLoadStoreOp::DONTCARE_DONTCARE,
                       kClearColor,
                       EResourceLayout::UNDEFINED,
                       // EResourceLayout::COLOR_ATTACHMENT);
                       EResourceLayout::PRESENT_SRC);
      renderPassInfo.m_attachments_.push_back(color);
    }

    const int32_t kDepthAttachmentIndex
        = (int32_t)renderPassInfo.m_attachments_.size();
    renderPassInfo.m_attachments_.push_back(depth);

    const int32_t kResolveAttachemntIndex
        = (int32_t)renderPassInfo.m_attachments_.size();
    if (m_useForwardRenderer_) {
      if ((int32_t)g_rhi->getSelectedMSAASamples() > 1) {
        renderPassInfo.m_attachments_.push_back(resolve);
      }
    }

    //////////////////////////////////////////////////////////////////////////
    // Setup subpass of basePass
    {
      // First subpass, Geometry pass
      Subpass subpass;
      subpass.initialize(0,
                         1,
                         EPipelineStageMask::COLOR_ATTACHMENT_OUTPUT_BIT,
                         EPipelineStageMask::FRAGMENT_SHADER_BIT);

      if (m_useForwardRenderer_) {
        subpass.m_outputColorAttachments_.push_back(0);
      } else {
        const int32_t kGBufferCount = kLightPassAttachmentIndex;
        for (int32_t i = 0; i < kGBufferCount; ++i) {
          subpass.m_outputColorAttachments_.push_back(i);
        }
      }

      subpass.m_outputDepthAttachment_ = kDepthAttachmentIndex;
      if (m_useForwardRenderer_) {
        if ((int32_t)g_rhi->getSelectedMSAASamples() > 1) {
          subpass.m_outputResolveAttachment_ = kResolveAttachemntIndex;
        }
      }
      renderPassInfo.m_subpasses_.push_back(subpass);
    }
    // TODO: not used for now
    // if (!useForwardRenderer && gOptions.UseSubpass) {
    //   // Second subpass, Lighting pass
    //   Subpass subpass;
    //   subpass.initialize(1,
    //                      2,
    //                      EPipelineStageMask::COLOR_ATTACHMENT_OUTPUT_BIT,
    //                      EPipelineStageMask::FRAGMENT_SHADER_BIT);

    //  const int32_t kGBufferCount = kLightPassAttachmentIndex;
    //  for (int32_t i = 0; i < kGBufferCount; ++i) {
    //    subpass.InputAttachments.push_back(i);
    //  }

    //  subpass.OutputColorAttachments.push_back(kLightPassAttachmentIndex);
    //  subpass.OutputDepthAttachment = kDepthAttachmentIndex;

    //  if ((int32_t)g_rhi->getSelectedMSAASamples() > 1) {
    //    subpass.OutputResolveAttachment = kResolveAttachemntIndex;
    //  }

    //  renderPassInfo.Subpasses.push_back(subpass);
    //}
    //////////////////////////////////////////////////////////////////////////
    m_baseRenderPass_ = (RenderPass*)g_rhi->getOrCreateRenderPass(
        renderPassInfo, {0, 0}, {screenWidth, screenHeight});

    auto getOrCreateShaderFunc = [useForwardRenderer
                                  = this->m_useForwardRenderer_](
                                     const RenderObject* renderObject) {
      GraphicsPipelineShader shaders;
      ShaderInfo             shaderInfo;

      if (renderObject->hasInstancing()) {
        assert(useForwardRenderer);

        ShaderInfo shaderInfo;
        shaderInfo.setName(NameStatic("default_instancing_testVS"));
        shaderInfo.setShaderFilepath(NameStatic(
            "assets/shaders/forward_rendering/shader_instancing.vs.hlsl"));
        shaderInfo.setShaderType(EShaderAccessStageFlag::VERTEX);
        shaders.m_vertexShader_ = g_rhi->createShader(shaderInfo);

        shaderInfo.setName(NameStatic("default_instancing_testPS"));
        shaderInfo.setShaderFilepath(
            NameStatic("assets/shaders/forward_rendering/shader.ps.hlsl"));
        shaderInfo.setShaderType(EShaderAccessStageFlag::FRAGMENT);
        shaders.m_pixelShader_ = g_rhi->createShader(shaderInfo);
        return shaders;
      }

      if (useForwardRenderer) {
        shaderInfo.setName(NameStatic("default_testVS"));
        shaderInfo.setShaderFilepath(
            // NameStatic("assets/shaders/forward_rendering/shader.vs.hlsl"));
            NameStatic("assets/shaders/demo/first_triangle.vs.hlsl"));
        shaderInfo.setShaderType(EShaderAccessStageFlag::VERTEX);
        shaders.m_vertexShader_ = g_rhi->createShader(shaderInfo);

        ShaderForwardPixelShader::ShaderPermutation shaderPermutation;
        /*shaderPermutation
            .setIndex<ShaderForwardPixelShader::USE_VARIABLE_SHADING_RATE>(
                USE_VARIABLE_SHADING_RATE_TIER2);*/
        shaderPermutation.setIndex<ShaderForwardPixelShader::USE_REVERSEZ>(
            USE_REVERSEZ_PERSPECTIVE_SHADOW);
        shaders.m_pixelShader_
            = ShaderForwardPixelShader::CreateShader(shaderPermutation);
      } else {
        const bool kIsUseSphericalMap
            = renderObject->m_materialPtr_
           && renderObject->m_materialPtr_->isUseSphericalMap();
        const bool kHasAlbedoTexture
            = renderObject->m_materialPtr_
           && renderObject->m_materialPtr_->hasAlbedoTexture();
        const bool kIsUseSRGBAlbedoTexture
            = renderObject->m_materialPtr_
           && renderObject->m_materialPtr_->isUseSRGBAlbedoTexture();
        const bool kHasVertexColor
            = renderObject->m_geometryDataPtr_
           && renderObject->m_geometryDataPtr_->hasVertexColor();
        const bool kHasVertexBiTangent
            = renderObject->m_geometryDataPtr_
           && renderObject->m_geometryDataPtr_->hasVertexBiTangent();

        // TODO: consider whether postfix 'Vs' is clearly describes 'vertex'
        ShaderGBufferVertexShader::ShaderPermutation shaderPermutationVs;
        shaderPermutationVs
            .setIndex<ShaderGBufferVertexShader::USE_VERTEX_COLOR>(
                kHasVertexColor);
        shaderPermutationVs
            .setIndex<ShaderGBufferVertexShader::USE_VERTEX_BITANGENT>(
                kHasVertexBiTangent);
        shaderPermutationVs
            .setIndex<ShaderGBufferVertexShader::USE_ALBEDO_TEXTURE>(
                kHasAlbedoTexture);
        shaderPermutationVs
            .setIndex<ShaderGBufferVertexShader::USE_SPHERICAL_MAP>(
                kIsUseSphericalMap);
        shaders.m_vertexShader_
            = ShaderGBufferVertexShader::CreateShader(shaderPermutationVs);

        ShaderGBufferPixelShader::ShaderPermutation shaderPermutationPs;
        shaderPermutationPs
            .setIndex<ShaderGBufferPixelShader::USE_VERTEX_COLOR>(
                kHasVertexColor);
        shaderPermutationPs
            .setIndex<ShaderGBufferPixelShader::USE_ALBEDO_TEXTURE>(
                kHasAlbedoTexture);
        shaderPermutationPs
            .setIndex<ShaderGBufferPixelShader::USE_SRGB_ALBEDO_TEXTURE>(
                kIsUseSRGBAlbedoTexture);
        // shaderPermutationPs
        //     .setIndex<ShaderGBufferPixelShader::USE_VARIABLE_SHADING_RATE>(
        //         USE_VARIABLE_SHADING_RATE_TIER2);
        shaderPermutationPs.setIndex<ShaderGBufferPixelShader::USE_PBR>(
            ENABLE_PBR);
        shaders.m_pixelShader_
            = ShaderGBufferPixelShader::CreateShader(shaderPermutationPs);
      }
      return shaders;
    };

    // TODO: remove (not used)
    // ShaderInfo             shaderInfo;
    // GraphicsPipelineShader TranslucentPassShader;
    //{
    //  shaderInfo.setName(NameStatic("default_testVS"));
    //  shaderInfo.setShaderFilepath(
    //      NameStatic("assets/shaders/deferred_rendering/gbuffer.vs.hlsl"));
    //      //NameStatic("assets/shaders/demo/first_triangle.vs.hlsl"));
    //  shaderInfo.setShaderType(EShaderAccessStageFlag::VERTEX);
    //  TranslucentPassShader.m_vertexShader_ = g_rhi->createShader(shaderInfo);

    //  ShaderGBufferPixelShader::ShaderPermutation ShaderPermutation;
    //  shaderPermutation.setIndex<ShaderGBufferPixelShader::USE_VERTEX_COLOR>(
    //      0);
    //  shaderPermutation.setIndex<ShaderGBufferPixelShader::USE_ALBEDO_TEXTURE>(
    //      1);
    //  //shaderPermutation
    //  //    .setIndex<ShaderGBufferPixelShader::USE_VARIABLE_SHADING_RATE>(
    //  //        USE_VARIABLE_SHADING_RATE_TIER2);
    //  TranslucentPassShader.m_pixelShader_
    //      = ShaderGBufferPixelShader::CreateShader(shaderPermutation);
    //}

    SimplePushConstant simplePushConstantData;
    // simplePushConstantData.ShowVRSArea = gOptions.ShowVRSArea;
    // simplePushConstantData.ShowGrid    = gOptions.ShowGrid;

    PushConstant* simplePushConstant
        = new (MemStack::get()->alloc<PushConstant>()) PushConstant(
            simplePushConstantData, EShaderAccessStageFlag::FRAGMENT);

#if PARALLELFOR_WITH_PASSSETUP
    BasePasses.resize(Object::GetStaticRenderObject().size());
    ParallelFor::ParallelForWithTaskPerThread(
        MaxPassSetupTaskPerThreadCount,
        Object::GetStaticRenderObject(),
        [&](size_t index, RenderObject* renderObject) {
          Material* material = nullptr;
          if (renderObject->MaterialPtr) {
            material = renderObject->MaterialPtr.get();
          } else {
            if (GDefaultMaterial) {
              material = GDefaultMaterial.get();
            }
          }

          new (&BasePasses[index])
              DrawCommand(RenderFrameContextPtr,
                          &view,
                          renderObject,
                          BaseRenderPass,
                          getOrCreateShaderFunc(renderObject),
                          &basePassPipelineStateFixed,
                          material,
                          {},
                          simplePushConstant);
          BasePasses[index].prepareToDraw(false);
        });
#else
    m_basePasses_.resize(Object::s_getStaticRenderObject().size());
    int32_t i = 0;
    for (auto iter : Object::s_getStaticRenderObject()) {
      Material* material = nullptr;
      if (iter->m_materialPtr_) {
        material = iter->m_materialPtr_.get();
      } else {
        if (g_defaultMaterial) {
          material = g_defaultMaterial.get();
        }
      }

      new (&m_basePasses_[i]) DrawCommand(m_renderFrameContextPtr_,
                                       &m_view_,
                                       iter,
                                       m_baseRenderPass_,
                                       getOrCreateShaderFunc(iter),
                                       &basePassPipelineStateFixed,
                                       material,
                                       {},
                                       simplePushConstant);
      m_basePasses_[i].prepareToDraw(false);
      ++i;
    }
#endif
  }

  virtual void basePass() {
    const auto& screenWidth  = m_window_->getSize().width();
    const auto& screenHeight = m_window_->getSize().height();

    if (m_basePassSetupCompleteEvent_.valid()) {
      m_basePassSetupCompleteEvent_.wait();
    }

    {
      if (m_useForwardRenderer_) {
        g_rhi->transitionLayout(
            m_renderFrameContextPtr_->getActiveCommandBuffer(),
            // RenderFrameContextPtr->SceneRenderTargetPtr->ColorPtr->getTexture(),
            m_renderFrameContextPtr_->m_sceneRenderTargetPtr_->m_finalColorPtr_
                ->getTexture(),
            EResourceLayout::COLOR_ATTACHMENT);
        // TODO: worked for Vulkan
        // EResourceLayout::PRESENT_SRC);
      } else {
        // TODO: not used for now
        /*for (int32_t i = 0;
           i
           < std::size(RenderFrameContextPtr->SceneRenderTargetPtr->GBuffer);
           ++i) {
        g_rhi->transitionLayout(
            RenderFrameContextPtr->getActiveCommandBuffer(),
            RenderFrameContextPtr->SceneRenderTargetPtr->GBuffer[i]
                ->getTexture(),
            EResourceLayout::COLOR_ATTACHMENT);
      }*/
      }

      {
        auto newLayout
            = m_renderFrameContextPtr_->m_sceneRenderTargetPtr_->m_depthPtr_
                      ->getTexture()
                      ->isDepthOnlyFormat()
                ? EResourceLayout::DEPTH_ATTACHMENT
                : EResourceLayout::DEPTH_STENCIL_ATTACHMENT;
        g_rhi->transitionLayout(
            m_renderFrameContextPtr_->getActiveCommandBuffer(),
            m_renderFrameContextPtr_->m_sceneRenderTargetPtr_->m_depthPtr_
                ->getTexture(),
            newLayout);
      }

      // BasepassOcclusionTest.BeginQuery(RenderFrameContextPtr->getActiveCommandBuffer());
      if (m_baseRenderPass_
          && m_baseRenderPass_->beginRenderPass(
              m_renderFrameContextPtr_->getActiveCommandBuffer())) {
        // Draw G-m_buffer : subpass 0
        for (const auto& command : m_basePasses_) {
          command.draw();
        }

        // TODO: not used for now
        // Draw Light : subpass 1
        /*if (!useForwardRenderer && gOptions.UseSubpass) {
          g_rhi->nextSubpass(
              RenderFrameContextPtr->getActiveCommandBuffer());
          DeferredLightPass_TodoRefactoring(BaseRenderPass);
        }*/

        m_baseRenderPass_->endRenderPass();
      }
    }
  }

  virtual void render() {
    setup();

    basePass();

    // Queue submit to prepare scenecolor RT for postprocess
    // if (gOptions.QueueSubmitAfterBasePass) {
    // RenderFrameContextPtr->getActiveCommandBuffer()->end();
    m_renderFrameContextPtr_->submitCurrentActiveCommandBuffer(
        RenderFrameContext::BasePass);
    // RenderFrameContextPtr->getActiveCommandBuffer()->begin();
    //}

    g_rhi->incrementFrameNumber();
  }

  bool m_useForwardRenderer_ = true;

  std::shared_ptr<RenderFrameContext> m_renderFrameContextPtr_;
  View                                m_view_;

  std::future<void> m_shadowPassSetupCompleteEvent_;
  std::future<void> m_basePassSetupCompleteEvent_;

  std::vector<DrawCommand> m_basePasses_;

  RenderPass* m_baseRenderPass_ = nullptr;

  // Current FrameIndex
  int32_t m_frameIndex_ = 0;

  // Thread per task for PassSetup
  const int32_t kMaxPassSetupTaskPerThreadCount = 100;

  std::shared_ptr<Window> m_window_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDERER_H