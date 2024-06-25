
#ifndef GAME_ENGINE_DRAW_COMMAND_H
#define GAME_ENGINE_DRAW_COMMAND_H

#include "gfx/renderer/material.h"
#include "gfx/rhi/buffer.h"
#include "gfx/rhi/pipeline_state_info.h"
#include "gfx/rhi/render_frame_context.h"
#include "gfx/rhi/render_pass.h"
#include "gfx/rhi/shader.h"
#include "gfx/rhi/shader_binding_instance_combiner.h"
#include "gfx/rhi/shader_binding_layout.h"
#include "gfx/scene/render_object.h"
#include "gfx/scene/view.h"

#include <memory>

namespace game_engine {

class DrawCommand {
  public:
  DrawCommand() = default;

  DrawCommand(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContextPtr,
      const View*                                 InView,
      RenderObject*                               InRenderObject,
      jRenderPass*                                InRenderPass,
      GraphicsPipelineShader                      InShader,
      jPipelineStateFixedInfo*                    InPipelineStateFixed,
      jMaterial*                                  InMaterial,
      const jShaderBindingInstanceArray&          InShaderBindingInstanceArray,
      const jPushConstant*                        InPushConstant,
      const jVertexBuffer* InOverrideInstanceData = nullptr,
      int32_t              InSubpassIndex         = 0)
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
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContextPtr,
      const ViewLight*                            InViewLight,
      RenderObject*                                InRenderObject,
      jRenderPass*                                InRenderPass,
      GraphicsPipelineShader                       InShader,
      jPipelineStateFixedInfo*                    InPipelineStateFixed,
      jMaterial*                                    InMaterial,
      const jShaderBindingInstanceArray& InShaderBindingInstanceArray, const
  jPushConstant*                        InPushConstant, const jVertexBuffer*
  InOverrideInstanceData, int32_t InSubpassIndex) :
  RenderFrameContextPtr(InRenderFrameContextPtr) , viewLight(InViewLight) ,
  RenderObject(InRenderObject) , RenderPass(InRenderPass) , Shader(InShader) ,
  PipelineStateFixed(InPipelineStateFixed) , jMaterial(InMaterial) ,
  PushConstant(InPushConstant) ,
  ShaderBindingInstanceArray(InShaderBindingInstanceArray) ,
  OverrideInstanceData(InOverrideInstanceData) , SubpassIndex(InSubpassIndex) {
    assert(RenderObject);
    IsViewLight = true;
  }*/

  DrawCommand(
      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContextPtr,
      RenderObject*                               InRenderObject,
      jRenderPass*                                InRenderPass,
      GraphicsPipelineShader                      InShader,
      jPipelineStateFixedInfo*                    InPipelineStateFixed,
      jMaterial*                                  InMaterial,
      const jShaderBindingInstanceArray&          InShaderBindingInstanceArray,
      const jPushConstant*                        InPushConstant,
      const jVertexBuffer* InOverrideInstanceData = nullptr,
      int32_t              InSubpassIndex         = 0)
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

  jShaderBindingInstanceArray   shaderBindingInstanceArray;
  ShaderBindingInstanceCombiner shaderBindingInstanceCombiner;

  const View* view;

  jRenderPass*                         RenderPass = nullptr;
  GraphicsPipelineShader               Shader;
  RenderObject*                        RenderObject             = nullptr;
  jPipelineStateFixedInfo*             PipelineStateFixed       = nullptr;
  jPipelineStateInfo*                 CurrentPipelineStateInfo = nullptr;
  jMaterial*                           Material                 = nullptr;
  const jPushConstant*                 PushConstant             = nullptr;
  const jVertexBuffer*                 OverrideInstanceData     = nullptr;
  std::shared_ptr<jRenderFrameContext> RenderFrameContextPtr;
  bool                                 IsPositionOnly = false;
  int32_t                              SubpassIndex   = 0;
  bool                                 Test           = false;

  std::shared_ptr<jShaderBindingInstance> OneRenderObjectUniformBuffer;
};

// TODO: not used (used for light draw commands)
// class DrawCommandGenerator {
//  public:
//  virtual ~DrawCommandGenerator() {}
//
//  virtual void Initialize(int32_t InRTWidth, int32_t InRTHeight) = 0;
//  virtual void GenerateDrawCommand(
//      DrawCommand*                                 OutDestDrawCommand,
//      const std::shared_ptr<jRenderFrameContext>& InRenderFrameContextPtr,
//      const View*                                 InView,
//      const ViewLight&                            InLightView,
//      jRenderPass*                                InRenderPass,
//      int32_t                                      InSubpassIndex)
//      = 0;
//};

}  // namespace game_engine

#endif  // GAME_ENGINE_DRAW_COMMAND_H