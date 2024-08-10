
// TODO:
// - move to more appropriate folder (e.g. scene)
// - consider moving scene folder to a more appropriate location (gfx is not the
// very best place for it)

#include "gfx/scene/render_object.h"

#include "gfx/rhi/rhi.h"

namespace game_engine {

// RenderObjectGeometryData
// ====================================================

void RenderObjectGeometryData::Create(
    const std::shared_ptr<VertexStreamData>& InVertexStream,
    const std::shared_ptr<IndexStreamData>&  InIndexStream,
    bool                                     InHasVertexColor,
    bool                                     InHasVertexBiTangent) {
  VertexStreamPtr = InVertexStream;
  IndexStreamPtr  = InIndexStream;

  assert(VertexStreamPtr->streams.size());
  if (VertexStreamPtr && VertexStreamPtr->streams.size()) {
    VertexStream_PositionOnlyPtr = std::make_shared<VertexStreamData>();
    VertexStream_PositionOnlyPtr->streams.push_back(
        VertexStreamPtr->streams[0]);
    VertexStream_PositionOnlyPtr->PrimitiveType
        = VertexStreamPtr->PrimitiveType;
    VertexStream_PositionOnlyPtr->elementCount = VertexStreamPtr->elementCount;
  }

  VertexBufferPtr = g_rhi->CreateVertexBuffer(VertexStreamPtr);
  VertexBuffer_PositionOnlyPtr
      = g_rhi->CreateVertexBuffer(VertexStream_PositionOnlyPtr);
  IndexBufferPtr = g_rhi->CreateIndexBuffer(IndexStreamPtr);

  bHasVertexColor     = InHasVertexColor;
  bHasVertexBiTangent = InHasVertexBiTangent;
}

void RenderObjectGeometryData::CreateNew_ForRaytracing(
    const std::shared_ptr<VertexStreamData>& InVertexStream,
    const std::shared_ptr<VertexStreamData>& InVertexStream_PositionOnly,
    const std::shared_ptr<IndexStreamData>&  InIndexStream,
    bool                                     InHasVertexColor,
    bool                                     InHasVertexBiTangent) {
  VertexStreamPtr              = InVertexStream;
  VertexStream_PositionOnlyPtr = InVertexStream_PositionOnly;
  IndexStreamPtr               = InIndexStream;

  VertexBufferPtr = g_rhi->CreateVertexBuffer(VertexStreamPtr);
  VertexBuffer_PositionOnlyPtr
      = g_rhi->CreateVertexBuffer(VertexStream_PositionOnlyPtr);
  IndexBufferPtr = g_rhi->CreateIndexBuffer(IndexStreamPtr);

  bHasVertexColor     = InHasVertexColor;
  bHasVertexBiTangent = InHasVertexBiTangent;
}

// Vertex buffers
void RenderObjectGeometryData::UpdateVertexStream(
    const std::shared_ptr<VertexStreamData>& vertexStream) {
  VertexStreamPtr = vertexStream;

  assert(VertexStreamPtr->streams.size());
  if (VertexStreamPtr && VertexStreamPtr->streams.size()) {
    VertexStream_PositionOnlyPtr = std::make_shared<VertexStreamData>();
    VertexStream_PositionOnlyPtr->streams.push_back(
        VertexStreamPtr->streams[0]);
    VertexStream_PositionOnlyPtr->PrimitiveType
        = VertexStreamPtr->PrimitiveType;
    VertexStream_PositionOnlyPtr->elementCount = VertexStreamPtr->elementCount;
  }
}

// RenderObject
// ====================================================

void RenderObject::Draw(
    const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
    int32_t                                     startIndex,
    int32_t                                     indexCount,
    int32_t                                     startVertex,
    int32_t                                     vertexCount,
    int32_t                                     instanceCount) {
  if (!GeometryDataPtr) {
    return;
  }

  startIndex = startIndex != -1 ? startIndex : 0;

  // TODO: remove (not used)
  // const EPrimitiveType primitiveType = GetPrimitiveType();
  if (GeometryDataPtr->IndexBufferPtr) {
    if (GeometryDataPtr->IndirectCommandBufferPtr) {
      g_rhi->DrawElementsIndirect(
          InRenderFrameContext,
          // primitiveType, // TODO: remove (not used)
          GeometryDataPtr->IndirectCommandBufferPtr.get(),
          startIndex,
          instanceCount);
    } else {
      auto& indexStreamData = GeometryDataPtr->IndexBufferPtr->indexStreamData;
      indexCount
          = indexCount != -1 ? indexCount : indexStreamData->elementCount;
      if (instanceCount <= 0) {
        g_rhi->DrawElementsBaseVertex(
            InRenderFrameContext,
            // primitiveType, // TODO: remove (not used)
            static_cast<int32_t>(indexStreamData->stream->GetElementSize()),
            startIndex,
            indexCount,
            startVertex);
      } else {
        g_rhi->DrawElementsInstancedBaseVertex(
            InRenderFrameContext,
            // primitiveType, // TODO: remove (not used)
            static_cast<int32_t>(indexStreamData->stream->GetElementSize()),
            startIndex,
            indexCount,
            startVertex,
            instanceCount);
      }
    }
  } else {
    if (GeometryDataPtr->IndirectCommandBufferPtr) {
      g_rhi->DrawIndirect(InRenderFrameContext,
                          // primitiveType, // TODO: remove (not used)
                          GeometryDataPtr->IndirectCommandBufferPtr.get(),
                          startVertex,
                          instanceCount);
    } else {
      vertexCount = vertexCount != -1
                      ? vertexCount
                      : GeometryDataPtr->VertexStreamPtr->elementCount;
      if (instanceCount <= 0) {
        g_rhi->DrawArrays(InRenderFrameContext, startVertex, vertexCount);
      } else {
        g_rhi->DrawArraysInstanced(InRenderFrameContext,
                                   // primitiveType, // TODO: remove (not used
                                   startVertex,
                                   vertexCount,
                                   instanceCount);
      }
    }
  }
}

void RenderObject::BindBuffers(
    const std::shared_ptr<jRenderFrameContext>& InRenderFrameContext,
    bool                                        InPositionOnly,
    const jVertexBuffer*                        InOverrideInstanceData) const {
  if (InPositionOnly) {
    if (GeometryDataPtr->VertexBuffer_PositionOnlyPtr) {
      GeometryDataPtr->VertexBuffer_PositionOnlyPtr->Bind(InRenderFrameContext);
    }
  } else {
    if (GeometryDataPtr->VertexBufferPtr) {
      GeometryDataPtr->VertexBufferPtr->Bind(InRenderFrameContext);
    }
  }

  if (InOverrideInstanceData) {
    InOverrideInstanceData->Bind(InRenderFrameContext);
  } else if (GeometryDataPtr->VertexBuffer_InstanceDataPtr) {
    GeometryDataPtr->VertexBuffer_InstanceDataPtr->Bind(InRenderFrameContext);
  }

  if (GeometryDataPtr->IndexBufferPtr) {
    GeometryDataPtr->IndexBufferPtr->Bind(InRenderFrameContext);
  }
}

const std::vector<float>& RenderObject::GetVertices() const {
  if (GeometryDataPtr->VertexStreamPtr
      && !GeometryDataPtr->VertexStreamPtr->streams.empty()) {
    return static_cast<BufferAttributeStream<float>*>(
               GeometryDataPtr->VertexStreamPtr->streams[0].get())
        ->Data;
  }

  static const std::vector<float> s_emtpy;
  return s_emtpy;
}

void RenderObject::UpdateWorldMatrix() {
  if (static_cast<int32_t>(DirtyFlags)
      & static_cast<int32_t>(EDirty::POS_ROT_SCALE)) {
    auto posMatrix   = math::g_translate(Pos);
    auto rotMatrix   = math::g_rotateLh(Rot);
    auto scaleMatrix = math::g_scale(Scale);
    World            = posMatrix * rotMatrix * scaleMatrix;

    NeedToUpdateRenderObjectUniformParameters = true;

    ClearDirtyFlags(EDirty::POS_ROT_SCALE);
  }
}

const std::shared_ptr<jShaderBindingInstance>&
    RenderObject::CreateShaderBindingInstance() {
  // Update uniform buffer if it need to.
  if (NeedToUpdateRenderObjectUniformParameters) {
    NeedToUpdateRenderObjectUniformParameters = false;

    RenderObjectUniformBuffer ubo;
    ubo.M    = World;
    ubo.InvM = ubo.M.inverse();

    RenderObjectUniformParametersPtr
        = std::shared_ptr<IUniformBufferBlock>(g_rhi->CreateUniformBufferBlock(
            NameStatic("RenderObjectUniformParameters"),
            LifeTimeType::MultiFrame,
            sizeof(RenderObjectUniformBuffer)));
    RenderObjectUniformParametersPtr->UpdateBufferData(&ubo, sizeof(ubo));

    TestUniformBuffer = g_rhi->CreateStructuredBuffer(
        sizeof(ubo),
        0,
        sizeof(ubo),
        EBufferCreateFlag::UAV | EBufferCreateFlag::ShaderBindingTable,
        EResourceLayout::GENERAL,
        &ubo,
        sizeof(ubo));

    int32_t                               BindingPoint = 0;
    jShaderBindingArray                   ShaderBindingArray;
    jShaderBindingResourceInlineAllocator ResourceInlineAllactor;

    ShaderBindingArray.Add(
        jShaderBinding(BindingPoint++,
                       1,
                       EShaderBindingType::UNIFORMBUFFER_DYNAMIC,
                        EShaderAccessStageFlag::ALL_GRAPHICS,
                       EShaderAccessStageFlag::ALL_GRAPHICS,
                       ResourceInlineAllactor.Alloc<jUniformBufferResource>(
                           RenderObjectUniformParametersPtr.get())));

    if (RenderObjectShaderBindingInstance) {
      RenderObjectShaderBindingInstance->Free();
    }

    RenderObjectShaderBindingInstance = g_rhi->CreateShaderBindingInstance(
        ShaderBindingArray, jShaderBindingInstanceType::MultiFrame);
    assert(RenderObjectShaderBindingInstance.get());
  }

  // ShaderBindingArray.Add(BindingPoint++, 1,
  // VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
  // VK_SHADER_STAGE_ALL_GRAPHICS 	,
  // ResourceInlineAllactor.Alloc<UniformBufferResource>(&RenderObjectUniformParameters));

  //   return g_rhi->CreateShaderBindingInstance(ShaderBindingArray);

  return RenderObjectShaderBindingInstance;
}

}  // namespace game_engine
