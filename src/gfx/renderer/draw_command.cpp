
#include "gfx/renderer/draw_command.h"

#include "gfx/rhi/rhi.h"
#include "gfx/rhi/vulkan/rhi_vk.h"


namespace game_engine {


  void DrawCommand::PrepareToDraw(bool InIsPositionOnly) {
    if (!Test) {
      // GetShaderBindings
      if (view) {
        view->GetShaderBindingInstance(
            shaderBindingInstanceArray,
            RenderFrameContextPtr->UseForwardRenderer);
      }

      // GetShaderBindings
      OneRenderObjectUniformBuffer
          = RenderObject->CreateShaderBindingInstance();
      shaderBindingInstanceArray.Add(OneRenderObjectUniformBuffer.get());

      if (Material) {
        shaderBindingInstanceArray.Add(
            Material->CreateShaderBindingInstance().get());
      }
    }

    // Gather ShaderBindings
    jShaderBindingLayoutArray ShaderBindingLayoutArray;
    for (int32_t i = 0; i < shaderBindingInstanceArray.NumOfData; ++i) {
      // Add DescriptorSetLayout data
      ShaderBindingLayoutArray.Add(
          shaderBindingInstanceArray[i]->ShaderBindingsLayouts);

      // Add shaderBindingInstanceCombiner data : DescriptorSets, DynamicOffsets
      shaderBindingInstanceCombiner.DescriptorSetHandles.Add(
          shaderBindingInstanceArray[i]->GetHandle());
      const std::vector<uint32_t>* pDynamicOffsetTest
          = shaderBindingInstanceArray[i]->GetDynamicOffsets();
      if (pDynamicOffsetTest && pDynamicOffsetTest->size()) {
        shaderBindingInstanceCombiner.DynamicOffsets.Add(
            (void*)pDynamicOffsetTest->data(),
            (int32_t)pDynamicOffsetTest->size());
      }
    }
    shaderBindingInstanceCombiner.shaderBindingInstanceArray
        = &shaderBindingInstanceArray;

    const auto& RenderObjectGeoDataPtr = RenderObject->GeometryDataPtr;

    jVertexBufferArray VertexBufferArray;
    VertexBufferArray.Add(
        InIsPositionOnly
            ? RenderObjectGeoDataPtr->VertexBuffer_PositionOnlyPtr.get()
            : RenderObjectGeoDataPtr->VertexBufferPtr.get());
    if (OverrideInstanceData) {
      VertexBufferArray.Add(OverrideInstanceData);
    } else if (RenderObjectGeoDataPtr->VertexBuffer_InstanceDataPtr) {
      VertexBufferArray.Add(
          RenderObjectGeoDataPtr->VertexBuffer_InstanceDataPtr.get());
    }

    // Create Pipeline
    CurrentPipelineStateInfo
        = (jPipelineStateInfo*)g_rhi->CreatePipelineStateInfo(
            PipelineStateFixed,
            Shader,
            VertexBufferArray,
            RenderPass,
            ShaderBindingLayoutArray,
            PushConstant,
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
    if (PushConstant && PushConstant->IsValid()) {
      const ResourceContainer<jPushConstantRange>* pushConstantRanges
          = PushConstant->GetPushConstantRanges();
      assert(pushConstantRanges);
      if (pushConstantRanges) {
        for (int32_t i = 0; i < pushConstantRanges->NumOfData; ++i) {
          const jPushConstantRange& range = (*pushConstantRanges)[i];
          vkCmdPushConstants(
              (VkCommandBuffer)RenderFrameContextPtr->GetActiveCommandBuffer()
                  ->GetNativeHandle(),
              ((PipelineStateInfoVk*)CurrentPipelineStateInfo)
                  ->vkPipelineLayout,
              GetVulkanShaderAccessFlags(range.AccessStageFlag),
              range.Offset,
              range.Size,
              PushConstant->GetConstantData());
        }
      }
    }

    RenderObject->BindBuffers(
        RenderFrameContextPtr, IsPositionOnly, OverrideInstanceData);

    // Draw
    const auto& RenderObjectGeoDataPtr = RenderObject->GeometryDataPtr;
    const jVertexBuffer* InstanceData
        = OverrideInstanceData
            ? OverrideInstanceData
            : RenderObjectGeoDataPtr->VertexBuffer_InstanceDataPtr.get();
    const int32_t InstanceCount
        = InstanceData ? InstanceData->GetElementCount() : 1;
    RenderObject->Draw(RenderFrameContextPtr, InstanceCount);
  }


}  // namespace game_engine
