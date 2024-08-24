
#include "gfx/renderer/draw_command.h"

#include "gfx/rhi/rhi.h"
#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

void DrawCommand::prepareToDraw(bool isPositionOnly) {
  if (!m_test_) {
    // GetShaderBindings
    if (m_view_) {
      m_view_->getShaderBindingInstance(m_shaderBindingInstanceArray_,
                                     m_renderFrameContextPtr_->m_useForwardRenderer_);
    }

    // GetShaderBindings
    m_oneRenderObjectUniformBuffer_ = m_renderObject_->createShaderBindingInstance();
    m_shaderBindingInstanceArray_.add(m_oneRenderObjectUniformBuffer_.get());

    if (m_material_) {
      m_shaderBindingInstanceArray_.add(
          m_material_->createShaderBindingInstance().get());
    }
  }

  // Gather ShaderBindings
  ShaderBindingLayoutArray shaderBindingLayoutArray;
  for (int32_t i = 0; i < m_shaderBindingInstanceArray_.m_numOfData_; ++i) {
    // Add DescriptorSetLayout data
    shaderBindingLayoutArray.add(
        m_shaderBindingInstanceArray_[i]->m_shaderBindingsLayouts_);

    // Add shaderBindingInstanceCombiner data : DescriptorSets, DynamicOffsets
    m_shaderBindingInstanceCombiner_.m_descriptorSetHandles_.add(
        m_shaderBindingInstanceArray_[i]->getHandle());
    const std::vector<uint32_t>* pDynamicOffsetTest
        = m_shaderBindingInstanceArray_[i]->getDynamicOffsets();
    if (pDynamicOffsetTest && pDynamicOffsetTest->size()) {
      m_shaderBindingInstanceCombiner_.m_dynamicOffsets_.add(
          (void*)pDynamicOffsetTest->data(),
          (int32_t)pDynamicOffsetTest->size());
    }
  }
  m_shaderBindingInstanceCombiner_.m_shaderBindingInstanceArray
      = &m_shaderBindingInstanceArray_;

  const auto& RenderObjectGeoDataPtr = m_renderObject_->m_geometryDataPtr_;

  VertexBufferArray vertexBufferArray;
  vertexBufferArray.add(
      isPositionOnly
          ? RenderObjectGeoDataPtr->m_vertexBufferPositionOnlyPtr_.get()
          : RenderObjectGeoDataPtr->m_vertexBufferPtr_.get());
  if (m_overrideInstanceData_) {
    vertexBufferArray.add(m_overrideInstanceData_);
  } else if (RenderObjectGeoDataPtr->m_vertexBufferInstanceDataPtr_) {
    vertexBufferArray.add(
        RenderObjectGeoDataPtr->m_vertexBufferInstanceDataPtr_.get());
  }

  // Create Pipeline
  m_currentPipelineStateInfo_
      = (PipelineStateInfo*)g_rhi->createPipelineStateInfo(
          m_pipelineStateFixed_,
          m_shader_,
          vertexBufferArray,
          m_renderPass_,
          shaderBindingLayoutArray,
          m_pushConstant_,
          m_subpassIndex_);

  m_isPositionOnly_ = isPositionOnly;
}

void DrawCommand::draw() const {
  assert(m_renderFrameContextPtr_);

  g_rhi->bindGraphicsShaderBindingInstances(
      m_renderFrameContextPtr_->getActiveCommandBuffer(),
      m_currentPipelineStateInfo_,
      m_shaderBindingInstanceCombiner_,
      0);

  // Bind the image that contains the shading rate patterns
#if USE_VARIABLE_SHADING_RATE_TIER2
  if (gOptions.UseVRS) {
    g_rhiVk->bindShadingRateImage(
        RenderFrameContextPtr->getActiveCommandBuffer(),
        g_rhiVk->GetSampleVRSTexture());
  }
#endif

  // Bind Pipeline
  m_currentPipelineStateInfo_->bind(m_renderFrameContextPtr_);

  // this is only for Vulkan for now
  // TODO: add check to Vulkan
  bool isUseVulkan = false;
  if (isUseVulkan) {
    if (m_pushConstant_ && m_pushConstant_->isValid()) {
      const ResourceContainer<PushConstantRange>* pushConstantRanges
          = m_pushConstant_->getPushConstantRanges();
      assert(pushConstantRanges);
      if (pushConstantRanges) {
        for (int32_t i = 0; i < pushConstantRanges->m_numOfData_; ++i) {
          const PushConstantRange& range = (*pushConstantRanges)[i];
          vkCmdPushConstants(
              (VkCommandBuffer)m_renderFrameContextPtr_->getActiveCommandBuffer()
                  ->getNativeHandle(),
              ((PipelineStateInfoVk*)m_currentPipelineStateInfo_)
                  ->m_pipelineLayout_,
              g_getVulkanShaderAccessFlags(range.m_accessStageFlag_),
              range.m_offset_,
              range.m_size_,
              m_pushConstant_->getConstantData());
        }
      }
    }
  }
  m_renderObject_->bindBuffers(
      m_renderFrameContextPtr_, m_isPositionOnly_, m_overrideInstanceData_);

  // Draw
  const auto&          RenderObjectGeoDataPtr = m_renderObject_->m_geometryDataPtr_;
  const VertexBuffer* InstanceData
      = m_overrideInstanceData_
          ? m_overrideInstanceData_
          : RenderObjectGeoDataPtr->m_vertexBufferInstanceDataPtr_.get();
  const int32_t InstanceCount
      = InstanceData ? InstanceData->getElementCount() : 1;
  m_renderObject_->draw(m_renderFrameContextPtr_, InstanceCount);
}

}  // namespace game_engine
