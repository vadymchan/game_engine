#ifndef GAME_ENGINE_RENDERER_H
#define GAME_ENGINE_RENDERER_H

// #include "gfx/renderer/draw_command.h"
// #include "gfx/renderer/material.h"
// #include "gfx/rhi/feature_switch.h"
// #include "gfx/rhi/mem_stack_allocator.h"
// #include "gfx/rhi/rhi.h"
// #include "gfx/scene/camera_old.h"
// #include "gfx/scene/object.h"
// #include "gfx/scene/render_object.h"
//
// #include <future>
// #include <memory>
// #include <vector>

// TODO:
// - implement abstraction layer for rhi (rendering api)
// - add debug pass for rendering (bounding boxes, light sources, normal vectors
// etc.)
// - shadow pass for the future

// namespace game_engine {
//
// #define ASYNC_WITH_SETUP           0
// #define PARALLELFOR_WITH_PASSSETUP 0
//
// struct SimplePushConstant {
//   // ======= BEGIN: public misc fields
//   ========================================
//
//   math::Vector4Df m_color_ = math::Vector4Df(1.0f, 1.0f, 1.0f, 1.0f);
//
//   // No need for now
//   // bool m_showVRSArea_ = false;
//   // bool m_padding_[3];
//
//   // bool m_showGrid_ = true;
//   // bool m_padding2_[3];
//
//   // ======= END: public misc fields ========================================
// };
//
// class Renderer {
//   public:
//   // ======= BEGIN: public constructors
//   =======================================
//
//   Renderer() = default;
//
//   Renderer(const std::shared_ptr<RenderFrameContext>& renderFrameContextPtr,
//            const View&                                view,
//            std::shared_ptr<Window>                    window)
//       : m_renderFrameContextPtr_(renderFrameContextPtr)
//       , m_view_(view)
//       , m_window_(std::move(window)) {}
//
//   // ======= END: public constructors =======================================
//
//   // ======= BEGIN: public destructor
//   =========================================
//
//   virtual ~Renderer() {}
//
//   // ======= END: public destructor =========================================
//
//   // ======= BEGIN: public overridden methods
//   =================================
//
//   virtual void setup() {
//     m_useForwardRenderer_ = m_renderFrameContextPtr_->m_useForwardRenderer_;
//
//     // view.ShadowCasterLights.reserve(view.Lights.size());
//
//     // for (int32_t i = 0; i < view.Lights.size(); ++i) {
//     //   ViewLight& viewLight = view.Lights[i];
//
//     //  if (viewLight.Light) {
//     //    if (viewLight.Light->IsShadowCaster) {
//     //      viewLight.ShadowMapPtr
//     //          = RenderFrameContextPtr->SceneRenderTargetPtr->GetShadowMap(
//     //              viewLight.Light);
//     //    }
//
//     //    viewLight.m_shaderBindingInstance
//     //        = viewLight.Light->PrepareShaderBindingInstance(
//     //            viewLight.ShadowMapPtr ?
//     viewLight.ShadowMapPtr->getTexture()
//     //                                   : nullptr);
//
//     //    if (viewLight.Light->IsShadowCaster) {
//     //      ViewLight NewViewLight = viewLight;
//     //      NewViewLight.m_shaderBindingInstance
//     //          = viewLight.Light->PrepareShaderBindingInstance(nullptr);
//     //      view.ShadowCasterLights.push_back(NewViewLight);
//     //    }
//     //  }
//     //}
//
//     // TODO: shadow pass setup before base pass
//     Renderer::setupBasePass();
//   }
//
//   virtual void basePass() {
//     // TODO: prob not needed, remove
//     // const auto& screenWidth  = m_window_->getSize().width();
//     // const auto& screenHeight = m_window_->getSize().height();
//
//     // TODO: prob not needed, remove
//     // if (m_basePassSetupCompleteEvent_.valid()) {
//     //   m_basePassSetupCompleteEvent_.wait();
//     // }
//
//     if (m_useForwardRenderer_) {
//       g_rhi->transitionLayout(
//           m_renderFrameContextPtr_->getActiveCommandBuffer(),
//           //
//           RenderFrameContextPtr->SceneRenderTargetPtr->ColorPtr->getTexture(),
//           m_renderFrameContextPtr_->m_sceneRenderTargetPtr_->m_finalColorPtr_
//               ->getTexture(),
//           EResourceLayout::COLOR_ATTACHMENT);
//       // TODO: worked for Vulkan
//       // EResourceLayout::PRESENT_SRC);
//     } else {
//       // TODO: not used for now
//       /*for (int32_t i = 0;
//          i
//          < std::size(RenderFrameContextPtr->SceneRenderTargetPtr->GBuffer);
//          ++i) {
//       g_rhi->transitionLayout(
//           RenderFrameContextPtr->getActiveCommandBuffer(),
//           RenderFrameContextPtr->SceneRenderTargetPtr->GBuffer[i]
//               ->getTexture(),
//           EResourceLayout::COLOR_ATTACHMENT);
//     }*/
//     }
//
//     // Depth only or depth stencil attachment
//     {
//       auto newLayout
//           = m_renderFrameContextPtr_->m_sceneRenderTargetPtr_->m_depthPtr_
//                     ->getTexture()
//                     ->isDepthOnlyFormat()
//               ? EResourceLayout::DEPTH_ATTACHMENT
//               : EResourceLayout::DEPTH_STENCIL_ATTACHMENT;
//
//       g_rhi->transitionLayout(
//           m_renderFrameContextPtr_->getActiveCommandBuffer(),
//           m_renderFrameContextPtr_->m_sceneRenderTargetPtr_->m_depthPtr_
//               ->getTexture(),
//           newLayout);
//     }
//
//     //
//     BasepassOcclusionTest.BeginQuery(RenderFrameContextPtr->getActiveCommandBuffer());
//     if (m_baseRenderPass_
//         && m_baseRenderPass_->beginRenderPass(
//             m_renderFrameContextPtr_->getActiveCommandBuffer())) {
//       // Draw G-m_buffer : subpass 0
//       for (const auto& command : m_basePasses_) {
//         command.draw();
//       }
//
//       // TODO: not used for now
//       // Draw Light : subpass 1
//       /*if (!useForwardRenderer && gOptions.UseSubpass) {
//         g_rhi->nextSubpass(
//             RenderFrameContextPtr->getActiveCommandBuffer());
//         DeferredLightPass_TodoRefactoring(BaseRenderPass);
//       }*/
//
//       m_baseRenderPass_->endRenderPass();
//     }
//   }
//
//   virtual void render() {
//     setup();
//
//     basePass();
//
//     // Queue submit to prepare scenecolor RT for postprocess
//     // if (gOptions.QueueSubmitAfterBasePass) {
//     // RenderFrameContextPtr->getActiveCommandBuffer()->end();
//     m_renderFrameContextPtr_->submitCurrentActiveCommandBuffer(
//         RenderFrameContext::BasePass);
//     // RenderFrameContextPtr->getActiveCommandBuffer()->begin();
//     //}
//
//     g_rhi->incrementFrameNumber();
//   }
//
//   // ======= END: public overridden methods =================================
//
//   // ======= BEGIN: public misc methods
//   =======================================
//
//   // TODO: consider making virtual
//   void setupBasePass() {
//     const auto& screenWidth  = m_window_->getSize().width();
//     const auto& screenHeight = m_window_->getSize().height();
//
//     // Prepare basepass pipeline
//     // TODO: refactor code to create RasterizationState
//     RasterizationStateInfo* rasterizationState = nullptr;
//     switch (g_rhi->getSelectedMSAASamples()) {
//       case EMSAASamples::COUNT_1:
//         rasterizationState = TRasterizationStateInfo<EPolygonMode::FILL,
//                                                      ECullMode::BACK,
//                                                      EFrontFace::CCW,
//                                                      false,
//                                                      0.0f,
//                                                      0.0f,
//                                                      0.0f,
//                                                      1.0f,
//                                                      false,
//                                                      false,
//                                                      (EMSAASamples)1,
//                                                      true,
//                                                      0.2f,
//                                                      false,
//                                                      false>::s_create();
//         break;
//       case EMSAASamples::COUNT_2:
//         rasterizationState = TRasterizationStateInfo<EPolygonMode::FILL,
//                                                      ECullMode::BACK,
//                                                      EFrontFace::CCW,
//                                                      false,
//                                                      0.0f,
//                                                      0.0f,
//                                                      0.0f,
//                                                      1.0f,
//                                                      false,
//                                                      false,
//                                                      (EMSAASamples)2,
//                                                      true,
//                                                      0.2f,
//                                                      false,
//                                                      false>::s_create();
//         break;
//       case EMSAASamples::COUNT_4:
//         rasterizationState = TRasterizationStateInfo<EPolygonMode::FILL,
//                                                      ECullMode::BACK,
//                                                      EFrontFace::CCW,
//                                                      false,
//                                                      0.0f,
//                                                      0.0f,
//                                                      0.0f,
//                                                      1.0f,
//                                                      false,
//                                                      false,
//                                                      (EMSAASamples)4,
//                                                      true,
//                                                      0.2f,
//                                                      false,
//                                                      false>::s_create();
//         break;
//       case EMSAASamples::COUNT_8:
//         rasterizationState = TRasterizationStateInfo<EPolygonMode::FILL,
//                                                      ECullMode::BACK,
//                                                      EFrontFace::CCW,
//                                                      false,
//                                                      0.0f,
//                                                      0.0f,
//                                                      0.0f,
//                                                      1.0f,
//                                                      false,
//                                                      false,
//                                                      (EMSAASamples)8,
//                                                      true,
//                                                      0.2f,
//                                                      false,
//                                                      false>::s_create();
//         break;
//       default:
//         assert(0);
//         break;
//     }
//
//     auto depthStencilState = TDepthStencilStateInfo<true,
//                                                     true,
//                                                     ECompareOp::LESS,
//                                                     false,
//                                                     false,
//                                                     0.0f,
//                                                     1.0f>::s_create();
//     auto blendingState     = TBlendingStateInfo<false,
//                                                 EBlendFactor::ONE,
//                                                 EBlendFactor::ZERO,
//                                                 EBlendOp::ADD,
//                                                 EBlendFactor::ONE,
//                                                 EBlendFactor::ZERO,
//                                                 EBlendOp::ADD,
//                                                 EColorMask::ALL>::s_create();
//
//     PipelineStateFixedInfo basePassPipelineStateFixed =
//     PipelineStateFixedInfo(
//         rasterizationState,
//         depthStencilState,
//         blendingState,
//         Viewport(0.0f, 0.0f, (float)screenWidth, (float)screenHeight),
//         Scissor(0, 0, screenWidth, screenWidth),
//         false /*gOptions.UseVRS*/);
//
//     // TODO: remove this code (not needed)
//     // -----------------------------------------------------
//     // auto TranslucentBlendingState
//     //    = TBlendingStateInfo<true,
//     //                         EBlendFactor::SRC_ALPHA,
//     //                         EBlendFactor::ONE_MINUS_SRC_ALPHA,
//     //                         EBlendOp::ADD,
//     //                         EBlendFactor::ONE,
//     //                         EBlendFactor::ZERO,
//     //                         EBlendOp::ADD,
//     //                         EColorMask::ALL>::s_create();
//     // PipelineStateFixedInfo TranslucentPassPipelineStateFixed
//     //    = PipelineStateFixedInfo(
//     //        rasterizationState,
//     //        depthStencilState,
//     //        TranslucentBlendingState,
//     //        Viewport(0.0f, 0.0f, (float)screenWidth, (float)screenHeight),
//     //        Scissor(0, 0, screenWidth, screenHeight),
//     //        gOptions.UseVRS);
//     // -----------------------------------------------------
//
//     const RtClearValue kClearColor = RtClearValue(0.0f, 0.0f, 0.0f, 1.0f);
//     const RtClearValue kClearDepth = RtClearValue(1.0f, 0);
//
//     Attachment depth = Attachment(
//         m_renderFrameContextPtr_->m_sceneRenderTargetPtr_->m_depthPtr_,
//         EAttachmentLoadStoreOp::CLEAR_STORE,
//         EAttachmentLoadStoreOp::CLEAR_STORE,
//         kClearDepth,
//         EResourceLayout::UNDEFINED,
//         EResourceLayout::DEPTH_STENCIL_ATTACHMENT);
//     Attachment resolve;
//
//     if (m_useForwardRenderer_) {
//       if ((int32_t)g_rhi->getSelectedMSAASamples() > 1) {
//         resolve = Attachment(
//             m_renderFrameContextPtr_->m_sceneRenderTargetPtr_->m_resolvePtr_,
//             EAttachmentLoadStoreOp::DONTCARE_STORE,
//             EAttachmentLoadStoreOp::DONTCARE_DONTCARE,
//             kClearColor,
//             EResourceLayout::UNDEFINED,
//             EResourceLayout::COLOR_ATTACHMENT,
//             true);
//       }
//     }
//
//     // Setup attachment
//     RenderPassInfo renderPassInfo;
//     if (!m_useForwardRenderer_) {
//       // TODO: not used for now
//       /*for (int32_t i = 0;
//          i < std::size(RenderFrameContextPtr->SceneRenderTargetPtr->GBuffer);
//          ++i) {
//       Attachment color = Attachment(
//           RenderFrameContextPtr->SceneRenderTargetPtr->GBuffer[i],
//           EAttachmentLoadStoreOp::CLEAR_STORE,
//           EAttachmentLoadStoreOp::DONTCARE_DONTCARE,
//           kClearColor,
//           EResourceLayout::UNDEFINED,
//           EResourceLayout::COLOR_ATTACHMENT);
//       renderPassInfo.Attachments.push_back(color);
//     }*/
//     }
//
//     const int32_t kLightPassAttachmentIndex
//         = (int32_t)renderPassInfo.m_attachments_.size();
//
//     // TODO: pay attention to UseSubpass
//     if (m_useForwardRenderer_ /*|| gOptions.UseSubpass*/) {
//       Attachment color
//           =
//           Attachment(/*RenderFrameContextPtr->SceneRenderTargetPtr->ColorPtr,*/
//                        m_renderFrameContextPtr_->m_sceneRenderTargetPtr_
//                            ->m_finalColorPtr_,
//                        EAttachmentLoadStoreOp::CLEAR_STORE,
//                        EAttachmentLoadStoreOp::DONTCARE_DONTCARE,
//                        kClearColor,
//                        EResourceLayout::UNDEFINED,
//                        // EResourceLayout::COLOR_ATTACHMENT);
//                        EResourceLayout::PRESENT_SRC);
//       renderPassInfo.m_attachments_.push_back(color);
//     }
//
//     const int32_t kDepthAttachmentIndex
//         = (int32_t)renderPassInfo.m_attachments_.size();
//     renderPassInfo.m_attachments_.push_back(depth);
//
//     const int32_t kResolveAttachemntIndex
//         = (int32_t)renderPassInfo.m_attachments_.size();
//     if (m_useForwardRenderer_) {
//       if ((int32_t)g_rhi->getSelectedMSAASamples() > 1) {
//         renderPassInfo.m_attachments_.push_back(resolve);
//       }
//     }
//
//     //////////////////////////////////////////////////////////////////////////
//     // Setup subpass of basePass
//     {
//       // First subpass, Geometry pass
//       Subpass subpass;
//       subpass.initialize(0,
//                          1,
//                          EPipelineStageMask::COLOR_ATTACHMENT_OUTPUT_BIT,
//                          EPipelineStageMask::FRAGMENT_SHADER_BIT);
//
//       if (m_useForwardRenderer_) {
//         subpass.m_outputColorAttachments_.push_back(0);
//       } else {
//         const int32_t kGBufferCount = kLightPassAttachmentIndex;
//         for (int32_t i = 0; i < kGBufferCount; ++i) {
//           subpass.m_outputColorAttachments_.push_back(i);
//         }
//       }
//
//       subpass.m_outputDepthAttachment_ = kDepthAttachmentIndex;
//       if (m_useForwardRenderer_) {
//         if ((int32_t)g_rhi->getSelectedMSAASamples() > 1) {
//           subpass.m_outputResolveAttachment_ = kResolveAttachemntIndex;
//         }
//       }
//       renderPassInfo.m_subpasses_.push_back(subpass);
//     }
//     // TODO: not used for now
//     // if (!useForwardRenderer && gOptions.UseSubpass) {
//     //   // Second subpass, Lighting pass
//     //   Subpass subpass;
//     //   subpass.initialize(1,
//     //                      2,
//     //                      EPipelineStageMask::COLOR_ATTACHMENT_OUTPUT_BIT,
//     //                      EPipelineStageMask::FRAGMENT_SHADER_BIT);
//     //
//     //  const int32_t kGBufferCount = kLightPassAttachmentIndex;
//     //  for (int32_t i = 0; i < kGBufferCount; ++i) {
//     //    subpass.InputAttachments.push_back(i);
//     //  }
//     //
//     //  subpass.OutputColorAttachments.push_back(kLightPassAttachmentIndex);
//     //  subpass.OutputDepthAttachment = kDepthAttachmentIndex;
//     //
//     //  if ((int32_t)g_rhi->getSelectedMSAASamples() > 1) {
//     //    subpass.OutputResolveAttachment = kResolveAttachemntIndex;
//     //  }
//     //
//     //  renderPassInfo.Subpasses.push_back(subpass);
//     //}
//     //////////////////////////////////////////////////////////////////////////
//     m_baseRenderPass_ = (RenderPass*)g_rhi->getOrCreateRenderPass(
//         renderPassInfo, {0, 0}, {screenWidth, screenHeight});
//
//     auto getOrCreateShaderFunc = [useForwardRenderer
//                                   = this->m_useForwardRenderer_](
//                                      const RenderObject* renderObject) {
//       GraphicsPipelineShader shaders;
//       ShaderInfo             shaderInfo;
//
//       if (renderObject->hasInstancing()) {
//         assert(useForwardRenderer);
//
//         ShaderInfo shaderInfo;
//         shaderInfo.setName(NameStatic("default_instancing_testVS"));
//         shaderInfo.setShaderFilepath(NameStatic(
//             "assets/shaders/forward_rendering/shader_instancing.vs.hlsl"));
//         shaderInfo.setShaderType(EShaderAccessStageFlag::VERTEX);
//         shaders.m_vertexShader_ = g_rhi->createShader(shaderInfo);
//
//         shaderInfo.setName(NameStatic("default_instancing_testPS"));
//         shaderInfo.setShaderFilepath(
//             NameStatic("assets/shaders/forward_rendering/shader.ps.hlsl"));
//         shaderInfo.setShaderType(EShaderAccessStageFlag::FRAGMENT);
//         shaders.m_pixelShader_ = g_rhi->createShader(shaderInfo);
//         return shaders;
//       }
//
//       if (useForwardRenderer) {
//         shaderInfo.setName(NameStatic("default_testVS"));
//         shaderInfo.setShaderFilepath(
//             //
//             NameStatic("assets/shaders/forward_rendering/shader.vs.hlsl"));
//             NameStatic("assets/shaders/demo/first_triangle.vs.hlsl"));
//         shaderInfo.setShaderType(EShaderAccessStageFlag::VERTEX);
//         shaders.m_vertexShader_ = g_rhi->createShader(shaderInfo);
//
//         ShaderForwardPixelShader::ShaderPermutation shaderPermutation;
//         /*shaderPermutation
//             .setIndex<ShaderForwardPixelShader::USE_VARIABLE_SHADING_RATE>(
//                 USE_VARIABLE_SHADING_RATE_TIER2);*/
//         shaderPermutation.setIndex<ShaderForwardPixelShader::USE_REVERSEZ>(
//             USE_REVERSEZ_PERSPECTIVE_SHADOW);
//         shaders.m_pixelShader_
//             = ShaderForwardPixelShader::CreateShader(shaderPermutation);
//       } else {
//         const bool kIsUseSphericalMap
//             = renderObject->m_materialPtr_
//            && renderObject->m_materialPtr_->isUseSphericalMap();
//         const bool kHasAlbedoTexture
//             = renderObject->m_materialPtr_
//            && renderObject->m_materialPtr_->hasAlbedoTexture();
//         const bool kIsUseSRGBAlbedoTexture
//             = renderObject->m_materialPtr_
//            && renderObject->m_materialPtr_->isUseSRGBAlbedoTexture();
//         const bool kHasVertexColor
//             = renderObject->m_geometryDataPtr_
//            && renderObject->m_geometryDataPtr_->hasVertexColor();
//         const bool kHasVertexBiTangent
//             = renderObject->m_geometryDataPtr_
//            && renderObject->m_geometryDataPtr_->hasVertexBiTangent();
//
//         // TODO: consider whether postfix 'Vs' is clearly describes 'vertex'
//         ShaderGBufferVertexShader::ShaderPermutation shaderPermutationVs;
//         shaderPermutationVs
//             .setIndex<ShaderGBufferVertexShader::USE_VERTEX_COLOR>(
//                 kHasVertexColor);
//         shaderPermutationVs
//             .setIndex<ShaderGBufferVertexShader::USE_VERTEX_BITANGENT>(
//                 kHasVertexBiTangent);
//         shaderPermutationVs
//             .setIndex<ShaderGBufferVertexShader::USE_ALBEDO_TEXTURE>(
//                 kHasAlbedoTexture);
//         shaderPermutationVs
//             .setIndex<ShaderGBufferVertexShader::USE_SPHERICAL_MAP>(
//                 kIsUseSphericalMap);
//         shaders.m_vertexShader_
//             = ShaderGBufferVertexShader::CreateShader(shaderPermutationVs);
//
//         ShaderGBufferPixelShader::ShaderPermutation shaderPermutationPs;
//         shaderPermutationPs
//             .setIndex<ShaderGBufferPixelShader::USE_VERTEX_COLOR>(
//                 kHasVertexColor);
//         shaderPermutationPs
//             .setIndex<ShaderGBufferPixelShader::USE_ALBEDO_TEXTURE>(
//                 kHasAlbedoTexture);
//         shaderPermutationPs
//             .setIndex<ShaderGBufferPixelShader::USE_SRGB_ALBEDO_TEXTURE>(
//                 kIsUseSRGBAlbedoTexture);
//         // shaderPermutationPs
//         // .setIndex<ShaderGBufferPixelShader::USE_VARIABLE_SHADING_RATE>(
//         //         USE_VARIABLE_SHADING_RATE_TIER2);
//         shaderPermutationPs.setIndex<ShaderGBufferPixelShader::USE_PBR>(
//             ENABLE_PBR);
//         shaders.m_pixelShader_
//             = ShaderGBufferPixelShader::CreateShader(shaderPermutationPs);
//       }
//       return shaders;
//     };
//
//     // TODO: remove (not used)
//     // ShaderInfo             shaderInfo;
//     // GraphicsPipelineShader TranslucentPassShader;
//     //{
//     //  shaderInfo.setName(NameStatic("default_testVS"));
//     //  shaderInfo.setShaderFilepath(
//     //      NameStatic("assets/shaders/deferred_rendering/gbuffer.vs.hlsl"));
//     //      //NameStatic("assets/shaders/demo/first_triangle.vs.hlsl"));
//     //  shaderInfo.setShaderType(EShaderAccessStageFlag::VERTEX);
//     //  TranslucentPassShader.m_vertexShader_ =
//     g_rhi->createShader(shaderInfo);
//     //
//     //  ShaderGBufferPixelShader::ShaderPermutation ShaderPermutation;
//     //
//     shaderPermutation.setIndex<ShaderGBufferPixelShader::USE_VERTEX_COLOR>(
//     //      0);
//     //
//     shaderPermutation.setIndex<ShaderGBufferPixelShader::USE_ALBEDO_TEXTURE>(
//     //      1);
//     //  //shaderPermutation
//     //  //    .setIndex<ShaderGBufferPixelShader::USE_VARIABLE_SHADING_RATE>(
//     //  //        USE_VARIABLE_SHADING_RATE_TIER2);
//     //  TranslucentPassShader.m_pixelShader_
//     //      = ShaderGBufferPixelShader::CreateShader(shaderPermutation);
//     //}
//
//     SimplePushConstant simplePushConstantData;
//     // simplePushConstantData.ShowVRSArea = gOptions.ShowVRSArea;
//     // simplePushConstantData.ShowGrid    = gOptions.ShowGrid;
//
//     PushConstant* simplePushConstant
//         = new (MemStack::get()->alloc<PushConstant>()) PushConstant(
//             simplePushConstantData, EShaderAccessStageFlag::FRAGMENT);
//
//     m_basePasses_.resize(Object::s_getStaticRenderObject().size());
//     int32_t i = 0;
//     for (auto iter : Object::s_getStaticRenderObject()) {
//       MaterialOld* material = nullptr;
//       if (iter->m_materialPtr_) {
//         material = iter->m_materialPtr_.get();
//       } else {
//         if (g_defaultMaterial) {
//           material = g_defaultMaterial.get();
//         }
//       }
//
//       new (&m_basePasses_[i]) DrawCommand(m_renderFrameContextPtr_,
//                                           &m_view_,
//                                           iter,
//                                           m_baseRenderPass_,
//                                           getOrCreateShaderFunc(iter),
//                                           &basePassPipelineStateFixed,
//                                           material,
//                                           {},
//                                           simplePushConstant);
//       m_basePasses_[i].prepareToDraw(false);
//       ++i;
//     }
//   }
//
//   // ======= END: public misc methods =======================================
//
//   // ======= BEGIN: public constants
//   ==========================================
//
//   // Thread per task for PassSetup
//   const int32_t kMaxPassSetupTaskPerThreadCount = 100;
//
//   // ======= END: public constants ==========================================
//
//   // ======= BEGIN: public misc fields
//   ========================================
//
//   bool m_useForwardRenderer_ = true;
//
//   std::shared_ptr<RenderFrameContext> m_renderFrameContextPtr_;
//   View                                m_view_;
//
//   std::future<void> m_shadowPassSetupCompleteEvent_;
//   std::future<void> m_basePassSetupCompleteEvent_;
//
//   std::vector<DrawCommand> m_basePasses_;
//
//   RenderPass* m_baseRenderPass_ = nullptr;
//
//   std::shared_ptr<Window> m_window_;
//
//   // ======= END: public misc fields ========================================
// };
//
// }  // namespace game_engine

#include "ecs/components/camera.h"
#include "ecs/components/light.h"
#include "ecs/components/render_model.h"
#include "ecs/components/transform.h"
#include "gfx/rhi/rhi.h"
#include "scene/scene.h"

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

    // Begin Render Pass
    if (renderPass_->beginRenderPass(
            m_renderContext_->renderFrameContext->getActiveCommandBuffer())) {
      for (const auto& drawData : drawDataList_) {
        // Bind Pipeline
        drawData.pipelineStateInfo_->bind(m_renderContext_->renderFrameContext);

        // Bind Shader Binding Instances
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
          // Add DescriptorSetLayout data
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

        // Bind Vertex Buffers (vertex buffer and instance buffer)
        const auto& gpuMesh = drawData.renderGeometryMesh_;
        gpuMesh->vertexBuffer->bind(m_renderContext_->renderFrameContext);
        drawData.instanceBuffer_->bind(m_renderContext_->renderFrameContext);

        // Bind Index Buffer
        gpuMesh->indexBuffer->bind(m_renderContext_->renderFrameContext);

        // Draw with instancing
        g_rhi->drawElementsInstanced(m_renderContext_->renderFrameContext,
                                     0,
                                     0,
                                     gpuMesh->indexBuffer->getElementCount(),
                                     drawData.instanceCount);
      }

      // End Render Pass
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

    // Map to collect model matrices per RenderModel
    std::unordered_map<std::shared_ptr<RenderModel>,
                       std::vector<math::Matrix4f<>>>
        modelInstances;

    for (auto entity : view) {
      auto& transform   = view.get<Transform>(entity);
      auto& renderModel = view.get<std::shared_ptr<RenderModel>>(entity);

      // Calculate model matrix
      math::Matrix4f modelMatrix = calculateTransformMatrix(transform);

      // Collect model matrices per RenderModel
      modelInstances[renderModel].push_back(modelMatrix);
    }

    // Prepare DrawData for each RenderModel and its RenderMeshes
    for (const auto& pair : modelInstances) {
      const std::shared_ptr<RenderModel>&  renderModel   = pair.first;
      const std::vector<math::Matrix4f<>>& modelMatrices = pair.second;

      // Create instance buffer
      std::shared_ptr<VertexBuffer> instanceBuffer;
      createInstanceBuffer(modelMatrices, instanceBuffer);

      // For each RenderMesh in the RenderModel
      for (const auto& renderMesh : renderModel->renderMeshes) {
        DrawData drawData;
        drawData.renderGeometryMesh_ = renderMesh->gpuMesh;
        drawData.material_           = renderMesh->material;
        drawData.instanceBuffer_     = instanceBuffer;
        drawData.instanceCount = static_cast<uint32_t>(modelMatrices.size());

        // Prepare Shader Binding Instances
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

        // Create material ShaderBindingInstance in BasePass
        if (drawData.material_) {
          drawData.materialShaderBindingInstance_
              = createMaterialShaderBindingInstance(drawData.material_);
          shaderBindingInstanceArray.add(
              drawData.materialShaderBindingInstance_.get());
        }

        // Collect Shader Binding Layouts
        ShaderBindingLayoutArray shaderBindingLayoutArray;
        for (int32_t i = 0; i < shaderBindingInstanceArray.m_numOfData_; ++i) {
          shaderBindingLayoutArray.add(
              shaderBindingInstanceArray[i]->m_shaderBindingsLayouts_);
        }

        // Prepare Vertex Buffer Array (vertex buffer and instance buffer)
        VertexBufferArray vertexBufferArray;
        vertexBufferArray.add(
            drawData.renderGeometryMesh_->vertexBuffer.get());  // Binding 0
        vertexBufferArray.add(drawData.instanceBuffer_.get());  // Binding 1

        // Create Pipeline State Info
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

        // Store DrawData
        drawDataList_.push_back(std::move(drawData));
      }
    }
  }

  void createInstanceBuffer(const std::vector<math::Matrix4f<>>& modelMatrices,
                            std::shared_ptr<VertexBuffer>& instanceBuffer) {
    constexpr size_t kMatrixSize = 16;

    // Flatten model matrices into a float array
    std::vector<float> instanceData;

    instanceData.reserve(modelMatrices.size() * kMatrixSize);
    for (const auto& matrix : modelMatrices) {
      instanceData.insert(
          instanceData.end(), matrix.data(), matrix.data() + kMatrixSize);
    }

    // Create buffer attribute stream for instance data
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

    // Set primitive type and element count
    instanceStreamData->m_primitiveType_ = EPrimitiveType::TRIANGLES;
    instanceStreamData->m_elementCount_
        = static_cast<int32_t>(modelMatrices.size());
    // instanceStreamData->m_startLocation_
    //     = 6;  // TODO: fix this (take the input slot)
    instanceStreamData->m_bindingIndex_ = 6;

    // Set input rate to instance
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
        texture = g_blackTexture;
      }

      // Create ShaderBinding
      shaderBindingArray.add(ShaderBinding(
          bindingPoint++,
          1,
          EShaderBindingType::TEXTURE_SAMPLER_SRV,
          false,
          EShaderAccessStageFlag::FRAGMENT,
          resourceAllocator.alloc<TextureResource>(texture.get(), nullptr)));
    }

    // Create ShaderBindingInstance
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

    // Begin Command Buffer
    m_renderContext_->renderFrameContext->getActiveCommandBuffer()->begin();

    // Execute Base Pass
    basePass_->execute();

    // End Command Buffer
    m_renderContext_->renderFrameContext->getActiveCommandBuffer()->end();

    // Submit Command Buffer
    m_renderContext_->renderFrameContext->submitCurrentActiveCommandBuffer(
        RenderFrameContext::BasePass);

    // Present Frame
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
