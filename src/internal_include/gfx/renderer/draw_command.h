
#ifndef GAME_ENGINE_DRAW_COMMAND_H
#define GAME_ENGINE_DRAW_COMMAND_H

#include "gfx/renderer/material.h"
#include "gfx/rhi/vulkan/buffer_vk.h"
#include "gfx/rhi/vulkan/pipeline_state_info_vk.h"
#include "gfx/rhi/vulkan/render_frame_context_vk.h"
#include "gfx/rhi/vulkan/render_pass_vk.h"
#include "gfx/rhi/vulkan/shader_binding_instance_combiner.h"
#include "gfx/rhi/vulkan/shader_binding_layout_vk.h"
#include "gfx/rhi/vulkan/shader_vk.h"
#include "gfx/scene/render_object.h"
#include "gfx/scene/view.h"

#include <memory>

namespace game_engine {

class DrawCommand {
  public:
  DrawCommand() = default;

  DrawCommand(
      const std::shared_ptr<RenderFrameContextVk>& InRenderFrameContextPtr,
      const View*                                  InView,
      RenderObject*                                InRenderObject,
      RenderPassVk*                                InRenderPass,
      GraphicsPipelineShader                       InShader,
      PipelineStateFixedInfoVk*                    InPipelineStateFixed,
      Material*                                    InMaterial,
      const ShaderBindingInstanceArray&            InShaderBindingInstanceArray,
      const PushConstantVk*                        InPushConstant,
      const VertexBufferVk* InOverrideInstanceData = nullptr,
      int32_t               InSubpassIndex         = 0)
      : RenderFrameContextPtr(InRenderFrameContextPtr)
      , view(InView)
      , RenderObject(InRenderObject)
      , RenderPass(InRenderPass)
      , Shader(InShader)
      , PipelineStateFixed(InPipelineStateFixed)
      , Material(InMaterial)
      , PushConstant(InPushConstant)
      , shaderBindingInstanceArray(InShaderBindingInstanceArray)
      , OverrideInstanceData(InOverrideInstanceData)
      , SubpassIndex(InSubpassIndex) {
    assert(RenderObject);
  }

  // TODO: no need (used for light / shadows)
  /*DrawCommand(
      const std::shared_ptr<RenderFrameContextVk>& InRenderFrameContextPtr,
      const ViewLight*                            InViewLight,
      RenderObject*                                InRenderObject,
      RenderPassVk*                                InRenderPass,
      GraphicsPipelineShader                       InShader,
      PipelineStateFixedInfoVk*                    InPipelineStateFixed,
      Material*                                    InMaterial,
      const ShaderBindingInstanceArray&            InShaderBindingInstanceArray,
      const PushConstantVk*                        InPushConstant,
      const VertexBufferVk*                        InOverrideInstanceData,
      int32_t                                      InSubpassIndex)
      : RenderFrameContextPtr(InRenderFrameContextPtr)
      , viewLight(InViewLight)
      , RenderObject(InRenderObject)
      , RenderPass(InRenderPass)
      , Shader(InShader)
      , PipelineStateFixed(InPipelineStateFixed)
      , Material(InMaterial)
      , PushConstant(InPushConstant)
      , ShaderBindingInstanceArray(InShaderBindingInstanceArray)
      , OverrideInstanceData(InOverrideInstanceData)
      , SubpassIndex(InSubpassIndex) {
    assert(RenderObject);
    IsViewLight = true;
  }*/

  DrawCommand(
      const std::shared_ptr<RenderFrameContextVk>& InRenderFrameContextPtr,
      RenderObject*                                InRenderObject,
      RenderPassVk*                                InRenderPass,
      GraphicsPipelineShader                       InShader,
      PipelineStateFixedInfoVk*                    InPipelineStateFixed,
      Material*                                    InMaterial,
      const ShaderBindingInstanceArray&            InShaderBindingInstanceArray,
      const PushConstantVk*                        InPushConstant,
      const VertexBufferVk* InOverrideInstanceData = nullptr,
      int32_t               InSubpassIndex         = 0)
      : RenderFrameContextPtr(InRenderFrameContextPtr)
      , RenderObject(InRenderObject)
      , RenderPass(InRenderPass)
      , Shader(InShader)
      , PipelineStateFixed(InPipelineStateFixed)
      , Material(InMaterial)
      , PushConstant(InPushConstant)
      , shaderBindingInstanceArray(InShaderBindingInstanceArray)
      , OverrideInstanceData(InOverrideInstanceData)
      , SubpassIndex(InSubpassIndex) {
    assert(RenderObject);
  }

  void PrepareToDraw(bool InIsPositionOnly);

  void Draw() const;

  ShaderBindingInstanceArray    shaderBindingInstanceArray;
  ShaderBindingInstanceCombiner shaderBindingInstanceCombiner;

  const View* view;

  RenderPassVk*                         RenderPass = nullptr;
  GraphicsPipelineShader                Shader;
  RenderObject*                         RenderObject             = nullptr;
  PipelineStateFixedInfoVk*             PipelineStateFixed       = nullptr;
  PipelineStateInfoVk*                  CurrentPipelineStateInfo = nullptr;
  Material*                             Material                 = nullptr;
  const PushConstantVk*                 PushConstant             = nullptr;
  const VertexBufferVk*                 OverrideInstanceData     = nullptr;
  std::shared_ptr<RenderFrameContextVk> RenderFrameContextPtr;
  bool                                  IsPositionOnly = false;
  int32_t                               SubpassIndex   = 0;
  bool                                  Test           = false;

  std::shared_ptr<ShaderBindingInstance> OneRenderObjectUniformBuffer;
};

// TODO: not used (used for light draw commands)
// class DrawCommandGenerator {
//  public:
//  virtual ~DrawCommandGenerator() {}
//
//  virtual void Initialize(int32_t InRTWidth, int32_t InRTHeight) = 0;
//  virtual void GenerateDrawCommand(
//      DrawCommand*                                 OutDestDrawCommand,
//      const std::shared_ptr<RenderFrameContextVk>& InRenderFrameContextPtr,
//      const View*                                 InView,
//      const ViewLight&                            InLightView,
//      RenderPassVk*                                InRenderPass,
//      int32_t                                      InSubpassIndex)
//      = 0;
//};

}  // namespace game_engine

#endif  // GAME_ENGINE_DRAW_COMMAND_H