
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
    Create(vertexStream, indexStream);
  }

  RenderObjectGeometryData(
      const std::shared_ptr<VertexStreamData>& vertexStream,
      const std::shared_ptr<VertexStreamData>& positionOnlyVertexStream,
      const std::shared_ptr<IndexStreamData>&  indexStream) {
    CreateNew_ForRaytracing(
        vertexStream, positionOnlyVertexStream, indexStream);
  }

  ~RenderObjectGeometryData() {
    m_vertexStreamPtr_.reset();
    m_vertexStreamPositionOnlyPtr_.reset();
  }

  void Create(const std::shared_ptr<VertexStreamData>& InVertexStream,
              const std::shared_ptr<IndexStreamData>&  indexStream,
              bool                                     InHasVertexColor = true,
              bool InHasVertexBiTangent = false);

  void CreateNew_ForRaytracing(
      const std::shared_ptr<VertexStreamData>& InVertexStream,
      const std::shared_ptr<VertexStreamData>& InVertexStream_PositionOnly,
      const std::shared_ptr<IndexStreamData>&  indexStream,
      bool                                     InHasVertexColor     = false,
      bool                                     InHasVertexBiTangent = true);

  // Vertex buffers
  void UpdateVertexStream(
      const std::shared_ptr<VertexStreamData>& vertexStream);

  EPrimitiveType GetPrimitiveType() const {
    return m_vertexStreamPtr_ ? m_vertexStreamPtr_->m_primitiveType_
                              : EPrimitiveType::MAX;
  }

  bool HasInstancing() const { return !!m_vertexBufferInstanceDataPtr_; }

  bool HasVertexColor() const { return m_hasVertexColor_; }

  bool HasVertexBiTangent() const { return m_hasVertexBiTangent_; }

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
  std::shared_ptr<Buffer> m_indirectCommandBufferPtr_;

  bool m_hasVertexColor_     = true;
  bool m_hasVertexBiTangent_ = false;
};

class RenderObject {
  public:
  RenderObject() {}

  virtual ~RenderObject() {}

  virtual void CreateRenderObject(
      const std::shared_ptr<RenderObjectGeometryData>&
          InRenderObjectGeometryData) {
    m_geometryDataPtr_ = InRenderObjectGeometryData;
    UpdateWorldMatrix();
  }

  virtual void Draw(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      int32_t                                    startIndex,
      int32_t                                    indexCount,
      int32_t                                    startVertex,
      int32_t                                    vertexCount,
      int32_t                                    instanceCount);

  virtual void Draw(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      int32_t                                    instanceCount = 1) {
    Draw(renderFrameContext, 0, -1, 0, -1, instanceCount);
  }

  EPrimitiveType GetPrimitiveType() const {
    return m_geometryDataPtr_->GetPrimitiveType();
  }

  virtual void BindBuffers(
      const std::shared_ptr<RenderFrameContext>& renderFrameContext,
      bool                                       InPositionOnly,
      const VertexBuffer* InOverrideInstanceData = nullptr) const;

  const std::vector<float>& GetVertices() const;

  bool HasInstancing() const { return m_geometryDataPtr_->HasInstancing(); }

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

  void UpdateWorldMatrix();

  math::Matrix4f m_world_;

  std::shared_ptr<RenderObjectGeometryData> m_geometryDataPtr_;

  // TODO: seems it not used, consider removing
  std::shared_ptr<Buffer> m_bottomLevelASBuffer_;
  std::shared_ptr<Buffer> m_scratchASBuffer_;
  std::shared_ptr<Buffer> m_vertexAndIndexOffsetBuffer_;

  template <typename T>
  T* GetBottomLevelASBuffer() const {
    return (T*)m_bottomLevelASBuffer_.get();
  }

  template <typename T>
  T* GetScratchASBuffer() const {
    return (T*)m_scratchASBuffer_.get();
  }

  template <typename T>
  T* GetVertexAndIndexOffsetBuffer() const {
    return (T*)m_vertexAndIndexOffsetBuffer_.get();
  }

  void SetPos(const math::Vector3Df& InPos) {
    m_position_ = InPos;
    SetDirtyFlags(EDirty::POS);
  }

  void SetRot(const math::Vector3Df& InRot) {
    m_rotation_ = InRot;
    SetDirtyFlags(EDirty::ROT);
  }

  void SetScale(const math::Vector3Df& InScale) {
    m_scale_ = InScale;
    SetDirtyFlags(EDirty::SCALE);
  }

  const math::Vector3Df& GetPos() const { return m_position_; }

  const math::Vector3Df& GetRot() const { return m_rotation_; }

  const math::Vector3Df& GetScale() const { return m_scale_; }

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
      CreateShaderBindingInstance();

  //////////////////////////////////////////////////////////////////////////

  std::shared_ptr<Material> m_materialPtr_;
  // TODO: consider removing
  std::shared_ptr<Buffer>   m_testUniformBuffer_;

  private:
  enum EDirty : int8_t {
    NONE          = 0,
    POS           = 1,
    ROT           = 1 << 1,
    SCALE         = 1 << 2,
    POS_ROT_SCALE = POS | ROT | SCALE,
  };

  EDirty m_dirtyFlags_ = EDirty::POS_ROT_SCALE;

  void SetDirtyFlags(EDirty InEnum) {
    using T       = std::underlying_type<EDirty>::type;
    m_dirtyFlags_ = static_cast<EDirty>(static_cast<T>(InEnum)
                                        | static_cast<T>(m_dirtyFlags_));
  }

  void ClearDirtyFlags(EDirty InEnum) {
    using T       = std::underlying_type<EDirty>::type;
    m_dirtyFlags_ = static_cast<EDirty>(static_cast<T>(InEnum)
                                        & (!static_cast<T>(m_dirtyFlags_)));
  }

  void ClearDirtyFlags() { m_dirtyFlags_ = EDirty::NONE; }

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
