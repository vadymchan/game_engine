
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
    const std::shared_ptr<IndexStreamData>&  indexStream,
    bool                                     InHasVertexColor,
    bool                                     InHasVertexBiTangent) {
  m_vertexStreamPtr_ = InVertexStream;
  m_indexStreamPtr_  = indexStream;

  assert(m_vertexStreamPtr_->m_streams_.size());
  if (m_vertexStreamPtr_ && m_vertexStreamPtr_->m_streams_.size()) {
    m_vertexStreamPositionOnlyPtr_ = std::make_shared<VertexStreamData>();
    m_vertexStreamPositionOnlyPtr_->m_streams_.push_back(
        m_vertexStreamPtr_->m_streams_[0]);
    m_vertexStreamPositionOnlyPtr_->m_primitiveType_
        = m_vertexStreamPtr_->m_primitiveType_;
    m_vertexStreamPositionOnlyPtr_->m_elementCount_
        = m_vertexStreamPtr_->m_elementCount_;
  }

  m_vertexBufferPtr_ = g_rhi->CreateVertexBuffer(m_vertexStreamPtr_);
  m_vertexBufferPositionOnlyPtr_
      = g_rhi->CreateVertexBuffer(m_vertexStreamPositionOnlyPtr_);
  m_indexBufferPtr_ = g_rhi->CreateIndexBuffer(m_indexStreamPtr_);

  m_hasVertexColor_     = InHasVertexColor;
  m_hasVertexBiTangent_ = InHasVertexBiTangent;
}

void RenderObjectGeometryData::CreateNew_ForRaytracing(
    const std::shared_ptr<VertexStreamData>& InVertexStream,
    const std::shared_ptr<VertexStreamData>& InVertexStream_PositionOnly,
    const std::shared_ptr<IndexStreamData>&  indexStream,
    bool                                     InHasVertexColor,
    bool                                     InHasVertexBiTangent) {
  m_vertexStreamPtr_             = InVertexStream;
  m_vertexStreamPositionOnlyPtr_ = InVertexStream_PositionOnly;
  m_indexStreamPtr_              = indexStream;

  m_vertexBufferPtr_ = g_rhi->CreateVertexBuffer(m_vertexStreamPtr_);
  m_vertexBufferPositionOnlyPtr_
      = g_rhi->CreateVertexBuffer(m_vertexStreamPositionOnlyPtr_);
  m_indexBufferPtr_ = g_rhi->CreateIndexBuffer(m_indexStreamPtr_);

  m_hasVertexColor_     = InHasVertexColor;
  m_hasVertexBiTangent_ = InHasVertexBiTangent;
}

// Vertex buffers
void RenderObjectGeometryData::UpdateVertexStream(
    const std::shared_ptr<VertexStreamData>& vertexStream) {
  m_vertexStreamPtr_ = vertexStream;

  assert(m_vertexStreamPtr_->m_streams_.size());
  if (m_vertexStreamPtr_ && m_vertexStreamPtr_->m_streams_.size()) {
    m_vertexStreamPositionOnlyPtr_ = std::make_shared<VertexStreamData>();
    m_vertexStreamPositionOnlyPtr_->m_streams_.push_back(
        m_vertexStreamPtr_->m_streams_[0]);
    m_vertexStreamPositionOnlyPtr_->m_primitiveType_
        = m_vertexStreamPtr_->m_primitiveType_;
    m_vertexStreamPositionOnlyPtr_->m_elementCount_
        = m_vertexStreamPtr_->m_elementCount_;
  }
}

// RenderObject
// ====================================================

void RenderObject::Draw(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
    int32_t                                    startIndex,
    int32_t                                    indexCount,
    int32_t                                    startVertex,
    int32_t                                    vertexCount,
    int32_t                                    instanceCount) {
  if (!m_geometryDataPtr_) {
    return;
  }

  startIndex = startIndex != -1 ? startIndex : 0;

  // TODO: remove (not used)
  // const EPrimitiveType primitiveType = GetPrimitiveType();
  if (m_geometryDataPtr_->m_indexBufferPtr_) {
    if (m_geometryDataPtr_->m_indirectCommandBufferPtr_) {
      g_rhi->DrawElementsIndirect(
          renderFrameContext,
          // primitiveType, // TODO: remove (not used)
          m_geometryDataPtr_->m_indirectCommandBufferPtr_.get(),
          startIndex,
          instanceCount);
    } else {
      auto& indexStreamData
          = m_geometryDataPtr_->m_indexBufferPtr_->m_indexStreamData_;
      indexCount
          = indexCount != -1 ? indexCount : indexStreamData->m_elementCount_;
      if (instanceCount <= 0) {
        g_rhi->DrawElementsBaseVertex(
            renderFrameContext,
            // primitiveType, // TODO: remove (not used)
            static_cast<int32_t>(indexStreamData->m_stream_->GetElementSize()),
            startIndex,
            indexCount,
            startVertex);
      } else {
        g_rhi->DrawElementsInstancedBaseVertex(
            renderFrameContext,
            // primitiveType, // TODO: remove (not used)
            static_cast<int32_t>(indexStreamData->m_stream_->GetElementSize()),
            startIndex,
            indexCount,
            startVertex,
            instanceCount);
      }
    }
  } else {
    if (m_geometryDataPtr_->m_indirectCommandBufferPtr_) {
      g_rhi->DrawIndirect(renderFrameContext,
                          // primitiveType, // TODO: remove (not used)
                          m_geometryDataPtr_->m_indirectCommandBufferPtr_.get(),
                          startVertex,
                          instanceCount);
    } else {
      vertexCount = vertexCount != -1
                      ? vertexCount
                      : m_geometryDataPtr_->m_vertexStreamPtr_->m_elementCount_;
      if (instanceCount <= 0) {
        g_rhi->DrawArrays(renderFrameContext, startVertex, vertexCount);
      } else {
        g_rhi->DrawArraysInstanced(renderFrameContext,
                                   // primitiveType, // TODO: remove (not used
                                   startVertex,
                                   vertexCount,
                                   instanceCount);
      }
    }
  }
}

void RenderObject::BindBuffers(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
    bool                                       InPositionOnly,
    const VertexBuffer*                        InOverrideInstanceData) const {
  if (InPositionOnly) {
    if (m_geometryDataPtr_->m_vertexBufferPositionOnlyPtr_) {
      m_geometryDataPtr_->m_vertexBufferPositionOnlyPtr_->Bind(
          renderFrameContext);
    }
  } else {
    if (m_geometryDataPtr_->m_vertexBufferPtr_) {
      m_geometryDataPtr_->m_vertexBufferPtr_->Bind(renderFrameContext);
    }
  }

  if (InOverrideInstanceData) {
    InOverrideInstanceData->Bind(renderFrameContext);
  } else if (m_geometryDataPtr_->m_vertexBufferInstanceDataPtr_) {
    m_geometryDataPtr_->m_vertexBufferInstanceDataPtr_->Bind(
        renderFrameContext);
  }

  if (m_geometryDataPtr_->m_indexBufferPtr_) {
    m_geometryDataPtr_->m_indexBufferPtr_->Bind(renderFrameContext);
  }
}

const std::vector<float>& RenderObject::GetVertices() const {
  if (m_geometryDataPtr_->m_vertexStreamPtr_
      && !m_geometryDataPtr_->m_vertexStreamPtr_->m_streams_.empty()) {
    return static_cast<BufferAttributeStream<float>*>(
               m_geometryDataPtr_->m_vertexStreamPtr_->m_streams_[0].get())
        ->m_data_;
  }

  static const std::vector<float> s_kEmtpy;
  return s_kEmtpy;
}

void RenderObject::UpdateWorldMatrix() {
  if (static_cast<int32_t>(m_dirtyFlags_)
      & static_cast<int32_t>(EDirty::POS_ROT_SCALE)) {
    auto posMatrix   = math::g_translate(m_position_);
    auto rotMatrix   = math::g_rotateLh(m_rotation_);
    auto scaleMatrix = math::g_scale(m_scale_);
    m_world_         = posMatrix * rotMatrix * scaleMatrix;

    m_needToUpdateRenderObjectUniformParameters_ = true;

    ClearDirtyFlags(EDirty::POS_ROT_SCALE);
  }
}

const std::shared_ptr<ShaderBindingInstance>&
    RenderObject::CreateShaderBindingInstance() {
  // Update uniform buffer if it need to.
  if (m_needToUpdateRenderObjectUniformParameters_) {
    m_needToUpdateRenderObjectUniformParameters_ = false;

    RenderObjectUniformBuffer ubo;
    ubo.m_matrix    = m_world_;
    ubo.m_invMatrix = ubo.m_matrix.inverse();

    m_renderObjectUniformParametersPtr_
        = std::shared_ptr<IUniformBufferBlock>(g_rhi->CreateUniformBufferBlock(
            NameStatic("RenderObjectUniformParameters"),
            LifeTimeType::MultiFrame,
            sizeof(RenderObjectUniformBuffer)));
    m_renderObjectUniformParametersPtr_->UpdateBufferData(&ubo, sizeof(ubo));

    m_testUniformBuffer_ = g_rhi->CreateStructuredBuffer(
        sizeof(ubo),
        0,
        sizeof(ubo),
        EBufferCreateFlag::UAV | EBufferCreateFlag::ShaderBindingTable,
        EResourceLayout::GENERAL,
        &ubo,
        sizeof(ubo));

    int32_t                              BindingPoint = 0;
    ShaderBindingArray                   shaderBindingArray;
    ShaderBindingResourceInlineAllocator ResourceInlineAllactor;

    shaderBindingArray.Add(
        ShaderBinding(BindingPoint++,
                      1,
                      EShaderBindingType::UNIFORMBUFFER_DYNAMIC,
                      false,
                      EShaderAccessStageFlag::ALL_GRAPHICS,
                      ResourceInlineAllactor.Alloc<UniformBufferResource>(
                          m_renderObjectUniformParametersPtr_.get())));

    if (m_renderObjectShaderBindingInstance_) {
      m_renderObjectShaderBindingInstance_->Free();
    }

    m_renderObjectShaderBindingInstance_ = g_rhi->CreateShaderBindingInstance(
        shaderBindingArray, ShaderBindingInstanceType::MultiFrame);
    assert(m_renderObjectShaderBindingInstance_.get());
  }

  // shaderBindingArray.Add(m_bindingPoint_++, 1,
  // VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
  // VK_SHADER_STAGE_ALL_GRAPHICS 	,
  // ResourceInlineAllactor.Alloc<UniformBufferResource>(&RenderObjectUniformParameters));

  //   return g_rhi->CreateShaderBindingInstance(shaderBindingArray);

  return m_renderObjectShaderBindingInstance_;
}

}  // namespace game_engine
