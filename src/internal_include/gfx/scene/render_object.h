
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
    VertexStreamPtr.reset();
    VertexStream_PositionOnlyPtr.reset();
  }

  void Create(const std::shared_ptr<VertexStreamData>& InVertexStream,
              const std::shared_ptr<IndexStreamData>&  InIndexStream,
              bool                                     InHasVertexColor = true,
              bool InHasVertexBiTangent = false);

  void CreateNew_ForRaytracing(
      const std::shared_ptr<VertexStreamData>& InVertexStream,
      const std::shared_ptr<VertexStreamData>& InVertexStream_PositionOnly,
      const std::shared_ptr<IndexStreamData>&  InIndexStream,
      bool                                     InHasVertexColor     = false,
      bool                                     InHasVertexBiTangent = true);

  // Vertex buffers
  void UpdateVertexStream(
      const std::shared_ptr<VertexStreamData>& vertexStream);

  EPrimitiveType GetPrimitiveType() const {
    return VertexStreamPtr ? VertexStreamPtr->PrimitiveType
                           : EPrimitiveType::MAX;
  }

  bool HasInstancing() const { return !!VertexBuffer_InstanceDataPtr; }

  bool HasVertexColor() const { return bHasVertexColor; }

  bool HasVertexBiTangent() const { return bHasVertexBiTangent; }

  std::shared_ptr<VertexStreamData> VertexStreamPtr;
  std::shared_ptr<VertexStreamData> VertexStream_InstanceDataPtr;
  std::shared_ptr<VertexStreamData> VertexStream_PositionOnlyPtr;

  // Index buffer
  std::shared_ptr<IndexStreamData> IndexStreamPtr;

  std::shared_ptr<VertexBuffer> VertexBufferPtr;
  std::shared_ptr<VertexBuffer> VertexBuffer_PositionOnlyPtr;
  std::shared_ptr<VertexBuffer> VertexBuffer_InstanceDataPtr;
  std::shared_ptr<IndexBuffer>  IndexBufferPtr;

  // IndirectCommand buffer
  std::shared_ptr<Buffer> IndirectCommandBufferPtr;

  bool bHasVertexColor     = true;
  bool bHasVertexBiTangent = false;
};

class RenderObject {
  public:
  RenderObject() {}

  virtual ~RenderObject() {}

  virtual void CreateRenderObject(
      const std::shared_ptr<RenderObjectGeometryData>&
          InRenderObjectGeometryData) {
    GeometryDataPtr = InRenderObjectGeometryData;
    UpdateWorldMatrix();
  }

  virtual void Draw(
      const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
      int32_t                                      startIndex,
      int32_t                                      indexCount,
      int32_t                                      startVertex,
      int32_t                                      vertexCount,
      int32_t                                      instanceCount);

  virtual void Draw(
      const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
      int32_t                                      instanceCount = 1) {
    Draw(InRenderFrameContext, 0, -1, 0, -1, instanceCount);
  }

  EPrimitiveType GetPrimitiveType() const {
    return GeometryDataPtr->GetPrimitiveType();
  }

  virtual void BindBuffers(
      const std::shared_ptr<RenderFrameContext>& InRenderFrameContext,
      bool                                         InPositionOnly,
      const VertexBuffer* InOverrideInstanceData = nullptr) const;

  const std::vector<float>& GetVertices() const;

  bool HasInstancing() const { return GeometryDataPtr->HasInstancing(); }

  // TODO: currently not needed, uncomment or delete
  /*virtual bool IsSupportRaytracing() const {
    if (!GeometryDataPtr) {
      return false;
    }

    if (!GeometryDataPtr->VertexBuffer_PositionOnlyPtr) {
      return false;
    }

    return GeometryDataPtr->VertexBuffer_PositionOnlyPtr->IsSupportRaytracing();
  }*/

  void UpdateWorldMatrix();

  math::Matrix4f World;

  std::shared_ptr<RenderObjectGeometryData> GeometryDataPtr;

  std::shared_ptr<Buffer> BottomLevelASBuffer;
  std::shared_ptr<Buffer> ScratchASBuffer;
  std::shared_ptr<Buffer> VertexAndIndexOffsetBuffer;

  template <typename T>
  T* GetBottomLevelASBuffer() const {
    return (T*)BottomLevelASBuffer.get();
  }

  template <typename T>
  T* GetScratchASBuffer() const {
    return (T*)ScratchASBuffer.get();
  }

  template <typename T>
  T* GetVertexAndIndexOffsetBuffer() const {
    return (T*)VertexAndIndexOffsetBuffer.get();
  }

  void SetPos(const math::Vector3Df& InPos) {
    Pos = InPos;
    SetDirtyFlags(EDirty::POS);
  }

  void SetRot(const math::Vector3Df& InRot) {
    Rot = InRot;
    SetDirtyFlags(EDirty::ROT);
  }

  void SetScale(const math::Vector3Df& InScale) {
    Scale = InScale;
    SetDirtyFlags(EDirty::SCALE);
  }

  const math::Vector3Df& GetPos() const { return Pos; }

  const math::Vector3Df& GetRot() const { return Rot; }

  const math::Vector3Df& GetScale() const { return Scale; }

  bool IsTwoSided       = false;
  bool IsHiddenBoundBox = false;

  //////////////////////////////////////////////////////////////////////////

  struct RenderObjectUniformBuffer {
    math::Matrix4f M;
    math::Matrix4f InvM;
  };

  //////////////////////////////////////////////////////////////////////////
  // RenderObjectUniformBuffer
  virtual const std::shared_ptr<ShaderBindingInstance>&
      CreateShaderBindingInstance();

  //////////////////////////////////////////////////////////////////////////

  std::shared_ptr<Material> MaterialPtr;
  std::shared_ptr<Buffer>  TestUniformBuffer;

  private:
  enum EDirty : int8_t {
    NONE          = 0,
    POS           = 1,
    ROT           = 1 << 1,
    SCALE         = 1 << 2,
    POS_ROT_SCALE = POS | ROT | SCALE,
  };

  EDirty DirtyFlags = EDirty::POS_ROT_SCALE;

  void SetDirtyFlags(EDirty InEnum) {
    using T    = std::underlying_type<EDirty>::type;
    DirtyFlags = static_cast<EDirty>(static_cast<T>(InEnum)
                                     | static_cast<T>(DirtyFlags));
  }

  void ClearDirtyFlags(EDirty InEnum) {
    using T    = std::underlying_type<EDirty>::type;
    DirtyFlags = static_cast<EDirty>(static_cast<T>(InEnum)
                                     & (!static_cast<T>(DirtyFlags)));
  }

  void ClearDirtyFlags() { DirtyFlags = EDirty::NONE; }

  math::Vector3Df Pos   = math::Vector3Df(0);
  math::Vector3Df Rot   = math::Vector3Df(0);
  math::Vector3Df Scale = math::Vector3Df(1);

  bool NeedToUpdateRenderObjectUniformParameters = false;
  std::shared_ptr<IUniformBufferBlock>     RenderObjectUniformParametersPtr;
  std::shared_ptr<ShaderBindingInstance> RenderObjectShaderBindingInstance;

  // Special code for PBR test
  float LastMetallic  = 0.0f;
  float LastRoughness = 0.0f;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_RENDER_OBJECT_H
