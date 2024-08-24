
// TODO:
// - move to more appropriate folder (e.g. scene)
// - consider moving scene folder to a more appropriate location (gfx is not the
// very best place for it)

#ifndef GAME_ENGINE_RENDER_OBJECT_H
#define GAME_ENGINE_RENDER_OBJECT_H

#include "gfx/renderer/material.h"
#include "gfx/rhi/buffer.h"
#include "gfx/rhi/i_uniform_buffer_block.h"
#include "utils/math/math_util.h"

#include <math_library/matrix.h>

#include <memory>
#include <vector>

namespace game_engine {

class RenderObjectGeometryData {
  public:
  RenderObjectGeometryData() = default;

  RenderObjectGeometryData(
      const std::shared_ptr<VertexStreamData>& vertexStream,
      const std::shared_ptr<IndexStreamData>&  indexStream) {
    create(vertexStream, indexStream);
  }

  RenderObjectGeometryData(
      const std::shared_ptr<VertexStreamData>& vertexStream,
      const std::shared_ptr<VertexStreamData>& positionOnlyVertexStream,
      const std::shared_ptr<IndexStreamData>&  indexStream) {
    createNewForRaytracing(
        vertexStream, positionOnlyVertexStream, indexStream);
  }

  ~RenderObjectGeometryData() {
    m_vertexStreamPtr_.reset();
    m_vertexStreamPositionOnlyPtr_.reset();
  }

  void create(const std::shared_ptr<VertexStreamData>& vertexStream,
              const std::shared_ptr<IndexStreamData>&  indexStream,
              bool                                     hasVertexColor = true,
              bool hasVertexBiTangent = false);

  void createNewForRaytracing(
      const std::shared_ptr<VertexStreamData>& vertexStream,
      const std::shared_ptr<VertexStreamData>& vertexStream_PositionOnly,
      const std::shared_ptr<IndexStreamData>&  indexStream,
      bool                                     hasVertexColor     = false,
      bool                                     hasVertexBiTangent = true);

  // Vertex buffers
  void updateVertexStream(
      const std::shared_ptr<VertexStreamData>& vertexStream);

  EPrimitiveType getPrimitiveType() const {
    return m_vertexStreamPtr_ ? m_vertexStreamPtr_->m_primitiveType_
                              : EPrimitiveType::MAX;
  }

  bool hasInstancing() const { return !!m_vertexBufferInstanceDataPtr_; }

  bool hasVertexColor() const { return m_hasVertexColor_; }

  bool hasVertexBiTangent() const { return m_hasVertexBiTangent_; }

  std::shared_ptr<VertexStreamData> m_vertexStreamPtr_;
  std::shared_ptr<VertexStreamData> m_vertexStreamInstanceDataPtr_;
  std::shared_ptr<VertexStreamData> m_vertexStreamPositionOnlyPtr_;

  // Index buffer
  std::shared_ptr<IndexStreamData> m_indexStreamPtr_;

  std::shared_ptr<VertexBuffer> m_vertexBufferPtr_;
  std::shared_ptr<VertexBuffer> m_vertexBufferPositionOnlyPtr_;
  std::shared_ptr<VertexBuffer> m_vertexBufferInstanceDataPtr_;
  std::shared_ptr<IndexBuffer>  m_indexBufferPtr_;

  // IndirectCommand buffer
  std::shared_ptr<IBuffer> m_indirectCommandBufferPtr_;

  bool m_hasVertexColor_     = true;
  bool m_hasVertexBiTangent_ = false;
};

class RenderObject {
  public:
  RenderObject() {}

  virtual ~RenderObject() {}

  virtual void createRenderObject(
      const std::shared_ptr<RenderObjectGeometryData>&
          renderObjectGeometryData) {
    m_geometryDataPtr_ = renderObjectGeometryData;
    updateWorldMatrix();
  }

  virtual void draw(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      int32_t                                    startIndex,
      int32_t                                    indexCount,
      int32_t                                    startVertex,
      int32_t                                    vertexCount,
      int32_t                                    instanceCount);

  virtual void draw(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      int32_t                                    instanceCount = 1) {
    draw(renderFrameContext, 0, -1, 0, -1, instanceCount);
  }

  EPrimitiveType getPrimitiveType() const {
    return m_geometryDataPtr_->getPrimitiveType();
  }

  virtual void bindBuffers(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      bool                                       positionOnly,
      const VertexBuffer* overrideInstanceData = nullptr) const;

  const std::vector<float>& getVertices() const;

  bool hasInstancing() const { return m_geometryDataPtr_->hasInstancing(); }

  // TODO: currently not needed, uncomment or delete
  /*virtual bool IsSupportRaytracing() const {
    if (!m_geometryDataPtr_) {
      return false;
    }

    if (!m_geometryDataPtr_->m_vertexBufferPositionOnlyPtr_) {
      return false;
    }

    return
  m_geometryDataPtr_->m_vertexBufferPositionOnlyPtr_->IsSupportRaytracing();
  }*/

  void updateWorldMatrix();

  math::Matrix4f m_world_;

  std::shared_ptr<RenderObjectGeometryData> m_geometryDataPtr_;

  // TODO: seems it not used, consider removing
  std::shared_ptr<IBuffer> m_bottomLevelASBuffer_;
  std::shared_ptr<IBuffer> m_scratchASBuffer_;
  std::shared_ptr<IBuffer> m_vertexAndIndexOffsetBuffer_;


  // TODO: seems it not used, consider removing
  template <typename T>
  T* getBottomLevelASBuffer() const {
    return (T*)m_bottomLevelASBuffer_.get();
  }

  // TODO: seems it not used, consider removing
  template <typename T>
  T* getScratchASBuffer() const {
    return (T*)m_scratchASBuffer_.get();
  }

  // TODO: seems it not used, consider removing
  template <typename T>
  T* getVertexAndIndexOffsetBuffer() const {
    return (T*)m_vertexAndIndexOffsetBuffer_.get();
  }

  // TODO: consider use parameter name `value`
  void setPosition(const math::Vector3Df& position) {
    m_position_ = position;
    setDirtyFlags_(EDirty::POS);
  }

  // TODO: consider use parameter name `value`
  void setRotation(const math::Vector3Df& rotation) {
    m_rotation_ = rotation;
    setDirtyFlags_(EDirty::ROT);
  }

  // TODO: consider use parameter name `value`
  void setScale(const math::Vector3Df& scale) {
    m_scale_ = scale;
    setDirtyFlags_(EDirty::SCALE);
  }

  const math::Vector3Df& getPosition() const { return m_position_; }

  const math::Vector3Df& getRotation() const { return m_rotation_; }

  const math::Vector3Df& getScale() const { return m_scale_; }

  bool m_isTwoSided_       = false;
  bool m_isHiddenBoundBox_ = false;

  //////////////////////////////////////////////////////////////////////////

  struct RenderObjectUniformBuffer {
    math::Matrix4f m_matrix;
    math::Matrix4f m_invMatrix;
  };

  //////////////////////////////////////////////////////////////////////////
  // RenderObjectUniformBuffer
  virtual const std::shared_ptr<ShaderBindingInstance>&
      createShaderBindingInstance();

  //////////////////////////////////////////////////////////////////////////

  std::shared_ptr<Material> m_materialPtr_;
  // TODO: consider removing
  std::shared_ptr<IBuffer>   m_testUniformBuffer_;

  private:
  enum EDirty : int8_t {
    NONE          = 0,
    POS           = 1,
    ROT           = 1 << 1,
    SCALE         = 1 << 2,
    POS_ROT_SCALE = POS | ROT | SCALE,
  };

  EDirty m_dirtyFlags_ = EDirty::POS_ROT_SCALE;

  // TODO: conside renaming 
  void setDirtyFlags_(EDirty inEnum) {
    using T       = std::underlying_type<EDirty>::type;
    m_dirtyFlags_ = static_cast<EDirty>(static_cast<T>(inEnum)
                                        | static_cast<T>(m_dirtyFlags_));
  }

  void clearDirtyFlags_(EDirty inEnum) {
    using T       = std::underlying_type<EDirty>::type;
    m_dirtyFlags_ = static_cast<EDirty>(static_cast<T>(inEnum)
                                        & (!static_cast<T>(m_dirtyFlags_)));
  }

  void clearDirtyFlags_() { m_dirtyFlags_ = EDirty::NONE; }

  math::Vector3Df m_position_ = math::Vector3Df(0);
  math::Vector3Df m_rotation_ = math::Vector3Df(0);
  math::Vector3Df m_scale_    = math::Vector3Df(1);

  // TODO: consider renaming field
  bool m_needToUpdateRenderObjectUniformParameters_ = false;
  std::shared_ptr<IUniformBufferBlock>   m_renderObjectUniformParametersPtr_;
  std::shared_ptr<ShaderBindingInstance> m_renderObjectShaderBindingInstance_;

  // TODO: Special code for PBR test (currently not used)
  //float m_lastMetallic_  = 0.0f;
  //float m_lastRoughness_ = 0.0f;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_OBJECT_H
