
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
  // ======= BEGIN: public constructors =======================================

  DrawCommand() = default;

  DrawCommand(const std::shared_ptr<RenderFrameContext>& renderFrameContextPtr,
              const View*                                view,
              RenderObject*                              renderObject,
              RenderPass*                                renderPass,
              GraphicsPipelineShader                     shader,
              PipelineStateFixedInfo*                    pipelineStateFixed,
              Material*                                  material,
              const ShaderBindingInstanceArray& shaderBindingInstanceArray,
              const PushConstant*               pushConstant,
              const VertexBuffer*               overrideInstanceData = nullptr,
              int32_t                           subpassIndex         = 0)
      : m_renderFrameContextPtr_(renderFrameContextPtr)
      , m_view_(view)
      , m_renderObject_(renderObject)
      , m_renderPass_(renderPass)
      , m_shader_(shader)
      , m_pipelineStateFixed_(pipelineStateFixed)
      , m_material_(material)
      , m_pushConstant_(pushConstant)
      , m_shaderBindingInstanceArray_(shaderBindingInstanceArray)
      , m_overrideInstanceData_(overrideInstanceData)
      , m_subpassIndex_(subpassIndex) {
    assert(m_renderObject_);
  }

  // TODO: no need (used for light / shadows)
  /*DrawCommand(
      const std::shared_ptr<RenderFrameContext>& renderFrameContextPtr,
      const ViewLight*                            viewLight,
      RenderObject*                                renderObject,
      RenderPass*                                renderPass,
      GraphicsPipelineShader                       shader,
      PipelineStateFixedInfo*                    pipelineStateFixed,
      Material*                                    material,
      const ShaderBindingInstanceArray& shaderBindingInstanceArray, const
  PushConstant*                        pushConstant, const VertexBuffer*
  overrideInstanceData, int32_t subpassIndex) :
  RenderFrameContextPtr(renderFrameContextPtr) , viewLight(viewLight) ,
  RenderObject(renderObject) , kRenderPass(renderPass) , Shader(shader) ,
  PipelineStateFixed(pipelineStateFixed) , Material(material) ,
  kPushConstant(pushConstant) ,
  m_shaderBindingInstanceArray(shaderBindingInstanceArray) ,
  OverrideInstanceData(overrideInstanceData) , SubpassIndex(subpassIndex) {
    assert(RenderObject);
    IsViewLight = true;
  }*/

  DrawCommand(const std::shared_ptr<RenderFrameContext>& renderFrameContextPtr,
              RenderObject*                              renderObject,
              RenderPass*                                renderPass,
              GraphicsPipelineShader                     shader,
              PipelineStateFixedInfo*                    pipelineStateFixed,
              Material*                                  material,
              const ShaderBindingInstanceArray& shaderBindingInstanceArray,
              const PushConstant*               pushConstant,
              const VertexBuffer*               overrideInstanceData = nullptr,
              int32_t                           subpassIndex         = 0)
      : m_renderFrameContextPtr_(renderFrameContextPtr)
      , m_renderObject_(renderObject)
      , m_renderPass_(renderPass)
      , m_shader_(shader)
      , m_pipelineStateFixed_(pipelineStateFixed)
      , m_material_(material)
      , m_pushConstant_(pushConstant)
      , m_shaderBindingInstanceArray_(shaderBindingInstanceArray)
      , m_overrideInstanceData_(overrideInstanceData)
      , m_subpassIndex_(subpassIndex) {
    assert(m_renderObject_);
  }

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public misc methods =======================================

  void prepareToDraw(bool isPositionOnly);

  void draw() const;

  // ======= END: public misc methods   =======================================

  // ======= BEGIN: public constants ==========================================

  const View*         m_view_;
  const PushConstant* m_pushConstant_         = nullptr;
  const VertexBuffer* m_overrideInstanceData_ = nullptr;

  // ======= END: public constants   ==========================================

  // ======= BEGIN: public misc fields ========================================

  ShaderBindingInstanceArray    m_shaderBindingInstanceArray_;
  ShaderBindingInstanceCombiner m_shaderBindingInstanceCombiner_;

  RenderPass*             m_renderPass_ = nullptr;
  GraphicsPipelineShader  m_shader_;
  RenderObject*           m_renderObject_             = nullptr;
  PipelineStateFixedInfo* m_pipelineStateFixed_       = nullptr;
  PipelineStateInfo*      m_currentPipelineStateInfo_ = nullptr;
  Material*               m_material_                 = nullptr;

  std::shared_ptr<RenderFrameContext> m_renderFrameContextPtr_;
  bool                                m_isPositionOnly_ = false;
  int32_t                             m_subpassIndex_   = 0;
  bool                                m_test_           = false;

  std::shared_ptr<ShaderBindingInstance> m_oneRenderObjectUniformBuffer_;

  // ======= END: public misc fields   ========================================


};

// TODO: not used (used for light draw commands)
// class DrawCommandGenerator {
//  public:
//  virtual ~DrawCommandGenerator() {}
//
//  virtual void initialize(int32_t rtWidth, int32_t rtHeight) = 0;
//  virtual void generateDrawCommand(
//      DrawCommand*                                 destDrawCommand,
//      const std::shared_ptr<RenderFrameContext>& renderFrameContextPtr,
//      const View*                                 view,
//      const ViewLight&                            lightView,
//      RenderPass*                                renderPass,
//      int32_t                                      subpassIndex)
//      = 0;
//};

}  // namespace game_engine

#endif  // GAME_ENGINE_DRAW_COMMAND_H