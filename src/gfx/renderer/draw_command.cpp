
#include "gfx/renderer/draw_command.h"

#include "gfx/rhi/rhi.h"
#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

void DrawCommand::PrepareToDraw(bool isPositionOnly) {
  if (!m_test_) {
    // GetShaderBindings
    if (m_view_) {
      m_view_->GetShaderBindingInstance(m_shaderBindingInstanceArray_,
                                     m_renderFrameContextPtr_->m_useForwardRenderer_);
    }

    // GetShaderBindings
    m_oneRenderObjectUniformBuffer_ = m_renderObject_->CreateShaderBindingInstance();
    m_shaderBindingInstanceArray_.Add(m_oneRenderObjectUniformBuffer_.get());

    if (m_material_) {
      m_shaderBindingInstanceArray_.Add(
          m_material_->CreateShaderBindingInstance().get());
    }
  }

  // Gather ShaderBindings
  ShaderBindingLayoutArray shaderBindingLayoutArray;
  for (int32_t i = 0; i < m_shaderBindingInstanceArray_.m_numOfData_; ++i) {
    // Add DescriptorSetLayout data
    shaderBindingLayoutArray.Add(
        m_shaderBindingInstanceArray_[i]->m_shaderBindingsLayouts_);

    // Add shaderBindingInstanceCombiner data : DescriptorSets, DynamicOffsets
    m_shaderBindingInstanceCombiner_.m_descriptorSetHandles_.Add(
        m_shaderBindingInstanceArray_[i]->GetHandle());
    const std::vector<uint32_t>* pDynamicOffsetTest
        = m_shaderBindingInstanceArray_[i]->GetDynamicOffsets();
    if (pDynamicOffsetTest && pDynamicOffsetTest->size()) {
      m_shaderBindingInstanceCombiner_.m_dynamicOffsets_.Add(
          (void*)pDynamicOffsetTest->data(),
          (int32_t)pDynamicOffsetTest->size());
    }
  }
  m_shaderBindingInstanceCombiner_.m_shaderBindingInstanceArray
      = &m_shaderBindingInstanceArray_;

  const auto& RenderObjectGeoDataPtr = m_renderObject_->m_geometryDataPtr_;

  VertexBufferArray vertexBufferArray;
  vertexBufferArray.Add(
      isPositionOnly
          ? RenderObjectGeoDataPtr->m_vertexBufferPositionOnlyPtr_.get()
          : RenderObjectGeoDataPtr->m_vertexBufferPtr_.get());
  if (m_overrideInstanceData_) {
    vertexBufferArray.Add(m_overrideInstanceData_);
  } else if (RenderObjectGeoDataPtr->m_vertexBufferInstanceDataPtr_) {
    vertexBufferArray.Add(
        RenderObjectGeoDataPtr->m_vertexBufferInstanceDataPtr_.get());
  }

  // Create Pipeline
  m_currentPipelineStateInfo_
      = (PipelineStateInfo*)g_rhi->CreatePipelineStateInfo(
          m_pipelineStateFixed_,
          m_shader_,
          vertexBufferArray,
          m_renderPass_,
          shaderBindingLayoutArray,
          m_pushConstant_,
          m_subpassIndex_);

  m_isPositionOnly_ = isPositionOnly;
}

void DrawCommand::Draw() const {
  assert(m_renderFrameContextPtr_);

  g_rhi->BindGraphicsShaderBindingInstances(
      m_renderFrameContextPtr_->GetActiveCommandBuffer(),
      m_currentPipelineStateInfo_,
      m_shaderBindingInstanceCombiner_,
      0);

  // Bind the image that contains the shading rate patterns
#if USE_VARIABLE_SHADING_RATE_TIER2
  if (gOptions.UseVRS) {
    g_rhi_vk->BindShadingRateImage(
        RenderFrameContextPtr->GetActiveCommandBuffer(),
        g_rhi_vk->GetSampleVRSTexture());
  }
#endif

  // Bind Pipeline
  m_currentPipelineStateInfo_->Bind(m_renderFrameContextPtr_);

  // this is only for Vulkan for now
  // TODO: add check to Vulkan
  bool isUseVulkan = false;
  if (isUseVulkan) {
    if (m_pushConstant_ && m_pushConstant_->IsValid()) {
      const ResourceContainer<PushConstantRange>* pushConstantRanges
          = m_pushConstant_->GetPushConstantRanges();
      assert(pushConstantRanges);
      if (pushConstantRanges) {
        for (int32_t i = 0; i < pushConstantRanges->m_numOfData_; ++i) {
          const PushConstantRange& range = (*pushConstantRanges)[i];
          vkCmdPushConstants(
              (VkCommandBuffer)m_renderFrameContextPtr_->GetActiveCommandBuffer()
                  ->GetNativeHandle(),
              ((PipelineStateInfoVk*)m_currentPipelineStateInfo_)
                  ->m_pipelineLayout_,
              GetVulkanShaderAccessFlags(range.m_accessStageFlag_),
              range.m_offset_,
              range.m_size_,
              m_pushConstant_->GetConstantData());
        }
      }
    }
  }
  m_renderObject_->BindBuffers(
      m_renderFrameContextPtr_, m_isPositionOnly_, m_overrideInstanceData_);

  // Draw
  const auto&          RenderObjectGeoDataPtr = m_renderObject_->m_geometryDataPtr_;
  const VertexBuffer* InstanceData
      = m_overrideInstanceData_
          ? m_overrideInstanceData_
          : RenderObjectGeoDataPtr->m_vertexBufferInstanceDataPtr_.get();
  const int32_t InstanceCount
      = InstanceData ? InstanceData->GetElementCount() : 1;
  m_renderObject_->Draw(m_renderFrameContextPtr_, InstanceCount);
}

}  // namespace game_engine
