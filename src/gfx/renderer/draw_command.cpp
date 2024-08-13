
#include "gfx/renderer/draw_command.h"

#include "gfx/rhi/rhi.h"
#include "gfx/rhi/vulkan/rhi_vk.h"

namespace game_engine {

void DrawCommand::PrepareToDraw(bool InIsPositionOnly) {
  if (!Test) {
    // GetShaderBindings
    if (view) {
      view->GetShaderBindingInstance(m_shaderBindingInstanceArray,
                                     RenderFrameContextPtr->UseForwardRenderer);
    }

    // GetShaderBindings
    OneRenderObjectUniformBuffer = RenderObject->CreateShaderBindingInstance();
    m_shaderBindingInstanceArray.Add(OneRenderObjectUniformBuffer.get());

    if (m_material) {
      m_shaderBindingInstanceArray.Add(
          m_material->CreateShaderBindingInstance().get());
    }
  }

  // Gather ShaderBindings
  ShaderBindingLayoutArray shaderBindingLayoutArray;
  for (int32_t i = 0; i < m_shaderBindingInstanceArray.NumOfData; ++i) {
    // Add DescriptorSetLayout data
    shaderBindingLayoutArray.Add(
        m_shaderBindingInstanceArray[i]->ShaderBindingsLayouts);

    // Add shaderBindingInstanceCombiner data : DescriptorSets, DynamicOffsets
    shaderBindingInstanceCombiner.DescriptorSetHandles.Add(
        m_shaderBindingInstanceArray[i]->GetHandle());
    const std::vector<uint32_t>* pDynamicOffsetTest
        = m_shaderBindingInstanceArray[i]->GetDynamicOffsets();
    if (pDynamicOffsetTest && pDynamicOffsetTest->size()) {
      shaderBindingInstanceCombiner.DynamicOffsets.Add(
          (void*)pDynamicOffsetTest->data(),
          (int32_t)pDynamicOffsetTest->size());
    }
  }
  shaderBindingInstanceCombiner.m_shaderBindingInstanceArray
      = &m_shaderBindingInstanceArray;

  const auto& RenderObjectGeoDataPtr = RenderObject->GeometryDataPtr;

  VertexBufferArray vertexBufferArray;
  vertexBufferArray.Add(
      InIsPositionOnly
          ? RenderObjectGeoDataPtr->VertexBuffer_PositionOnlyPtr.get()
          : RenderObjectGeoDataPtr->VertexBufferPtr.get());
  if (OverrideInstanceData) {
    vertexBufferArray.Add(OverrideInstanceData);
  } else if (RenderObjectGeoDataPtr->VertexBuffer_InstanceDataPtr) {
    vertexBufferArray.Add(
        RenderObjectGeoDataPtr->VertexBuffer_InstanceDataPtr.get());
  }

  // Create Pipeline
  CurrentPipelineStateInfo
      = (PipelineStateInfo*)g_rhi->CreatePipelineStateInfo(
          PipelineStateFixed,
          Shader,
          vertexBufferArray,
          m_renderPass,
          shaderBindingLayoutArray,
          m_pushConstant,
          SubpassIndex);

  IsPositionOnly = InIsPositionOnly;
}

void DrawCommand::Draw() const {
  assert(RenderFrameContextPtr);

  g_rhi->BindGraphicsShaderBindingInstances(
      RenderFrameContextPtr->GetActiveCommandBuffer(),
      CurrentPipelineStateInfo,
      shaderBindingInstanceCombiner,
      0);

  // Bind the image that contains the shading rate patterns
#if USE_VARIABLE_SHADING_RATE_TIER2
  if (gOptions.UseVRS) {
    // TODO: change g_rhi1_vk
    g_rhi1_vk->BindShadingRateImage(
        RenderFrameContextPtr->GetActiveCommandBuffer(),
        g_rhi1_vk->GetSampleVRSTexture());
  }
#endif

  // Bind Pipeline
  CurrentPipelineStateInfo->Bind(RenderFrameContextPtr);

  // this is only for Vulkan for now
  // TODO: add check to Vulkan
  bool isUseVulkan = false;
  if (isUseVulkan) {
    if (m_pushConstant && m_pushConstant->IsValid()) {
      const ResourceContainer<PushConstantRange>* pushConstantRanges
          = m_pushConstant->GetPushConstantRanges();
      assert(pushConstantRanges);
      if (pushConstantRanges) {
        for (int32_t i = 0; i < pushConstantRanges->NumOfData; ++i) {
          const PushConstantRange& range = (*pushConstantRanges)[i];
          vkCmdPushConstants(
              (VkCommandBuffer)RenderFrameContextPtr->GetActiveCommandBuffer()
                  ->GetNativeHandle(),
              ((PipelineStateInfoVk*)CurrentPipelineStateInfo)
                  ->vkPipelineLayout,
              GetVulkanShaderAccessFlags(range.AccessStageFlag),
              range.Offset,
              range.Size,
              m_pushConstant->GetConstantData());
        }
      }
    }
  }
  RenderObject->BindBuffers(
      RenderFrameContextPtr, IsPositionOnly, OverrideInstanceData);

  // Draw
  const auto&          RenderObjectGeoDataPtr = RenderObject->GeometryDataPtr;
  const VertexBuffer* InstanceData
      = OverrideInstanceData
          ? OverrideInstanceData
          : RenderObjectGeoDataPtr->VertexBuffer_InstanceDataPtr.get();
  const int32_t InstanceCount
      = InstanceData ? InstanceData->GetElementCount() : 1;
  RenderObject->Draw(RenderFrameContextPtr, InstanceCount);
}

}  // namespace game_engine
