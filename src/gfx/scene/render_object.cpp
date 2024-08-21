
// TODO:
// - move to more appropriate folder (e.g. scene)
// - consider moving scene folder to a more appropriate location (gfx is not the
// very best place for it)

#include "gfx/scene/render_object.h"

#include "gfx/rhi/rhi.h"

namespace game_engine {

// RenderObjectGeometryData
// ====================================================

void RenderObjectGeometryData::create(
    const std::shared_ptr<VertexStreamData>& vertexStream,
    const std::shared_ptr<IndexStreamData>&  indexStream,
    bool                                     hasVertexColor,
    bool                                     hasVertexBiTangent) {
  m_vertexStreamPtr_ = vertexStream;
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

  m_vertexBufferPtr_ = g_rhi->createVertexBuffer(m_vertexStreamPtr_);
  m_vertexBufferPositionOnlyPtr_
      = g_rhi->createVertexBuffer(m_vertexStreamPositionOnlyPtr_);
  m_indexBufferPtr_ = g_rhi->createIndexBuffer(m_indexStreamPtr_);

  m_hasVertexColor_     = hasVertexColor;
  m_hasVertexBiTangent_ = hasVertexBiTangent;
}

void RenderObjectGeometryData::createNewForRaytracing(
    const std::shared_ptr<VertexStreamData>& vertexStream,
    const std::shared_ptr<VertexStreamData>& vertexStream_PositionOnly,
    const std::shared_ptr<IndexStreamData>&  indexStream,
    bool                                     hasVertexColor,
    bool                                     hasVertexBiTangent) {
  m_vertexStreamPtr_             = vertexStream;
  m_vertexStreamPositionOnlyPtr_ = vertexStream_PositionOnly;
  m_indexStreamPtr_              = indexStream;

  m_vertexBufferPtr_ = g_rhi->createVertexBuffer(m_vertexStreamPtr_);
  m_vertexBufferPositionOnlyPtr_
      = g_rhi->createVertexBuffer(m_vertexStreamPositionOnlyPtr_);
  m_indexBufferPtr_ = g_rhi->createIndexBuffer(m_indexStreamPtr_);

  m_hasVertexColor_     = hasVertexColor;
  m_hasVertexBiTangent_ = hasVertexBiTangent;
}

// Vertex buffers
void RenderObjectGeometryData::updateVertexStream(
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

void RenderObject::draw(
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
      g_rhi->drawElementsIndirect(
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
        g_rhi->drawElementsBaseVertex(
            renderFrameContext,
            // primitiveType, // TODO: remove (not used)
            static_cast<int32_t>(indexStreamData->m_stream_->getElementSize()),
            startIndex,
            indexCount,
            startVertex);
      } else {
        g_rhi->drawElementsInstancedBaseVertex(
            renderFrameContext,
            // primitiveType, // TODO: remove (not used)
            static_cast<int32_t>(indexStreamData->m_stream_->getElementSize()),
            startIndex,
            indexCount,
            startVertex,
            instanceCount);
      }
    }
  } else {
    if (m_geometryDataPtr_->m_indirectCommandBufferPtr_) {
      g_rhi->drawIndirect(renderFrameContext,
                          // primitiveType, // TODO: remove (not used)
                          m_geometryDataPtr_->m_indirectCommandBufferPtr_.get(),
                          startVertex,
                          instanceCount);
    } else {
      vertexCount = vertexCount != -1
                      ? vertexCount
                      : m_geometryDataPtr_->m_vertexStreamPtr_->m_elementCount_;
      if (instanceCount <= 0) {
        g_rhi->drawArrays(renderFrameContext, startVertex, vertexCount);
      } else {
        g_rhi->drawArraysInstanced(renderFrameContext,
                                   // primitiveType, // TODO: remove (not used
                                   startVertex,
                                   vertexCount,
                                   instanceCount);
      }
    }
  }
}

void RenderObject::bindBuffers(
    const std::shared_ptr<RenderFrameContext>& renderFrameContext,
    bool                                       positionOnly,
    const VertexBuffer*                        overrideInstanceData) const {
  if (positionOnly) {
    if (m_geometryDataPtr_->m_vertexBufferPositionOnlyPtr_) {
      m_geometryDataPtr_->m_vertexBufferPositionOnlyPtr_->bind(
          renderFrameContext);
    }
  } else {
    if (m_geometryDataPtr_->m_vertexBufferPtr_) {
      m_geometryDataPtr_->m_vertexBufferPtr_->bind(renderFrameContext);
    }
  }

  if (overrideInstanceData) {
    overrideInstanceData->bind(renderFrameContext);
  } else if (m_geometryDataPtr_->m_vertexBufferInstanceDataPtr_) {
    m_geometryDataPtr_->m_vertexBufferInstanceDataPtr_->bind(
        renderFrameContext);
  }

  if (m_geometryDataPtr_->m_indexBufferPtr_) {
    m_geometryDataPtr_->m_indexBufferPtr_->bind(renderFrameContext);
  }
}

const std::vector<float>& RenderObject::getVertices() const {
  if (m_geometryDataPtr_->m_vertexStreamPtr_
      && !m_geometryDataPtr_->m_vertexStreamPtr_->m_streams_.empty()) {
    return static_cast<BufferAttributeStream<float>*>(
               m_geometryDataPtr_->m_vertexStreamPtr_->m_streams_[0].get())
        ->m_data_;
  }

  static const std::vector<float> s_kEmtpy;
  return s_kEmtpy;
}

void RenderObject::updateWorldMatrix() {
  if (static_cast<int32_t>(m_dirtyFlags_)
      & static_cast<int32_t>(EDirty::POS_ROT_SCALE)) {
    auto posMatrix   = math::g_translate(m_position_);
    auto rotMatrix   = math::g_rotateLh(m_rotation_);
    auto scaleMatrix = math::g_scale(m_scale_);
    m_world_         = posMatrix * rotMatrix * scaleMatrix;

    m_needToUpdateRenderObjectUniformParameters_ = true;

    clearDirtyFlags_(EDirty::POS_ROT_SCALE);
  }
}

const std::shared_ptr<ShaderBindingInstance>&
    RenderObject::createShaderBindingInstance() {
  // update uniform buffer if it need to.
  if (m_needToUpdateRenderObjectUniformParameters_) {
    m_needToUpdateRenderObjectUniformParameters_ = false;

    RenderObjectUniformBuffer ubo;
    ubo.m_matrix    = m_world_;
    ubo.m_invMatrix = ubo.m_matrix.inverse();

    m_renderObjectUniformParametersPtr_
        = std::shared_ptr<IUniformBufferBlock>(g_rhi->createUniformBufferBlock(
            NameStatic("RenderObjectUniformParameters"),
            LifeTimeType::MultiFrame,
            sizeof(RenderObjectUniformBuffer)));
    m_renderObjectUniformParametersPtr_->updateBufferData(&ubo, sizeof(ubo));

    m_testUniformBuffer_ = g_rhi->createStructuredBuffer(
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

    shaderBindingArray.add(
        ShaderBinding(BindingPoint++,
                      1,
                      EShaderBindingType::UNIFORMBUFFER_DYNAMIC,
                      false,
                      EShaderAccessStageFlag::ALL_GRAPHICS,
                      ResourceInlineAllactor.alloc<UniformBufferResource>(
                          m_renderObjectUniformParametersPtr_.get())));

    if (m_renderObjectShaderBindingInstance_) {
      m_renderObjectShaderBindingInstance_->free();
    }

    m_renderObjectShaderBindingInstance_ = g_rhi->createShaderBindingInstance(
        shaderBindingArray, ShaderBindingInstanceType::MultiFrame);
    assert(m_renderObjectShaderBindingInstance_.get());
  }

  // shaderBindingArray.add(m_bindingPoint_++, 1,
  // VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
  // VK_SHADER_STAGE_ALL_GRAPHICS 	,
  // ResourceInlineAllactor.alloc<UniformBufferResource>(&RenderObjectUniformParameters));

  //   return g_rhi->createShaderBindingInstance(shaderBindingArray);

  return m_renderObjectShaderBindingInstance_;
}

}  // namespace game_engine
