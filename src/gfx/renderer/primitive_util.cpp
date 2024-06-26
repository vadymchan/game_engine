﻿
#include "gfx/renderer/primitive_util.h"

#include "gfx/rhi/rhi_type.h"
#include "gfx/rhi/vulkan/rhi_vk.h"
#include "utils/logger/global_logger.h"
#include "utils/math/math_util.h"

#include <memory>

using math::g_degreeToRadian;
using math::g_kPi;

namespace game_engine {

struct Triangle {
  int32_t Index[3];
};

void CalculateTangents(math::Vector4Df*       OutTangentArray,
                       int32_t                InTriangleCount,
                       const Triangle*        InTriangleArray,
                       int32_t                InVertexCount,
                       const math::Vector3Df* InVertexArray,
                       const math::Vector3Df* InNormalArray,
                       const math::Vector2Df* InTexCoordArray) {
  math::Vector3Df* Tangent   = new math::Vector3Df[InVertexCount * 2];
  math::Vector3Df* Bitangent = Tangent + InVertexCount;
  for (int32_t i = 0; i < InVertexCount; ++i) {
    Tangent[i]   = math::g_zeroVector<float, 3>();
    Bitangent[i] = math::g_zeroVector<float, 3>();
  }

  for (int32_t k = 0; k < InTriangleCount; ++k) {
    int32_t i0 = InTriangleArray[k].Index[0];
    int32_t i1 = InTriangleArray[k].Index[1];
    int32_t i2 = InTriangleArray[k].Index[2];

    const math::Vector3Df& p0 = InVertexArray[i0];
    const math::Vector3Df& p1 = InVertexArray[i1];
    const math::Vector3Df& p2 = InVertexArray[i2];
    const math::Vector2Df& w0 = InTexCoordArray[i0];
    const math::Vector2Df& w1 = InTexCoordArray[i1];
    const math::Vector2Df& w2 = InTexCoordArray[i2];

    math::Vector3Df e1 = p1 - p0;
    math::Vector3Df e2 = p2 - p0;
    float           x1 = w1.x() - w0.x();
    float           x2 = w2.x() - w0.x();
    float           y1 = w1.y() - w0.y();
    float           y2 = w2.y() - w0.y();

    float           r = 1.0f / (x1 * y2 - x2 * y1);
    math::Vector3Df t = (e1 * y2 - e2 * y1) * r;
    math::Vector3Df b = (e2 * x1 - e1 * x2) * r;

    Tangent[i0]   += t;
    Tangent[i1]   += t;
    Tangent[i2]   += t;
    Bitangent[i0] += b;
    Bitangent[i1] += b;
    Bitangent[i2] += b;
  }

  for (int32_t i = 0; i < InVertexCount; ++i) {
    const math::Vector3Df& t = Tangent[i];
    const math::Vector3Df& b = Bitangent[i];
    const math::Vector3Df& n = InNormalArray[i];

    // perform the rejection of the normal component from the tangent
    math::Vector3Df t_rejected = (t - n * t.dot(n)).normalize();

    OutTangentArray[i]     = math::Vector4Df(t_rejected.x(),
                                         t_rejected.y(),
                                         t_rejected.z(),
                                         0.0f);  // Normalize(t - Dot(t, n)*n)
    OutTangentArray[i].w() = (t.cross(b).dot(n) > 0.0f) ? 1.0f : -1.0f;
  }

  delete[] Tangent;
}

void QuadPrimitive::SetPlane(const math::Plane& plane) {
  this->plane = plane;
  RenderObjects[0]->SetRot(math::GetEulerAngleFrom(plane.n));
  RenderObjects[0]->SetPos(plane.n * plane.d);
}

void BillboardQuadPrimitive::Update(float deltaTime) {
  if (camera) {
    const math::Vector3Df normalizedCameraDir
        = (camera->Pos - RenderObjects[0]->GetPos()).normalize();
    const math::Vector3Df eularAngleOfCameraDir
        = math::GetEulerAngleFrom(normalizedCameraDir);

    RenderObjects[0]->SetRot(eularAngleOfCameraDir);
  } else {
    GlobalLogger::Log(LogLevel::Warning,
                      "BillboardQuad is updated without camera");
  }
}

void UIQuadPrimitive::SetTexture(const TextureVk* texture) {
  // Todo
  assert(0);
}

void UIQuadPrimitive::SetUniformParams(const Shader* shader) const {
  // Todo
  assert(0);
}

const TextureVk* UIQuadPrimitive::GetTexture() const {
  // Todo
  assert(0);
  return nullptr;
}

void FullscreenQuadPrimitive::SetUniformBuffer(const Shader* shader) const {
  // Todo
  assert(0);
}

void FullscreenQuadPrimitive::SetTexture(
    int                       index,
    const TextureVk*          texture,
    const SamplerStateInfoVk* samplerState) {
  // Todo
  assert(0);
}

void BoundBoxObject::SetUniformBuffer(const Shader* shader) {
  // Todo
  assert(0);
}

void BoundBoxObject::UpdateBoundBox(const BoundBox& boundBox) {
  this->boundBox = boundBox;
  UpdateBoundBox();
}

void BoundBoxObject::UpdateBoundBox() {
  float vertices[] = {
    boundBox.Min.x(), boundBox.Min.y(), boundBox.Min.z(), boundBox.Max.x(),
    boundBox.Min.y(), boundBox.Min.z(), boundBox.Max.x(), boundBox.Min.y(),
    boundBox.Min.z(), boundBox.Max.x(), boundBox.Min.y(), boundBox.Max.z(),
    boundBox.Max.x(), boundBox.Min.y(), boundBox.Max.z(), boundBox.Min.x(),
    boundBox.Min.y(), boundBox.Max.z(), boundBox.Min.x(), boundBox.Min.y(),
    boundBox.Max.z(), boundBox.Min.x(), boundBox.Min.y(), boundBox.Min.z(),

    boundBox.Min.x(), boundBox.Max.y(), boundBox.Min.z(), boundBox.Max.x(),
    boundBox.Max.y(), boundBox.Min.z(), boundBox.Max.x(), boundBox.Max.y(),
    boundBox.Min.z(), boundBox.Max.x(), boundBox.Max.y(), boundBox.Max.z(),
    boundBox.Max.x(), boundBox.Max.y(), boundBox.Max.z(), boundBox.Min.x(),
    boundBox.Max.y(), boundBox.Max.z(), boundBox.Min.x(), boundBox.Max.y(),
    boundBox.Max.z(), boundBox.Min.x(), boundBox.Max.y(), boundBox.Min.z(),

    boundBox.Min.x(), boundBox.Min.y(), boundBox.Min.z(), boundBox.Min.x(),
    boundBox.Max.y(), boundBox.Min.z(), boundBox.Max.x(), boundBox.Min.y(),
    boundBox.Min.z(), boundBox.Max.x(), boundBox.Max.y(), boundBox.Min.z(),
    boundBox.Max.x(), boundBox.Min.y(), boundBox.Max.z(), boundBox.Max.x(),
    boundBox.Max.y(), boundBox.Max.z(), boundBox.Min.x(), boundBox.Max.y(),
    boundBox.Max.z(), boundBox.Min.x(), boundBox.Min.y(), boundBox.Max.z(),
  };

  const int32_t elementCount = static_cast<int32_t>(std::size(vertices) / 3);
  assert(RenderObjectGeometryDataPtr->VertexStreamPtr->elementCount
         == elementCount);

  BufferAttributeStream<float>* PositionParam
      = static_cast<BufferAttributeStream<float>*>(
          RenderObjectGeometryDataPtr->VertexStreamPtr->streams[0].get());
  memcpy(&PositionParam->Data[0], vertices, sizeof(vertices));
}

void BoundSphereObject::SetUniformBuffer(const Shader* shader) {
  // Todo
  assert(0);
}

void ArrowSegmentPrimitive::Update(float deltaTime) {
  Object::Update(deltaTime);

  if (SegmentObject) {
    SegmentObject->Update(deltaTime);
  }
  if (ConeObject) {
    ConeObject->Update(deltaTime);
  }
}

void ArrowSegmentPrimitive::SetPos(const math::Vector3Df& pos) {
  SegmentObject->RenderObjects[0]->SetPos(pos);
}

void ArrowSegmentPrimitive::SetStart(const math::Vector3Df& start) {
  SegmentObject->Start = start;
}

void ArrowSegmentPrimitive::SetEnd(const math::Vector3Df& end) {
  SegmentObject->End = end;
}

void ArrowSegmentPrimitive::SetTime(float time) {
  SegmentObject->Time = time;
}

//////////////////////////////////////////////////////////////////////////
// Utilities

std::vector<float> GenerateColor(const math::Vector4Df& color,
                                 int32_t                elementCount) {
  std::vector<float> temp;
  temp.resize(elementCount * 4);
  for (int i = 0; i < elementCount; ++i) {
    temp[i * 4 + 0] = color.x();
    temp[i * 4 + 1] = color.y();
    temp[i * 4 + 2] = color.z();
    temp[i * 4 + 3] = color.w();
  }

  return std::move(temp);
}

BoundBox GenerateBoundBox(const std::vector<float>& vertices) {
  auto min = math::Vector3Df(FLT_MAX);
  auto max = math::Vector3Df(FLT_MIN);
  for (size_t i = 0; i < vertices.size() / 3; ++i) {
    auto curIndex = i * 3;
    auto x        = vertices[curIndex];
    auto y        = vertices[curIndex + 1];
    auto z        = vertices[curIndex + 2];
    if (max.x() < x) {
      max.x() = x;
    }
    if (max.y() < y) {
      max.y() = y;
    }
    if (max.z() < z) {
      max.z() = z;
    }

    if (min.x() > x) {
      min.x() = x;
    }
    if (min.y() > y) {
      min.y() = y;
    }
    if (min.z() > z) {
      min.z() = z;
    }
  }

  return {min, max};
}

BoundSphere GenerateBoundSphere(const std::vector<float>& vertices) {
  auto maxDist = FLT_MIN;
  for (size_t i = 0; i < vertices.size() / 3; ++i) {
    auto curIndex = i * 3;
    auto x        = vertices[curIndex];
    auto y        = vertices[curIndex + 1];
    auto z        = vertices[curIndex + 2];

    auto       currentPos = math::Vector3Df(x, y, z);
    const auto dist       = currentPos.magnitude();
    if (maxDist < dist) {
      maxDist = dist;
    }
  }
  return {maxDist};
}

void CreateBoundObjects(const std::vector<float>& vertices,
                        Object*                   ownerObject) {
  ownerObject->boundBox    = GenerateBoundBox(vertices);
  ownerObject->boundSphere = GenerateBoundSphere(vertices);

  ownerObject->BoundBoxObject
      = CreateBoundBox(ownerObject->boundBox, ownerObject);
  ownerObject->BoundSphereObject
      = CreateBoundSphere(ownerObject->boundSphere, ownerObject);
  Object::AddBoundBoxObject(ownerObject->BoundBoxObject);
  Object::AddBoundSphereObject(ownerObject->BoundSphereObject);
}

BoundBoxObject* CreateBoundBox(BoundBox               boundBox,
                               Object*                ownerObject,
                               const math::Vector4Df& color) {
  float vertices[] = {
    boundBox.Min.x(), boundBox.Min.y(), boundBox.Min.z(), boundBox.Max.x(),
    boundBox.Min.y(), boundBox.Min.z(), boundBox.Max.x(), boundBox.Min.y(),
    boundBox.Min.z(), boundBox.Max.x(), boundBox.Min.y(), boundBox.Max.z(),
    boundBox.Max.x(), boundBox.Min.y(), boundBox.Max.z(), boundBox.Min.x(),
    boundBox.Min.y(), boundBox.Max.z(), boundBox.Min.x(), boundBox.Min.y(),
    boundBox.Max.z(), boundBox.Min.x(), boundBox.Min.y(), boundBox.Min.z(),

    boundBox.Min.x(), boundBox.Max.y(), boundBox.Min.z(), boundBox.Max.x(),
    boundBox.Max.y(), boundBox.Min.z(), boundBox.Max.x(), boundBox.Max.y(),
    boundBox.Min.z(), boundBox.Max.x(), boundBox.Max.y(), boundBox.Max.z(),
    boundBox.Max.x(), boundBox.Max.y(), boundBox.Max.z(), boundBox.Min.x(),
    boundBox.Max.y(), boundBox.Max.z(), boundBox.Min.x(), boundBox.Max.y(),
    boundBox.Max.z(), boundBox.Min.x(), boundBox.Max.y(), boundBox.Min.z(),

    boundBox.Min.x(), boundBox.Min.y(), boundBox.Min.z(), boundBox.Min.x(),
    boundBox.Max.y(), boundBox.Min.z(), boundBox.Max.x(), boundBox.Min.y(),
    boundBox.Min.z(), boundBox.Max.x(), boundBox.Max.y(), boundBox.Min.z(),
    boundBox.Max.x(), boundBox.Min.y(), boundBox.Max.z(), boundBox.Max.x(),
    boundBox.Max.y(), boundBox.Max.z(), boundBox.Min.x(), boundBox.Max.y(),
    boundBox.Max.z(), boundBox.Min.x(), boundBox.Min.y(), boundBox.Max.z(),
  };

  const int32_t elementCount = static_cast<int32_t>(std::size(vertices) / 3);

  auto vertexStreamData = std::make_shared<VertexStreamData>();

  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("POSITION"),
        EBufferType::Static,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        std::vector<float>(vertices, vertices + elementCount * 3));

    vertexStreamData->streams.push_back(streamParam);
  }

  vertexStreamData->PrimitiveType = EPrimitiveType::LINES;
  vertexStreamData->elementCount  = elementCount;

  auto object       = new BoundBoxObject();
  auto renderObject = new RenderObject();
  object->RenderObjectGeometryDataPtr
      = std::make_shared<RenderObjectGeometryData>(vertexStreamData, nullptr);
  renderObject->CreateRenderObject(object->RenderObjectGeometryDataPtr);
  object->RenderObjects.push_back(renderObject);
  object->SkipShadowMapGen       = true;
  object->SkipUpdateShadowVolume = true;
  object->OwnerObject            = ownerObject;
  object->Color                  = color;

  if (object->OwnerObject) {
    object->PostUpdateFunc = [](Object* thisObject, float deltaTime) {
      assert(thisObject);
      auto boundBoxObject = static_cast<BoundBoxObject*>(thisObject);
      if (!boundBoxObject) {
        return;
      }

      assert(boundBoxObject->OwnerObject);
      auto ownerObject = boundBoxObject->OwnerObject;

      boundBoxObject->RenderObjects[0]->SetPos(
          ownerObject->RenderObjects[0]->GetPos());
      boundBoxObject->RenderObjects[0]->SetRot(
          ownerObject->RenderObjects[0]->GetRot());
      boundBoxObject->RenderObjects[0]->SetScale(
          ownerObject->RenderObjects[0]->GetScale());
      boundBoxObject->Visible = ownerObject->Visible;
    };
  }

  return object;
}

BoundSphereObject* CreateBoundSphere(BoundSphere            boundSphere,
                                     Object*                ownerObject,
                                     const math::Vector4Df& color) {
  int32_t slice = 15;
  if (slice % 2) {
    ++slice;
  }

  std::vector<float> vertices;
  const float        stepRadian = math::g_degreeToRadian(360.0f / slice);
  const float        radius     = boundSphere.Radius;

  for (int32_t j = 0; j < slice / 2; ++j) {
    for (int32_t i = 0; i <= slice; ++i) {
      const math::Vector3Df temp(
          cosf(stepRadian * i) * radius * sinf(stepRadian * (j + 1)),
          cosf(stepRadian * (j + 1)) * radius,
          sinf(stepRadian * i) * radius * sinf(stepRadian * (j + 1)));
      vertices.push_back(temp.x());
      vertices.push_back(temp.y());
      vertices.push_back(temp.z());
    }
  }

  // top
  vertices.push_back(0.0f);
  vertices.push_back(radius);
  vertices.push_back(0.0f);

  // down
  vertices.push_back(0.0f);
  vertices.push_back(-radius);
  vertices.push_back(0.0f);

  int32_t elementCount = static_cast<int32_t>(vertices.size() / 3);

  // attribute
  auto vertexStreamData = std::make_shared<VertexStreamData>();
  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("POSITION"),
        EBufferType::Static,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        vertices);

    vertexStreamData->streams.push_back(streamParam);
  }

  vertexStreamData->PrimitiveType = EPrimitiveType::LINES;
  vertexStreamData->elementCount  = elementCount;

  // IndexStream
  std::vector<uint32_t> faces;
  int32_t               iCount      = 0;
  int32_t               toNextSlice = slice + 1;
  int32_t               temp        = 6;
  for (int32_t i = 0; i < (slice) / 2 - 2; ++i, iCount += 1) {
    for (int32_t i = 0; i < slice; ++i, iCount += 1) {
      faces.push_back(iCount);
      faces.push_back(iCount + 1);
      faces.push_back(iCount + toNextSlice);
      faces.push_back(iCount + toNextSlice);
      faces.push_back(iCount + 1);
      faces.push_back(iCount + toNextSlice + 1);
    }
  }

  for (int32_t i = 0; i < slice; ++i, iCount += 1) {
    faces.push_back(iCount);
    faces.push_back(iCount + 1);
    faces.push_back(elementCount - 1);
  }

  iCount = 0;
  for (int32_t i = 0; i < slice; ++i, iCount += 1) {
    faces.push_back(iCount);
    faces.push_back(elementCount - 2);
    faces.push_back(iCount + 1);
  }

  auto indexStreamData          = std::make_shared<IndexStreamData>();
  indexStreamData->elementCount = static_cast<int32_t>(faces.size());
  {
    auto streamParam = new BufferAttributeStream<uint32_t>(
        Name("Index"),
        EBufferType::Static,
        sizeof(uint32_t) * 3,
        {IBufferAttribute::Attribute(
            EBufferElementType::UINT32, 0, sizeof(int32_t) * 3)},
        faces

    );
    indexStreamData->stream = streamParam;
  }

  auto object       = new BoundSphereObject();
  auto renderObject = new RenderObject();
  object->RenderObjectGeometryDataPtr
      = std::make_shared<RenderObjectGeometryData>(vertexStreamData,
                                                   indexStreamData);
  renderObject->CreateRenderObject(object->RenderObjectGeometryDataPtr);
  object->RenderObjects.push_back(renderObject);
  object->SkipShadowMapGen       = true;
  object->SkipUpdateShadowVolume = true;
  object->OwnerObject            = ownerObject;
  object->boundSphere            = boundSphere;

  object->PostUpdateFunc = [](Object* thisObject, float deltaTime) {
    assert(thisObject);
    auto boundSphereObject = static_cast<BoundSphereObject*>(thisObject);
    if (!boundSphereObject) {
      return;
    }

    assert(boundSphereObject->OwnerObject);
    auto ownerObject = boundSphereObject->OwnerObject;

    boundSphereObject->RenderObjects[0]->SetPos(
        ownerObject->RenderObjects[0]->GetPos());
    boundSphereObject->RenderObjects[0]->SetRot(
        ownerObject->RenderObjects[0]->GetRot());
    boundSphereObject->RenderObjects[0]->SetScale(
        ownerObject->RenderObjects[0]->GetScale());
    boundSphereObject->Visible = ownerObject->Visible;
  };

  return object;
}

//////////////////////////////////////////////////////////////////////////
// Primitive Generation

RenderObject* CreateQuad_Internal(const math::Vector3Df& pos,
                                  const math::Vector3Df& size,
                                  const math::Vector3Df& scale,
                                  const math::Vector4Df& color) {
  auto halfSize = size / 2.0;
  auto offset   = math::g_zeroVector<float, 3>();

  std::vector<BaseVertex> vertices(6);
  {
    vertices[0].Pos = math::Vector3Df(
        offset.x() + (-halfSize.x()), 0.0f, offset.z() + (-halfSize.z()));
    vertices[0].Normal    = math::Vector3Df(0.0f, 1.0f, 0.0f);
    vertices[0].Tangent   = math::Vector3Df(1.0f, 0.0f, 0.0f);
    vertices[0].Bitangent = math::Vector3Df(0.0f, 0.0f, 1.0f);
    vertices[0].TexCoord  = math::Vector2Df(0.0f, 0.0f);

    vertices[1].Pos = math::Vector3Df(
        offset.x() + (halfSize.x()), 0.0f, offset.z() + (-halfSize.z()));
    vertices[1].Normal    = math::Vector3Df(0.0f, 1.0f, 0.0f);
    vertices[1].Tangent   = math::Vector3Df(1.0f, 0.0f, 0.0f);
    vertices[1].Bitangent = math::Vector3Df(0.0f, 0.0f, 1.0f);
    vertices[1].TexCoord  = math::Vector2Df(1.0f, 0.0f);

    vertices[2].Pos = math::Vector3Df(
        offset.x() + (halfSize.x()), 0.0f, offset.z() + (halfSize.z()));
    vertices[2].Normal    = math::Vector3Df(0.0f, 1.0f, 0.0f);
    vertices[2].Tangent   = math::Vector3Df(1.0f, 0.0f, 0.0f);
    vertices[2].Bitangent = math::Vector3Df(0.0f, 0.0f, 1.0f);
    vertices[2].TexCoord  = math::Vector2Df(1.0f, 1.0f);

    vertices[3].Pos = math::Vector3Df(
        offset.x() + (halfSize.x()), 0.0f, offset.z() + (halfSize.z()));
    vertices[3].Normal    = math::Vector3Df(0.0f, 1.0f, 0.0f);
    vertices[3].Tangent   = math::Vector3Df(1.0f, 0.0f, 0.0f);
    vertices[3].Bitangent = math::Vector3Df(0.0f, 0.0f, 1.0f);
    vertices[3].TexCoord  = math::Vector2Df(1.0f, 1.0f);

    vertices[4].Pos = math::Vector3Df(
        offset.x() + (-halfSize.x()), 0.0f, offset.z() + (halfSize.z()));
    vertices[4].Normal    = math::Vector3Df(0.0f, 1.0f, 0.0f);
    vertices[4].Tangent   = math::Vector3Df(1.0f, 0.0f, 0.0f);
    vertices[4].Bitangent = math::Vector3Df(0.0f, 0.0f, 1.0f);
    vertices[4].TexCoord  = math::Vector2Df(0.0f, 1.0f);

    vertices[5].Pos = math::Vector3Df(
        offset.x() + (-halfSize.x()), 0.0f, offset.z() + (-halfSize.z()));
    vertices[5].Normal    = math::Vector3Df(0.0f, 1.0f, 0.0f);
    vertices[5].Tangent   = math::Vector3Df(1.0f, 0.0f, 0.0f);
    vertices[5].Bitangent = math::Vector3Df(0.0f, 0.0f, 1.0f);
    vertices[5].TexCoord  = math::Vector2Df(0.0f, 0.0f);
  }

  std::vector<uint32_t> indices;
  indices.reserve(6);
  for (int32_t i = 0; i < 6; ++i) {
    indices.push_back(i);
  }

  // PositionOnly VertexStream
  std::vector<PositionOnlyVertex> verticesPositionOnly(vertices.size());
  for (int32_t i = 0; i < (int32_t)vertices.size(); ++i) {
    verticesPositionOnly[i].Pos = vertices[i].Pos;
  }

  const int32_t NumOfVertices       = (int32_t)vertices.size();
  auto positionOnlyVertexStreamData = std::make_shared<VertexStreamData>();
  {
    auto streamParam
        = std::make_shared<BufferAttributeStream<PositionOnlyVertex>>(
            Name("PositionOnlyVertex"),
            EBufferType::Static,
            sizeof(PositionOnlyVertex),
            std::vector<IBufferAttribute::Attribute>{
              IBufferAttribute::Attribute(NameStatic("POSITION"),
                                          EBufferElementType::FLOAT,
                                          0,
                                          sizeof(float) * 3)},
            verticesPositionOnly);

    positionOnlyVertexStreamData->PrimitiveType = EPrimitiveType::TRIANGLES;
    positionOnlyVertexStreamData->elementCount  = NumOfVertices;
  }

  // Base VertexStream
  auto vertexStreamData = std::make_shared<VertexStreamData>();
  {
    auto streamParam = std::make_shared<BufferAttributeStream<BaseVertex>>(
        Name("BaseVertex"),
        EBufferType::Static,
        sizeof(BaseVertex),
        std::vector<IBufferAttribute::Attribute>{
          IBufferAttribute::Attribute(NameStatic("POSITION"),
                                      EBufferElementType::FLOAT,
                                      0,
                                      sizeof(float) * 3),
          IBufferAttribute::Attribute(NameStatic("NORMAL"),
                                      EBufferElementType::FLOAT,
                                      sizeof(float) * 3,
                                      sizeof(float) * 3),
          IBufferAttribute::Attribute(NameStatic("TANGENT"),
                                      EBufferElementType::FLOAT,
                                      sizeof(float) * 6,
                                      sizeof(float) * 3),
          IBufferAttribute::Attribute(NameStatic("BITANGENT"),
                                      EBufferElementType::FLOAT,
                                      sizeof(float) * 9,
                                      sizeof(float) * 3),
          IBufferAttribute::Attribute(NameStatic("TEXCOORD"),
                                      EBufferElementType::FLOAT,
                                      sizeof(float) * 12,
                                      sizeof(float) * 2)},
        vertices);

    vertexStreamData->streams.push_back(streamParam);

    vertexStreamData->PrimitiveType = EPrimitiveType::TRIANGLES;
    vertexStreamData->elementCount  = NumOfVertices;
  }

  auto indexStreamData          = std::make_shared<IndexStreamData>();
  indexStreamData->elementCount = static_cast<int32_t>(indices.size());
  {
    auto streamParam = new BufferAttributeStream<uint32_t>(
        Name("Index"),
        EBufferType::Static,
        sizeof(uint32_t) * 3,
        {IBufferAttribute::Attribute(
            EBufferElementType::UINT32, 0, sizeof(int32_t) * 3)},
        indices);
    indexStreamData->stream = streamParam;
  }

  auto renderObject                = new RenderObject();
  auto RenderObjectGeometryDataPtr = std::make_shared<RenderObjectGeometryData>(
      vertexStreamData, positionOnlyVertexStreamData, indexStreamData);
  renderObject->CreateRenderObject(RenderObjectGeometryDataPtr);
  renderObject->SetPos(pos);
  renderObject->SetScale(scale);
  renderObject->MaterialPtr = GDefaultMaterial;
  return renderObject;
}

QuadPrimitive* CreateQuad(const math::Vector3Df& pos,
                          const math::Vector3Df& size,
                          const math::Vector3Df& scale,
                          const math::Vector4Df& color) {
  auto object       = new QuadPrimitive();
  auto RenderObject = CreateQuad_Internal(pos, size, scale, color);
  object->RenderObjectGeometryDataPtr = RenderObject->GeometryDataPtr;
  object->RenderObjects.push_back(RenderObject);
  RenderObject->IsTwoSided = true;
  // object->CreateBoundBox();
  return object;
}

//////////////////////////////////////////////////////////////////////////
Object* CreateGizmo(const math::Vector3Df& pos,
                    const math::Vector3Df& rot,
                    const math::Vector3Df& scale) {
  float length     = 5.0f;
  float length2    = length * 0.6f;
  float vertices[] = {
    0.0f,    0.0f,
    0.0f,    0.0f,
    0.0f,    length,
    0.0f,    0.0f,
    length,  length2 / 2.0f,
    0.0f,    length2,
    0.0f,    0.0f,
    length,  -length2 / 2.0f,
    0.0f,    length2,

    0.0f,    0.0f,
    0.0f,    length,
    0.0f,    0.0f,
    length,  0.0f,
    0.0f,    length2,
    0.0f,    length2 / 2.0f,
    length,  0.0f,
    0.0f,    length2,
    0.0f,    -length2 / 2.0f,

    0.0f,    0.0f,
    0.0f,    0.0f,
    length,  0.0f,
    0.0f,    length,
    0.0f,    length2 / 2.0f,
    length2, 0.0f,
    0.0f,    length,
    0.0f,    -length2 / 2.0f,
    length2, 0.0f,
  };

  float colors[] = {
    0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
  };

  const int32_t elementCount = std::size(vertices) / 3;

  // attribute
  auto vertexStreamData = std::make_shared<VertexStreamData>();

  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("POSITION"),
        EBufferType::Static,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        std::vector<float>(vertices, vertices + elementCount * 3));
    vertexStreamData->streams.push_back(streamParam);
  }

  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("COLOR"),
        EBufferType::Static,
        sizeof(float) * 4,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 4)},
        std::vector<float>(colors, colors + elementCount * 4));
    vertexStreamData->streams.push_back(streamParam);
  }

  {
    std::vector<float> normals(elementCount * 3);
    for (int32_t i = 0; i < elementCount; ++i) {
      normals[i * 3 + 0] = 0.0f;
      normals[i * 3 + 1] = 1.0f;
      normals[i * 3 + 2] = 0.0f;
    }
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("NORMAL"),
        EBufferType::Static,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        normals);
    vertexStreamData->streams.push_back(streamParam);
  }

  {
    std::vector<float> tangents(elementCount * 3);
    for (int32_t i = 0; i < elementCount; ++i) {
      tangents[i * 3 + 0] = 1.0f;
      tangents[i * 3 + 1] = 0.0f;
      tangents[i * 3 + 2] = 0.0f;
    }
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("TANGENT"),
        EBufferType::Static,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        tangents);
    vertexStreamData->streams.push_back(streamParam);
  }

  {
    std::vector<float> texcoords(elementCount * 2, 0.0f);
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("TEXCOORD"),
        EBufferType::Static,
        sizeof(float) * 2,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 2)},
        texcoords);
    vertexStreamData->streams.push_back(streamParam);
  }

  vertexStreamData->PrimitiveType = EPrimitiveType::LINES;
  vertexStreamData->elementCount  = elementCount;

  auto object       = new QuadPrimitive();
  auto renderObject = new RenderObject();
  object->RenderObjectGeometryDataPtr
      = std::make_shared<RenderObjectGeometryData>(vertexStreamData, nullptr);
  renderObject->CreateRenderObject(object->RenderObjectGeometryDataPtr);
  object->RenderObjects.push_back(renderObject);
  renderObject->SetPos(pos);
  renderObject->SetScale(scale);
  // object->CreateBoundBox();
  return object;
}

Object* CreateTriangle(const math::Vector3Df& pos,
                       const math::Vector3Df& size,
                       const math::Vector3Df& scale,
                       const math::Vector4Df& color) {
  const auto halfSize = size / 2.0f;
  const auto offset   = math::g_zeroVector<float, 3>();
  // TODO: consider another base vertex coordinates (equilateral triangle or
  // right-angled isosceles triangle with one vertex at origin; clockwise or
  // counterclockwise order; parralel around other plane)
  // vertices parralel to XZ plane
  // float      vertices[] = {
  //  offset.x() + (halfSize.x()),
  //  0.0,
  //  offset.z() + (-halfSize.z()),
  //  offset.x() + (halfSize.x()),
  //  0.0,
  //  offset.z() + (halfSize.z()),
  //  offset.x() + (-halfSize.x()),
  //  0.0,
  //  offset.z() + (-halfSize.z()),
  //};

  // vertices parralel to XZ plane
  float vertices[] = {
    offset.x() + (halfSize.x()),
    offset.y() + (halfSize.y()),
    0.0f,

    offset.x() + (-halfSize.x()),
    offset.y() + (-halfSize.y()),
    0.0f,

    offset.x() + (halfSize.x()),
    offset.y() + (-halfSize.y()),
    0.0f,
  };

  math::Vector2Df texcoords[] = {math::Vector2Df(1.0f, 0.0f),
                                 math::Vector2Df(0.0f, 1.0f),
                                 math::Vector2Df(1.0f, 1.0f)};

  int32_t elementCount = std::size(vertices) / 3;

  // attribute
  auto vertexStreamData = std::make_shared<VertexStreamData>();

  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("POSITION"),
        EBufferType::Static,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        std::vector<float>(vertices, vertices + elementCount * 3));
    vertexStreamData->streams.push_back(streamParam);
  }

  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("COLOR"),
        EBufferType::Static,
        sizeof(float) * 4,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 4)},
        GenerateColor(color, elementCount));
    vertexStreamData->streams.push_back(streamParam);
  }

  {
    std::vector<float> normals(elementCount * 3, 0.0f);
    for (int32_t i = 0; i < elementCount; ++i) {
      normals[i * 3 + 0] = 0.0f;
      normals[i * 3 + 1] = 1.0f;
      normals[i * 3 + 2] = 0.0f;
    }
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("NORMAL"),
        EBufferType::Static,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        normals);
    vertexStreamData->streams.push_back(streamParam);
  }

  {
    std::vector<float> tangents(elementCount * 3, 0.0f);
    for (int32_t i = 0; i < elementCount; ++i) {
      tangents[i * 3 + 0] = 1.0f;
      tangents[i * 3 + 1] = 0.0f;
      tangents[i * 3 + 2] = 0.0f;
    }
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("TANGENT"),
        EBufferType::Static,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        tangents);
    vertexStreamData->streams.push_back(streamParam);
  }

  {
    // TODO: hot-fix solution (need to rewrite)
    std::vector<float> texcoordsData;
    texcoordsData.reserve(std::size(texcoords) * 2);

    for (const auto& texcoord : texcoords) {
      texcoordsData.insert(
          texcoordsData.end(), texcoord.data(), texcoord.data() + 2);
    }

    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("TEXCOORD"),
        EBufferType::Static,
        sizeof(float) * 2,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 2)},
        std::move(texcoordsData));
    vertexStreamData->streams.push_back(streamParam);
  }

  vertexStreamData->PrimitiveType = EPrimitiveType::TRIANGLES;
  vertexStreamData->elementCount  = elementCount;

  auto object       = new Object();
  auto renderObject = new RenderObject();
  object->RenderObjectGeometryDataPtr
      = std::make_shared<RenderObjectGeometryData>(vertexStreamData, nullptr);
  renderObject->CreateRenderObject(object->RenderObjectGeometryDataPtr);
  object->RenderObjects.push_back(renderObject);
  renderObject->SetPos(pos);
  renderObject->SetScale(scale);
  renderObject->IsTwoSided = true;
  // object->CreateBoundBox();

  return object;
}

Object* CreateCube(const math::Vector3Df& pos,
                   const math::Vector3Df& size,
                   const math::Vector3Df& scale,
                   const math::Vector4Df& color) {
  const math::Vector3Df halfSize = size / 2.0f;
  const math::Vector3Df offset   = math::g_zeroVector<float, 3>();

  float vertices[] = {
    offset.x() + (halfSize.x()),  offset.y() + (-halfSize.y()),
    offset.z() + (halfSize.z()),  offset.x() + (-halfSize.x()),
    offset.y() + (-halfSize.y()), offset.z() + (-halfSize.z()),
    offset.x() + (-halfSize.x()), offset.y() + (-halfSize.y()),
    offset.z() + (halfSize.z()),  offset.x() + (-halfSize.x()),
    offset.y() + (-halfSize.y()), offset.z() + (-halfSize.z()),
    offset.x() + (halfSize.x()),  offset.y() + (-halfSize.y()),
    offset.z() + (halfSize.z()),  offset.x() + (halfSize.x()),
    offset.y() + (-halfSize.y()), offset.z() + (-halfSize.z()),

    offset.x() + (halfSize.x()),  offset.y() + (halfSize.y()),
    offset.z() + (halfSize.z()),  offset.x() + (-halfSize.x()),
    offset.y() + (halfSize.y()),  offset.z() + (-halfSize.z()),
    offset.x() + (halfSize.x()),  offset.y() + (halfSize.y()),
    offset.z() + (-halfSize.z()), offset.x() + (-halfSize.x()),
    offset.y() + (halfSize.y()),  offset.z() + (-halfSize.z()),
    offset.x() + (halfSize.x()),  offset.y() + (halfSize.y()),
    offset.z() + (halfSize.z()),  offset.x() + (-halfSize.x()),
    offset.y() + (halfSize.y()),  offset.z() + (halfSize.z()),

    offset.x() + (halfSize.x()),  offset.y() + (halfSize.y()),
    offset.z() + (-halfSize.z()), offset.x() + (-halfSize.x()),
    offset.y() + (-halfSize.y()), offset.z() + (-halfSize.z()),
    offset.x() + (halfSize.x()),  offset.y() + (-halfSize.y()),
    offset.z() + (-halfSize.z()), offset.x() + (-halfSize.x()),
    offset.y() + (-halfSize.y()), offset.z() + (-halfSize.z()),
    offset.x() + (halfSize.x()),  offset.y() + (halfSize.y()),
    offset.z() + (-halfSize.z()), offset.x() + (-halfSize.x()),
    offset.y() + (halfSize.y()),  offset.z() + (-halfSize.z()),

    offset.x() + (halfSize.x()),  offset.y() + (halfSize.y()),
    offset.z() + (halfSize.z()),  offset.x() + (halfSize.x()),
    offset.y() + (-halfSize.y()), offset.z() + (-halfSize.z()),
    offset.x() + (halfSize.x()),  offset.y() + (-halfSize.y()),
    offset.z() + (halfSize.z()),  offset.x() + (halfSize.x()),
    offset.y() + (-halfSize.y()), offset.z() + (-halfSize.z()),
    offset.x() + (halfSize.x()),  offset.y() + (halfSize.y()),
    offset.z() + (halfSize.z()),  offset.x() + (halfSize.x()),
    offset.y() + (halfSize.y()),  offset.z() + (-halfSize.z()),

    offset.x() + (-halfSize.x()), offset.y() + (halfSize.y()),
    offset.z() + (halfSize.z()),  offset.x() + (halfSize.x()),
    offset.y() + (-halfSize.y()), offset.z() + (halfSize.z()),
    offset.x() + (-halfSize.x()), offset.y() + (-halfSize.y()),
    offset.z() + (halfSize.z()),  offset.x() + (halfSize.x()),
    offset.y() + (-halfSize.y()), offset.z() + (halfSize.z()),
    offset.x() + (-halfSize.x()), offset.y() + (halfSize.y()),
    offset.z() + (halfSize.z()),  offset.x() + (halfSize.x()),
    offset.y() + (halfSize.y()),  offset.z() + (halfSize.z()),

    offset.x() + (-halfSize.x()), offset.y() + (halfSize.y()),
    offset.z() + (-halfSize.z()), offset.x() + (-halfSize.x()),
    offset.y() + (-halfSize.y()), offset.z() + (halfSize.z()),
    offset.x() + (-halfSize.x()), offset.y() + (-halfSize.y()),
    offset.z() + (-halfSize.z()), offset.x() + (-halfSize.x()),
    offset.y() + (-halfSize.y()), offset.z() + (halfSize.z()),
    offset.x() + (-halfSize.x()), offset.y() + (halfSize.y()),
    offset.z() + (-halfSize.z()), offset.x() + (-halfSize.x()),
    offset.y() + (halfSize.y()),  offset.z() + (halfSize.z()),
  };

  float normals[] = {
    0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,
    0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,

    0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
    0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

    0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f,
    0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f,

    1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
    1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

    0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
    0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,

    -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,
    -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,
  };

  math::Vector2Df texcoords[] = {
    math::Vector2Df(0.0f, 0.0f), math::Vector2Df(1.0f, 1.0f),
    math::Vector2Df(1.0f, 0.0f), math::Vector2Df(1.0f, 1.0f),
    math::Vector2Df(0.0f, 0.0f), math::Vector2Df(0.0f, 1.0f),

    math::Vector2Df(1.0f, 0.0f), math::Vector2Df(0.0f, 1.0f),
    math::Vector2Df(1.0f, 1.0f), math::Vector2Df(0.0f, 1.0f),
    math::Vector2Df(1.0f, 0.0f), math::Vector2Df(0.0f, 0.0f),

    math::Vector2Df(1.0f, 0.0f), math::Vector2Df(0.0f, 1.0f),
    math::Vector2Df(1.0f, 1.0f), math::Vector2Df(0.0f, 1.0f),
    math::Vector2Df(1.0f, 0.0f), math::Vector2Df(0.0f, 0.0f),

    math::Vector2Df(1.0f, 0.0f), math::Vector2Df(0.0f, 1.0f),
    math::Vector2Df(1.0f, 1.0f), math::Vector2Df(0.0f, 1.0f),
    math::Vector2Df(1.0f, 0.0f), math::Vector2Df(0.0f, 0.0f),

    math::Vector2Df(1.0f, 0.0f), math::Vector2Df(0.0f, 1.0f),
    math::Vector2Df(1.0f, 1.0f), math::Vector2Df(0.0f, 1.0f),
    math::Vector2Df(1.0f, 0.0f), math::Vector2Df(0.0f, 0.0f),

    math::Vector2Df(1.0f, 0.0f), math::Vector2Df(0.0f, 1.0f),
    math::Vector2Df(1.0f, 1.0f), math::Vector2Df(0.0f, 1.0f),
    math::Vector2Df(1.0f, 0.0f), math::Vector2Df(0.0f, 0.0f),
  };

  const int32_t elementCount = std::size(vertices) / 3;

  // attribute
  auto vertexStreamData = std::make_shared<VertexStreamData>();

  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("POSITION"),
        EBufferType::Static,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        std::vector<float>(vertices, vertices + elementCount * 3));
    vertexStreamData->streams.push_back(streamParam);
  }

  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("COLOR"),
        EBufferType::Static,
        sizeof(float) * 4,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 4)},
        GenerateColor(color, elementCount));
    vertexStreamData->streams.push_back(streamParam);
  }

  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("NORMAL"),
        EBufferType::Static,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        std::vector<float>(normals, normals + elementCount * 3));
    vertexStreamData->streams.push_back(streamParam);
  }

  {
    // Create tangent
    const int32_t         TotalNumOfTriangles = elementCount / 3;
    std::vector<Triangle> TriangleArray;
    for (int32_t i = 0; i < TotalNumOfTriangles; ++i) {
      TriangleArray.push_back(Triangle{i * 3, i * 3 + 1, i * 3 + 2});
    }

    constexpr int32_t verticesElementCount = sizeof(vertices) / sizeof(float);
    std::vector<math::Vector4Df> TangentArray(elementCount);
    CalculateTangents(TangentArray.data(),
                      static_cast<int32_t>(TriangleArray.size()),
                      TriangleArray.data(),
                      elementCount,
                      // TODO: pay attention to this cast (seems odd)
                      reinterpret_cast<const math::Vector3Df*>(vertices),
                      reinterpret_cast<const math::Vector3Df*>(normals),
                      texcoords);

    std::vector<float> tangentData;
    tangentData.reserve(elementCount * 3);
    for (int32_t i = 0; i < elementCount; ++i) {
      tangentData.push_back(TangentArray[i].x());
      tangentData.push_back(TangentArray[i].y());
      tangentData.push_back(TangentArray[i].z());
    }

    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("TANGENT"),
        EBufferType::Static,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        tangentData);
    vertexStreamData->streams.push_back(streamParam);
  }

  {
    // TODO: hot-fix solution (need to rewrite)
    std::vector<float> texcoordsData;
    texcoordsData.reserve(std::size(texcoords) * 2);

    for (const auto& texcoord : texcoords) {
      texcoordsData.insert(
          texcoordsData.end(), texcoord.data(), texcoord.data() + 2);
    }

    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("TEXCOORD"),
        EBufferType::Static,
        sizeof(float) * 2,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 2)},
        std::move(texcoordsData));
    vertexStreamData->streams.push_back(streamParam);
  }

  auto                  indexStreamData = std::make_shared<IndexStreamData>();
  std::vector<uint32_t> faces;
  faces.resize(elementCount);
  for (int32_t i = 0; i < elementCount; ++i) {
    faces[i] = i;
  }
  indexStreamData->elementCount = static_cast<int32_t>(faces.size());
  {
    auto streamParam = new BufferAttributeStream<uint32_t>(
        Name("Index"),
        EBufferType::Static,
        sizeof(uint32_t) * 3,
        {IBufferAttribute::Attribute(
            EBufferElementType::UINT32, 0, sizeof(int32_t) * 3)},
        faces);
    indexStreamData->stream = streamParam;
  }

  vertexStreamData->PrimitiveType = EPrimitiveType::TRIANGLES;
  vertexStreamData->elementCount  = elementCount;

  auto object       = new Object();
  auto renderObject = new RenderObject();
  object->RenderObjectGeometryDataPtr
      = std::make_shared<RenderObjectGeometryData>(vertexStreamData,
                                                   indexStreamData);
  renderObject->CreateRenderObject(object->RenderObjectGeometryDataPtr);
  object->RenderObjects.push_back(renderObject);
  renderObject->SetPos(pos);
  renderObject->SetScale(scale);
  // object->CreateBoundBox();

  return object;
}

Object* CreateCapsule(const math::Vector3Df& pos,
                      float                  height,
                      float                  radius,
                      int32_t                slice,
                      const math::Vector3Df& scale,
                      const math::Vector4Df& color) {
  if (height < 0) {
    height = 0.0f;
    assert(!"capsule height must be more than or equal zero.");
    return nullptr;
  }

  const float   stepRadian = math::g_degreeToRadian(360.0f / slice);
  const int32_t halfSlice  = slice / 2;

  const float        halfHeight = height / 2.0f;
  std::vector<float> vertices;
  vertices.reserve((halfSlice + 1) * (slice + 1) * 3);
  std::vector<float> normals;
  normals.reserve((halfSlice + 1) * (slice + 1) * 3);

  if (slice % 2) {
    ++slice;
  }

  for (int32_t j = 0; j <= halfSlice; ++j) {
    const bool isUpperSphere = (j > halfSlice / 2.0f);
    const bool isLowerSphere = (j < halfSlice / 2.0f);

    for (int32_t i = 0; i <= slice; ++i) {
      float x    = cosf(stepRadian * i) * radius * sinf(stepRadian * j);
      float y    = cosf(stepRadian * j) * radius;
      float z    = sinf(stepRadian * i) * radius * sinf(stepRadian * j);
      float yExt = 0.0f;
      if (isUpperSphere) {
        yExt = -halfHeight;
      }
      if (isLowerSphere) {
        yExt = halfHeight;
      }
      vertices.push_back(x);
      vertices.push_back(y + yExt);
      vertices.push_back(z);

      math::Vector3Df normal;
      if (!isUpperSphere && !isLowerSphere) {
        normal = math::Vector3Df(x, 0.0f, z);
      } else {
        normal = math::Vector3Df(x, y, z);
      }
      normal = normal.normalize();
      normals.push_back(normal.x());
      normals.push_back(normal.y());
      normals.push_back(normal.z());
    }
  }

  int32_t elementCount = static_cast<int32_t>(vertices.size() / 3);

  // attribute
  auto vertexStreamData = std::make_shared<VertexStreamData>();

  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("POSITION"),
        EBufferType::Static,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        std::vector<float>(vertices.begin(), vertices.end()));
    vertexStreamData->streams.push_back(streamParam);
  }

  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("COLOR"),
        EBufferType::Static,
        sizeof(float) * 4,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 4)},
        GenerateColor(color, elementCount));
    vertexStreamData->streams.push_back(streamParam);
  }

  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("NORMAL"),
        EBufferType::Static,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        std::vector<float>(normals.begin(), normals.end()));
    vertexStreamData->streams.push_back(streamParam);
  }

  // Dummy
  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("TANGENT"),
        EBufferType::Static,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        std::vector<float>(normals.size(), 0.0f));
    vertexStreamData->streams.push_back(streamParam);
  }

  // Dummy
  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("TEXCOORD"),
        EBufferType::Static,
        sizeof(float) * 2,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 2)},
        std::vector<float>(normals.size(), 0.0f));
    vertexStreamData->streams.push_back(streamParam);
  }

  vertexStreamData->PrimitiveType = EPrimitiveType::TRIANGLES;
  vertexStreamData->elementCount  = elementCount;

  // IndexStream

  std::vector<uint32_t> faces;
  faces.reserve((halfSlice + 1) * (slice - 1) * 6);
  int32_t iCount      = 0;
  int32_t toNextSlice = slice + 1;
  for (int32_t j = 0; j <= halfSlice; ++j) {
    for (int32_t i = 0; i < (slice - 1); ++i, iCount += 1) {
      faces.push_back(iCount + 1);
      faces.push_back(iCount);
      faces.push_back(iCount + toNextSlice);
      faces.push_back(iCount + 1);
      faces.push_back(iCount + toNextSlice);
      faces.push_back(iCount + toNextSlice + 1);
    }
  }

  auto indexStreamData          = std::make_shared<IndexStreamData>();
  indexStreamData->elementCount = static_cast<int32_t>(faces.size());
  {
    auto streamParam = new BufferAttributeStream<uint32_t>(
        Name("Index"),
        EBufferType::Static,
        sizeof(uint32_t) * 3,
        {IBufferAttribute::Attribute(
            EBufferElementType::UINT32, 0, sizeof(int32_t) * 3)},
        faces);
    indexStreamData->stream = streamParam;
  }

  auto object       = new Object();
  auto renderObject = new RenderObject();
  object->RenderObjectGeometryDataPtr
      = std::make_shared<RenderObjectGeometryData>(vertexStreamData,
                                                   indexStreamData);
  renderObject->CreateRenderObject(object->RenderObjectGeometryDataPtr);
  object->RenderObjects.push_back(renderObject);
  renderObject->SetPos(pos);
  renderObject->SetScale(scale);
  // object->CreateBoundBox();

  return object;
}

ConePrimitive* CreateCone(const math::Vector3Df& pos,
                          float                  height,
                          float                  radius,
                          int32_t                slice,
                          const math::Vector3Df& scale,
                          const math::Vector4Df& color,
                          bool                   isWireframe /*= false*/,
                          bool                   createBoundInfo /* = true*/) {
  const float           halfHeight = height / 2.0f;
  const math::Vector3Df topVert(0.0f, halfHeight, 0.0f);
  const math::Vector3Df bottomVert(0.0f, -halfHeight, 0.0f);

  if (slice % 2) {
    ++slice;
  }

  std::vector<float> vertices(slice * 18);
  float              stepRadian = math::g_degreeToRadian(360.0f / slice);
  for (int32_t i = 1; i <= slice; ++i) {
    const float rad     = i * stepRadian;
    const float prevRad = rad - stepRadian;

    math::Vector3Df* currentPos = (math::Vector3Df*)&vertices[(i - 1) * 18];
    // Top
    *currentPos = math::Vector3Df(
        cosf(rad) * radius, bottomVert.y(), sinf(rad) * radius);
    ++currentPos;
    *currentPos = math::Vector3Df(topVert);
    ++currentPos;
    *currentPos = math::Vector3Df(
        cosf(prevRad) * radius, bottomVert.y(), sinf(prevRad) * radius);
    ++currentPos;

    // Bottom
    *currentPos = math::Vector3Df(
        cosf(prevRad) * radius, bottomVert.y(), sinf(prevRad) * radius);
    ++currentPos;
    *currentPos = math::Vector3Df(bottomVert);
    ++currentPos;
    *currentPos = math::Vector3Df(
        cosf(rad) * radius, bottomVert.y(), sinf(rad) * radius);
    ++currentPos;
  }

  int32_t elementCount = static_cast<int32_t>(vertices.size() / 3);

  std::vector<float> normals(slice * 18);

  // lenght of the flank of the cone
  const float flank_len = sqrtf(radius * radius + height * height);

  // unit vector along the flank of the cone
  const float cone_x = radius / flank_len;
  const float cone_y = -height / flank_len;

  // Cone Top Normal
  for (int32_t i = 1; i <= slice; ++i) {
    const float           rad = i * stepRadian;
    const math::Vector3Df normal(
        -cone_y * cosf(rad), cone_x, -cone_y * sinf(rad));

    float* currentPos = &normals[(i - 1) * 18];

    // Top
    memcpy(&currentPos[0], &normal, sizeof(normal));
    memcpy(&currentPos[3], &normal, sizeof(normal));
    memcpy(&currentPos[6], &normal, sizeof(normal));

    // Bottom
    const math::Vector3Df temp(0.0f, -1.0f, 0.0f);
    memcpy(&currentPos[9], &temp, sizeof(temp));
    memcpy(&currentPos[12], &temp, sizeof(temp));
    memcpy(&currentPos[15], &temp, sizeof(temp));
  }

  // attribute
  auto vertexStreamData = std::make_shared<VertexStreamData>();

  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("POSITION"),
        EBufferType::Static,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        std::vector<float>(vertices.begin(), vertices.end()));
    vertexStreamData->streams.push_back(streamParam);
  }

  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("COLOR"),
        EBufferType::Static,
        sizeof(float) * 4,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 4)},
        GenerateColor(color, elementCount));
    vertexStreamData->streams.push_back(streamParam);
  }

  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("NORMAL"),
        EBufferType::Static,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        std::vector<float>(normals.begin(), normals.end()));
    vertexStreamData->streams.push_back(streamParam);
  }

  // Dummy
  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("TANGENT"),
        EBufferType::Static,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        std::vector<float>(normals.size(), 0.0f));
    vertexStreamData->streams.push_back(streamParam);
  }

  // Dummy
  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("TEXCOORD"),
        EBufferType::Static,
        sizeof(float) * 2,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 2)},
        std::vector<float>(normals.size(), 0.0f));
    vertexStreamData->streams.push_back(streamParam);
  }

  vertexStreamData->PrimitiveType
      = isWireframe ? EPrimitiveType::LINES : EPrimitiveType::TRIANGLES;
  vertexStreamData->elementCount = elementCount;

  auto object       = new ConePrimitive();
  auto renderObject = new RenderObject();
  object->RenderObjectGeometryDataPtr
      = std::make_shared<RenderObjectGeometryData>(vertexStreamData, nullptr);
  renderObject->CreateRenderObject(object->RenderObjectGeometryDataPtr);
  object->RenderObjects.push_back(renderObject);
  renderObject->SetPos(pos);
  renderObject->SetScale(scale);
  object->Height = height;
  object->Radius = radius;
  object->Color  = color;
  // if (createBoundInfo)
  //	object->CreateBoundBox();
  return object;
}

Object* CreateCylinder(const math::Vector3Df& pos,
                       float                  height,
                       float                  radius,
                       int32_t                slices,
                       const math::Vector3Df& scale,
                       const math::Vector4Df& color) {
  const auto            halfHeight = height / 2.0f;
  const math::Vector3Df topVert(0.0f, halfHeight, 0.0f);
  const math::Vector3Df bottomVert(0.0f, -halfHeight, 0.0f);

  if (slices % 2) {
    ++slices;
  }

  std::vector<BaseVertex> vertices(slices * 12);
  std::vector<uint32_t>   indices;

  const float stepRadian = math::g_degreeToRadian(360.0f / slices);
  for (int32_t i = 0; i < slices; ++i) {
    float rad     = i * stepRadian;
    float prevRad = rad - stepRadian;

    BaseVertex* CurrentBaseVertex = (BaseVertex*)&vertices[i * 12];

    // Top
    CurrentBaseVertex->Pos     = math::Vector3Df(topVert);
    CurrentBaseVertex->Normal  = math::Vector3Df(0.0f, 1.0f, 0.0f);
    CurrentBaseVertex->Tangent = math::Vector3Df(1.0f, 0.0f, 0.0f);
    CurrentBaseVertex->Bitangent
        = CurrentBaseVertex->Normal.cross(CurrentBaseVertex->Tangent);
    ++CurrentBaseVertex;

    CurrentBaseVertex->Pos = math::Vector3Df(
        cosf(prevRad) * radius, topVert.y(), sinf(prevRad) * radius);
    CurrentBaseVertex->Normal  = math::Vector3Df(0.0f, 1.0f, 0.0f);
    CurrentBaseVertex->Tangent = math::Vector3Df(1.0f, 0.0f, 0.0f);
    CurrentBaseVertex->Bitangent
        = CurrentBaseVertex->Normal.cross(CurrentBaseVertex->Tangent);
    ++CurrentBaseVertex;

    CurrentBaseVertex->Pos
        = math::Vector3Df(cosf(rad) * radius, topVert.y(), sinf(rad) * radius);
    CurrentBaseVertex->Normal  = math::Vector3Df(0.0f, 1.0f, 0.0f);
    CurrentBaseVertex->Tangent = math::Vector3Df(1.0f, 0.0f, 0.0f);
    CurrentBaseVertex->Bitangent
        = CurrentBaseVertex->Normal.cross(CurrentBaseVertex->Tangent);
    ++CurrentBaseVertex;

    // Mid
    CurrentBaseVertex->Pos = math::Vector3Df(
        cosf(prevRad) * radius, topVert.y(), sinf(prevRad) * radius);
    CurrentBaseVertex->Normal
        = math::Vector3Df(cosf(prevRad), 0.0f, sinf(prevRad));
    CurrentBaseVertex->Tangent = math::Vector3Df(0.0f, 1.0f, 0.0f);
    CurrentBaseVertex->Bitangent
        = CurrentBaseVertex->Normal.cross(CurrentBaseVertex->Tangent);
    ++CurrentBaseVertex;

    CurrentBaseVertex->Pos = math::Vector3Df(
        cosf(prevRad) * radius, bottomVert.y(), sinf(prevRad) * radius);
    CurrentBaseVertex->Normal
        = math::Vector3Df(cosf(prevRad), 0.0f, sinf(prevRad));
    CurrentBaseVertex->Tangent = math::Vector3Df(0.0f, 1.0f, 0.0f);
    CurrentBaseVertex->Bitangent
        = CurrentBaseVertex->Normal.cross(CurrentBaseVertex->Tangent);
    ++CurrentBaseVertex;

    CurrentBaseVertex->Pos
        = math::Vector3Df(cosf(rad) * radius, topVert.y(), sinf(rad) * radius);
    CurrentBaseVertex->Normal  = math::Vector3Df(cosf(rad), 0.0f, sinf(rad));
    CurrentBaseVertex->Tangent = math::Vector3Df(0.0f, 1.0f, 0.0f);
    CurrentBaseVertex->Bitangent
        = CurrentBaseVertex->Normal.cross(CurrentBaseVertex->Tangent);
    ++CurrentBaseVertex;

    CurrentBaseVertex->Pos = math::Vector3Df(
        cosf(prevRad) * radius, bottomVert.y(), sinf(prevRad) * radius);
    CurrentBaseVertex->Normal
        = math::Vector3Df(cosf(prevRad), 0.0f, sinf(prevRad));
    CurrentBaseVertex->Tangent = math::Vector3Df(0.0f, -1.0f, 0.0f);
    CurrentBaseVertex->Bitangent
        = CurrentBaseVertex->Normal.cross(CurrentBaseVertex->Tangent);
    ++CurrentBaseVertex;

    CurrentBaseVertex->Pos = math::Vector3Df(
        cosf(rad) * radius, bottomVert.y(), sinf(rad) * radius);
    CurrentBaseVertex->Normal  = math::Vector3Df(cosf(rad), 0.0f, sinf(rad));
    CurrentBaseVertex->Tangent = math::Vector3Df(0.0f, -1.0f, 0.0f);
    CurrentBaseVertex->Bitangent
        = CurrentBaseVertex->Normal.cross(CurrentBaseVertex->Tangent);
    ++CurrentBaseVertex;

    CurrentBaseVertex->Pos
        = math::Vector3Df(cosf(rad) * radius, topVert.y(), sinf(rad) * radius);
    CurrentBaseVertex->Normal  = math::Vector3Df(cosf(rad), 0.0f, sinf(rad));
    CurrentBaseVertex->Tangent = math::Vector3Df(0.0f, -1.0f, 0.0f);
    CurrentBaseVertex->Bitangent
        = CurrentBaseVertex->Normal.cross(CurrentBaseVertex->Tangent);
    ++CurrentBaseVertex;

    // Bottom
    CurrentBaseVertex->Pos
        = math::Vector3Df(bottomVert.x(), bottomVert.y(), bottomVert.z());
    CurrentBaseVertex->Normal  = math::Vector3Df(0.0f, -1.0f, 0.0f);
    CurrentBaseVertex->Tangent = math::Vector3Df(-1.0f, 0.0f, 0.0f);
    CurrentBaseVertex->Bitangent
        = CurrentBaseVertex->Normal.cross(CurrentBaseVertex->Tangent);
    ++CurrentBaseVertex;

    CurrentBaseVertex->Pos = math::Vector3Df(
        cosf(rad) * radius, bottomVert.y(), sinf(rad) * radius);
    CurrentBaseVertex->Normal  = math::Vector3Df(0.0f, -1.0f, 0.0f);
    CurrentBaseVertex->Tangent = math::Vector3Df(-1.0f, 0.0f, 0.0f);
    CurrentBaseVertex->Bitangent
        = CurrentBaseVertex->Normal.cross(CurrentBaseVertex->Tangent);
    ++CurrentBaseVertex;

    CurrentBaseVertex->Pos = math::Vector3Df(
        cosf(prevRad) * radius, bottomVert.y(), sinf(prevRad) * radius);
    CurrentBaseVertex->Normal  = math::Vector3Df(0.0f, -1.0f, 0.0f);
    CurrentBaseVertex->Tangent = math::Vector3Df(-1.0f, 0.0f, 0.0f);
    CurrentBaseVertex->Bitangent
        = CurrentBaseVertex->Normal.cross(CurrentBaseVertex->Tangent);
    ++CurrentBaseVertex;
  }

  indices.reserve(vertices.size());
  for (uint32_t i = 0; i < vertices.size(); ++i) {
    indices.push_back(i);
  }
  /////////////////////////////////////////////////////

  // PositionOnly VertexStream
  std::vector<PositionOnlyVertex> verticesPositionOnly(vertices.size());
  for (int32_t i = 0; i < (int32_t)vertices.size(); ++i) {
    verticesPositionOnly[i].Pos = vertices[i].Pos;
  }

  const int32_t NumOfVertices       = (int32_t)vertices.size();
  auto positionOnlyVertexStreamData = std::make_shared<VertexStreamData>();
  {
    auto streamParam
        = std::make_shared<BufferAttributeStream<PositionOnlyVertex>>(
            Name("PositionOnlyVertex"),
            EBufferType::Static,
            sizeof(PositionOnlyVertex),
            std::vector<IBufferAttribute::Attribute>{
              IBufferAttribute::Attribute(NameStatic("POSITION"),
                                          EBufferElementType::FLOAT,
                                          0,
                                          sizeof(float) * 3)},
            verticesPositionOnly);
    positionOnlyVertexStreamData->streams.push_back(streamParam);

    positionOnlyVertexStreamData->PrimitiveType = EPrimitiveType::TRIANGLES;
    positionOnlyVertexStreamData->elementCount  = NumOfVertices;
  }

  // Base VertexStream
  auto vertexStreamData = std::make_shared<VertexStreamData>();
  {
    auto streamParam = std::make_shared<BufferAttributeStream<BaseVertex>>(
        Name("BaseVertex"),
        EBufferType::Static,
        sizeof(BaseVertex),
        std::vector<IBufferAttribute::Attribute>{
          IBufferAttribute::Attribute(NameStatic("POSITION"),
                                      EBufferElementType::FLOAT,
                                      0,
                                      sizeof(float) * 3),
          IBufferAttribute::Attribute(NameStatic("NORMAL"),
                                      EBufferElementType::FLOAT,
                                      sizeof(float) * 3,
                                      sizeof(float) * 3),
          IBufferAttribute::Attribute(NameStatic("TANGENT"),
                                      EBufferElementType::FLOAT,
                                      sizeof(float) * 6,
                                      sizeof(float) * 3),
          IBufferAttribute::Attribute(NameStatic("BITANGENT"),
                                      EBufferElementType::FLOAT,
                                      sizeof(float) * 9,
                                      sizeof(float) * 3),
          IBufferAttribute::Attribute(NameStatic("TEXCOORD"),
                                      EBufferElementType::FLOAT,
                                      sizeof(float) * 12,
                                      sizeof(float) * 2)},
        vertices);
    vertexStreamData->streams.push_back(streamParam);

    vertexStreamData->PrimitiveType = EPrimitiveType::TRIANGLES;
    vertexStreamData->elementCount  = NumOfVertices;
  }

  auto indexStreamData          = std::make_shared<IndexStreamData>();
  indexStreamData->elementCount = static_cast<int32_t>(indices.size());
  {
    auto streamParam = new BufferAttributeStream<uint32_t>(
        Name("Index"),
        EBufferType::Static,
        sizeof(uint32_t) * 3,
        {IBufferAttribute::Attribute(
            EBufferElementType::UINT32, 0, sizeof(int32_t) * 3)},
        indices);
    indexStreamData->stream = streamParam;
  }

  auto object       = new Object();
  auto renderObject = new RenderObject();
  object->RenderObjectGeometryDataPtr
      = std::make_shared<RenderObjectGeometryData>(
          vertexStreamData, positionOnlyVertexStreamData, indexStreamData);
  renderObject->CreateRenderObject(object->RenderObjectGeometryDataPtr);
  object->RenderObjects.push_back(renderObject);
  renderObject->SetPos(pos);
  renderObject->SetScale(scale);
  renderObject->MaterialPtr = GDefaultMaterial;
  return object;
}

Object* CreateSphere(const math::Vector3Df& pos,
                     float                  radius,
                     uint32_t               slices,
                     uint32_t               stacks,
                     const math::Vector3Df& scale,
                     const math::Vector4Df& color,
                     bool                   isWireframe /*= false*/,
                     bool                   createBoundInfo /*= true*/) {
  const auto offset = math::g_zeroVector<float, 3>();

  if (slices < 6) {
    slices = 6;
  } else if (slices % 2) {
    ++slices;
  }

  if (stacks < 6) {
    stacks = 6;
  } else if (stacks % 2) {
    ++stacks;
  }

  //////////////////////////////////////////////////////////////////////////
  std::vector<BaseVertex> vertices;
  const float             sectorStep = 2 * math::g_kPi / slices;
  const float             stackStep  = math::g_kPi / stacks;

  for (uint32_t i = 0; i <= stacks; ++i) {
    const float stackAngle
        = math::g_kPi / 2
        - i * stackStep;  // stack angle in the range [-π/2, π/2]
    const float xy = radius * cosf(stackAngle);
    const float z  = radius * sinf(stackAngle);

    for (uint32_t j = 0; j <= slices; ++j) {
      float sectorAngle = j * sectorStep;  // sector angle in the range [0, 2π]

      // Vertex position
      const float           x = xy * cosf(sectorAngle);
      const float           y = xy * sinf(sectorAngle);
      const math::Vector3Df pos(x, z, y);  // swap z, y, because z is up vector.

      // Normalized vertex normal
      math::Vector3Df normal = pos.normalize();

      // Tangent and bitangent
      const math::Vector3Df tangent(
          -sinf(sectorAngle), cosf(sectorAngle), 0.0f);
      const math::Vector3Df bitangent = normal.cross(tangent);

      // UV coordinates
      const float           u = (float)j / slices;
      const float           v = (float)i / stacks;
      const math::Vector2Df uv(u, v);

      vertices.push_back({pos, normal, tangent, bitangent, uv});
    }
  }

  // Indices
  std::vector<uint32_t> indices;
  int                   k1, k2;
  for (uint32_t i = 0; i < stacks; ++i) {
    k1 = i * (slices + 1);
    k2 = k1 + slices + 1;

    for (uint32_t j = 0; j < slices; ++j, ++k1, ++k2) {
      if (i != 0) {
        indices.push_back(k1);
        indices.push_back(k2);
        indices.push_back(k1 + 1);
      }
      if (i != (stacks - 1)) {
        indices.push_back(k1 + 1);
        indices.push_back(k2);
        indices.push_back(k2 + 1);
      }
    }
  }
  //////////////////////////////////////////////////////////////////////////

  // PositionOnly VertexStream
  std::vector<PositionOnlyVertex> verticesPositionOnly(vertices.size());
  for (int32_t i = 0; i < (int32_t)vertices.size(); ++i) {
    verticesPositionOnly[i].Pos = vertices[i].Pos;
  }

  const int32_t NumOfVertices       = (int32_t)vertices.size();
  auto positionOnlyVertexStreamData = std::make_shared<VertexStreamData>();
  {
    auto streamParam
        = std::make_shared<BufferAttributeStream<PositionOnlyVertex>>(
            Name("PositionOnlyVertex"),
            EBufferType::Static,
            sizeof(PositionOnlyVertex),
            std::vector<IBufferAttribute::Attribute>{
              IBufferAttribute::Attribute(NameStatic("POSITION"),
                                          EBufferElementType::FLOAT,
                                          0,
                                          sizeof(float) * 3)},
            verticesPositionOnly);
    positionOnlyVertexStreamData->streams.push_back(streamParam);

    positionOnlyVertexStreamData->PrimitiveType
        = isWireframe ? EPrimitiveType::LINES : EPrimitiveType::TRIANGLES;
    positionOnlyVertexStreamData->elementCount = NumOfVertices;
  }

  // Base VertexStream
  auto vertexStreamData = std::make_shared<VertexStreamData>();
  {
    auto streamParam = std::make_shared<BufferAttributeStream<BaseVertex>>(
        Name("BaseVertex"),
        EBufferType::Static,
        sizeof(BaseVertex),
        std::vector<IBufferAttribute::Attribute>{
          IBufferAttribute::Attribute(NameStatic("POSITION"),
                                      EBufferElementType::FLOAT,
                                      0,
                                      sizeof(float) * 3),
          IBufferAttribute::Attribute(NameStatic("NORMAL"),
                                      EBufferElementType::FLOAT,
                                      sizeof(float) * 3,
                                      sizeof(float) * 3),
          IBufferAttribute::Attribute(NameStatic("TANGENT"),
                                      EBufferElementType::FLOAT,
                                      sizeof(float) * 6,
                                      sizeof(float) * 3),
          IBufferAttribute::Attribute(NameStatic("BITANGENT"),
                                      EBufferElementType::FLOAT,
                                      sizeof(float) * 9,
                                      sizeof(float) * 3),
          IBufferAttribute::Attribute(NameStatic("TEXCOORD"),
                                      EBufferElementType::FLOAT,
                                      sizeof(float) * 12,
                                      sizeof(float) * 2)},
        vertices);
    vertexStreamData->streams.push_back(streamParam);

    vertexStreamData->PrimitiveType
        = isWireframe ? EPrimitiveType::LINES : EPrimitiveType::TRIANGLES;
    vertexStreamData->elementCount = NumOfVertices;
  }

  auto indexStreamData          = std::make_shared<IndexStreamData>();
  indexStreamData->elementCount = static_cast<int32_t>(indices.size());
  {
    auto streamParam = new BufferAttributeStream<uint32_t>(
        Name("Index"),
        EBufferType::Static,
        sizeof(uint32_t) * 3,
        {IBufferAttribute::Attribute(
            EBufferElementType::UINT32, 0, sizeof(int32_t) * 3)},
        indices);
    // streamParam->Data.resize(indices.size());
    // memcpy(
    //     &streamParam->Data[0], &indices[0], indices.size() *
    //     sizeof(uint32_t));
    indexStreamData->stream = streamParam;
  }

  auto object       = new Object();
  auto renderObject = new RenderObject();
  object->RenderObjectGeometryDataPtr
      = std::make_shared<RenderObjectGeometryData>(
          vertexStreamData, positionOnlyVertexStreamData, indexStreamData);
  renderObject->CreateRenderObject(object->RenderObjectGeometryDataPtr);
  object->RenderObjects.push_back(renderObject);
  renderObject->SetPos(pos);
  renderObject->SetScale(scale);
  renderObject->MaterialPtr = GDefaultMaterial;
  return object;
}

BillboardQuadPrimitive* CreateBillobardQuad(const math::Vector3Df& pos,
                                            const math::Vector3Df& size,
                                            const math::Vector3Df& scale,
                                            const math::Vector4Df& color,
                                            Camera*                camera) {
  auto object       = new BillboardQuadPrimitive();
  auto RenderObject = CreateQuad_Internal(pos, size, scale, color);
  object->RenderObjectGeometryDataPtr = RenderObject->GeometryDataPtr;
  object->RenderObjects.push_back(RenderObject);
  object->camera           = camera;
  RenderObject->IsTwoSided = true;
  return object;
}

UIQuadPrimitive* CreateUIQuad(const math::Vector2Df& pos,
                              const math::Vector2Df& size,
                              TextureVk*             texture) {
  float vertices[] = {
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    1.0f,
    1.0f,
    1.0f,
    0.0f,
  };

  if (useVulkanNdcYFlip) {
    int32_t SrcIdx = 1;
    int32_t DstIdx = 2;
    std::swap(vertices[SrcIdx * 2], vertices[DstIdx * 2]);
    std::swap(vertices[SrcIdx * 2 + 1], vertices[DstIdx * 2 + 1]);
  }

  int32_t elementCount = std::size(vertices) / 2;

  // attribute
  auto vertexStreamData = std::make_shared<VertexStreamData>();

  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("VertPos"),
        EBufferType::Static,
        sizeof(float) * 2,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 2)},
        std::vector<float>(vertices, vertices + std::size(vertices)));
    vertexStreamData->streams.push_back(streamParam);
  }

  vertexStreamData->PrimitiveType = EPrimitiveType::TRIANGLE_STRIP;
  vertexStreamData->elementCount  = elementCount;

  auto object       = new UIQuadPrimitive();
  auto renderObject = new RenderObject();
  object->RenderObjectGeometryDataPtr
      = std::make_shared<RenderObjectGeometryData>(vertexStreamData, nullptr);
  renderObject->CreateRenderObject(object->RenderObjectGeometryDataPtr);
  object->RenderObjects.push_back(renderObject);
  object->Pos  = pos;
  object->Size = size;

  return object;
}

FullscreenQuadPrimitive* CreateFullscreenQuad(TextureVk* texture) {
  float vertices[] = {0.0f, 1.0f, 2.0f};

  uint32_t elementCount = static_cast<uint32_t>(std::size(vertices));
  // attribute
  auto     vertexStreamData = std::make_shared<VertexStreamData>();

  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("POSITION"),
        EBufferType::Static,
        sizeof(float),
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float))},
        std::vector<float>(std::begin(vertices), std::end(vertices)));
    vertexStreamData->streams.push_back(streamParam);
  }

  vertexStreamData->PrimitiveType = EPrimitiveType::TRIANGLE_STRIP;
  vertexStreamData->elementCount  = elementCount;

  auto object       = new FullscreenQuadPrimitive();
  auto renderObject = new RenderObject();
  object->RenderObjectGeometryDataPtr
      = std::make_shared<RenderObjectGeometryData>(vertexStreamData, nullptr);
  renderObject->CreateRenderObject(object->RenderObjectGeometryDataPtr);
  object->RenderObjects.push_back(renderObject);
  return object;
}

SegmentPrimitive* CreateSegment(const math::Vector3Df& start,
                                const math::Vector3Df& end,
                                float                  time,
                                const math::Vector4Df& color) {
  math::Vector3Df currentEnd(math::g_zeroVector<float, 3>());
  if (time < 1.0) {
    float t    = std::clamp(time, 0.0f, 1.0f);
    currentEnd = (end - start) + start;
  } else {
    currentEnd = end;
  }

  float vertices[] = {
    start.x(),
    start.y(),
    start.z(),
    currentEnd.x(),
    currentEnd.y(),
    currentEnd.z(),
  };

  int32_t elementCount = static_cast<int32_t>(std::size(vertices) / 3);

  // attribute
  auto vertexStreamData = std::make_shared<VertexStreamData>();

  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("POSITION"),
        EBufferType::Dynamic,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        std::vector<float>(vertices, vertices + std::size(vertices)));
    vertexStreamData->streams.push_back(streamParam);
  }

  {
    auto colorData   = GenerateColor(color, elementCount);
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("COLOR"),
        EBufferType::Static,
        sizeof(float) * 4,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 4)},
        std::move(colorData));
    vertexStreamData->streams.push_back(streamParam);
  }

  vertexStreamData->PrimitiveType = EPrimitiveType::LINES;
  vertexStreamData->elementCount  = elementCount;

  auto object       = new SegmentPrimitive();
  auto renderObject = new RenderObject();
  object->RenderObjectGeometryDataPtr
      = std::make_shared<RenderObjectGeometryData>(vertexStreamData, nullptr);
  renderObject->CreateRenderObject(object->RenderObjectGeometryDataPtr);
  object->RenderObjects.push_back(renderObject);
  renderObject = renderObject;
  renderObject->SetScale(math::Vector3Df(time));
  object->Time           = time;
  object->Start          = start;
  object->End            = end;
  object->Color          = color;
  object->PostUpdateFunc = [](Object* thisObject, float deltaTime) {
    auto thisSegmentObject = static_cast<SegmentPrimitive*>(thisObject);
    thisObject->RenderObjects[0]->SetScale(
        math::Vector3Df(thisSegmentObject->Time));
  };
  // object->CreateBoundBox();
  return object;
}

ArrowSegmentPrimitive* CreateArrowSegment(const math::Vector3Df& start,
                                          const math::Vector3Df& end,
                                          float                  time,
                                          float                  coneHeight,
                                          float                  coneRadius,
                                          const math::Vector4Df& color) {
  auto object            = new ArrowSegmentPrimitive();
  object->SegmentObject  = CreateSegment(start, end, time, color);
  object->ConeObject     = CreateCone(math::g_zeroVector<float, 3>(),
                                  coneHeight,
                                  coneRadius,
                                  10,
                                  math::g_oneVector<float, 3>(),
                                  color);
  object->PostUpdateFunc = [](Object* thisObject, float deltaTime) {
    auto thisArrowSegmentObject
        = static_cast<ArrowSegmentPrimitive*>(thisObject);
    thisArrowSegmentObject->ConeObject->RenderObjects[0]->SetPos(
        thisArrowSegmentObject->SegmentObject->RenderObjects[0]->GetPos()
        + thisArrowSegmentObject->SegmentObject->GetCurrentEnd());
    thisArrowSegmentObject->ConeObject->RenderObjects[0]->SetRot(
        math::GetEulerAngleFrom(
            thisArrowSegmentObject->SegmentObject->GetDirectionNormalized()));
  };

  return object;
}

// DirectionalLightPrimitive* CreateDirectionalLightDebug(
//     const math::Vector3Df& pos,
//     const math::Vector3Df& scale,
//     float                  length,
//     Camera*               targetCamera,
//     DirectionalLight*     light,
//     const char*            textureFilename) {
//   DirectionalLightPrimitive* object = new DirectionalLightPrimitive();
//
//   std::weak_ptr<jImageData> data
//       = ImageFileLoader::GetInstance().LoadImageDataFromFile(
//           Name(textureFilename));
//   object->BillboardObject = CreateBillobardQuad(pos,
//                                                 math::g_oneVector<float,
//                                                 3>(), scale,
//                                                 math::Vector4Df(1.0f),
//                                                 targetCamera);
//   if (data.lock()->imageBulkData.ImageData.size() > 0) {
//     TextureVk* texture = ImageFileLoader::GetInstance()
//                              .LoadTextureFromFile(Name(textureFilename))
//                              .lock()
//                              .get();
//     object->BillboardObject->RenderObjects[0]->MaterialPtr
//         = std::make_shared<jMaterial>();
//     object->BillboardObject->RenderObjects[0]
//         ->MaterialPtr
//         ->TexData[static_cast<int32_t>(jMaterial::EMaterialTextureType::Albedo)]
//         .Texture
//         = texture;
//     object->BillboardObject->RenderObjects[0]->IsHiddenBoundBox = true;
//   }
//   object->ArrowSegementObject
//       = CreateArrowSegment(math::g_zeroVector<float, 3>(),
//                            light->GetLightData().Direction * length,
//                            1.0f,
//                            scale.x(),
//                            scale.x() / 2,
//                            math::Vector4Df(0.8f, 0.2f, 0.3f, 1.0f));
//   object->ArrowSegementObject->ConeObject->RenderObjects[0]->IsHiddenBoundBox
//       = true;
//   object->ArrowSegementObject->SegmentObject->RenderObjects[0]->IsHiddenBoundBox
//       = true;
//   object->Pos            = pos;
//   object->Light          = light;
//   object->PostUpdateFunc = [length](Object* thisObject, float deltaTime) {
//     auto thisDirectionalLightObject
//         = static_cast<DirectionalLightPrimitive*>(thisObject);
//     thisDirectionalLightObject->BillboardObject->RenderObjects[0]->SetPos(
//         thisDirectionalLightObject->Pos);
//     thisDirectionalLightObject->ArrowSegementObject->SetPos(
//         thisDirectionalLightObject->Pos);
//     thisDirectionalLightObject->ArrowSegementObject->SetEnd(
//         thisDirectionalLightObject->Light->GetLightData().Direction *
//         length);
//   };
//   object->SkipShadowMapGen       = true;
//   object->SkipUpdateShadowVolume = true;
//   light->LightDebugObject        = object;
//   return object;
// }
//
// PointLightPrimitive* CreatePointLightDebug(const math::Vector3Df& scale,
//                                             Camera* targetCamera,
//                                             PointLight*           light,
//                                             const char* textureFilename) {
//   PointLightPrimitive* object = new PointLightPrimitive();
//
//   std::weak_ptr<jImageData> data
//       = ImageFileLoader::GetInstance().LoadImageDataFromFile(
//           Name(textureFilename));
//   const PointLightUniformBufferData& LightData = light->GetLightData();
//   object->BillboardObject = CreateBillobardQuad(LightData.Position,
//                                                 math::g_oneVector<float,
//                                                 3>(), scale,
//                                                 math::Vector4Df(1.0f),
//                                                 targetCamera);
//   if (data.lock()->imageBulkData.ImageData.size() > 0) {
//     auto texture = ImageFileLoader::GetInstance()
//                        .LoadTextureFromFile(Name(textureFilename))
//                        .lock()
//                        .get();
//     object->BillboardObject->RenderObjects[0]->MaterialPtr
//         = std::make_shared<jMaterial>();
//     object->BillboardObject->RenderObjects[0]
//         ->MaterialPtr
//         ->TexData[static_cast<int32_t>(jMaterial::EMaterialTextureType::Albedo)]
//         .Texture
//         = texture;
//     object->BillboardObject->RenderObjects[0]->IsHiddenBoundBox = true;
//   }
//   object->SphereObject = CreateSphere(LightData.Position,
//                                       LightData.MaxDistance,
//                                       20,
//                                       10,
//                                       math::g_oneVector<float, 3>(),
//                                       math::Vector4Df(LightData.Color, 1.0f),
//                                       true,
//                                       false);
//   object->SphereObject->RenderObjects[0]->IsHiddenBoundBox = true;
//   object->Light                                            = light;
//   object->PostUpdateFunc = [](Object* thisObject, float deltaTime) {
//     auto pointLightPrimitive =
//     static_cast<PointLightPrimitive*>(thisObject); const
//     PointLightUniformBufferData& LightData
//         = pointLightPrimitive->Light->GetLightData();
//     pointLightPrimitive->BillboardObject->RenderObjects[0]->SetPos(
//         LightData.Position);
//     pointLightPrimitive->SphereObject->RenderObjects[0]->SetPos(
//         LightData.Position);
//     pointLightPrimitive->SphereObject->RenderObjects[0]->SetScale(
//         math::Vector3Df(LightData.MaxDistance));
//   };
//   object->SkipShadowMapGen       = true;
//   object->SkipUpdateShadowVolume = true;
//   light->LightDebugObject        = object;
//   return object;
// }
//
// SpotLightPrimitive* CreateSpotLightDebug(const math::Vector3Df& scale,
//                                           Camera* targetCamera, SpotLight*
//                                           light, const char* textureFilename)
//                                           {
//   SpotLightPrimitive* object = new SpotLightPrimitive();
//
//   std::weak_ptr<jImageData> data
//       = ImageFileLoader::GetInstance().LoadImageDataFromFile(
//           Name(textureFilename));
//   const SpotLightUniformBufferData& LightData = light->GetLightData();
//   object->BillboardObject = CreateBillobardQuad(LightData.Position,
//                                                 math::g_oneVector<float,
//                                                 3>(), scale,
//                                                 math::Vector4Df(1.0f),
//                                                 targetCamera);
//   if (data.lock()->imageBulkData.ImageData.size() > 0) {
//     auto texture = ImageFileLoader::GetInstance()
//                        .LoadTextureFromFile(Name(textureFilename))
//                        .lock()
//                        .get();
//     object->BillboardObject->RenderObjects[0]->MaterialPtr
//         = std::make_shared<jMaterial>();
//     object->BillboardObject->RenderObjects[0]
//         ->MaterialPtr
//         ->TexData[static_cast<int32_t>(jMaterial::EMaterialTextureType::Albedo)]
//         .Texture
//         = texture;
//   }
//   object->UmbraConeObject = CreateCone(
//       LightData.Position,
//       1.0,
//       1.0,
//       20,
//       math::g_oneVector<float, 3>(),
//       math::Vector4Df(
//           LightData.Color.x(), LightData.Color.y(),
//           LightData.Color.z(), 1.0f),
//       true,
//       false);
//   object->UmbraConeObject->RenderObjects[0]->IsHiddenBoundBox = true;
//   object->PenumbraConeObject                                  = CreateCone(
//       LightData.Position,
//       1.0,
//       1.0,
//       20,
//       math::g_oneVector<float, 3>(),
//       math::Vector4Df(
//           LightData.Color.x(), LightData.Color.y(), LightData.Color.z(),
//           0.5f),
//       true,
//       false);
//   object->PenumbraConeObject->RenderObjects[0]->IsHiddenBoundBox = true;
//   object->Light                                                  = light;
//   object->PostUpdateFunc = [](Object* thisObject, float deltaTime) {
//     auto spotLightObject = static_cast<SpotLightPrimitive*>(thisObject);
//     const SpotLightUniformBufferData& LightData
//         = spotLightObject->Light->GetLightData();
//     spotLightObject->BillboardObject->RenderObjects[0]->SetPos(
//         LightData.Position);
//
//     const auto lightDir       = -LightData.Direction;
//     const auto directionToRot = math::GetEulerAngleFrom(lightDir);
//     const auto spotLightPos
//         = LightData.Position
//         + lightDir
//               * (-spotLightObject->UmbraConeObject->RenderObjects[0]
//                       ->GetScale()
//                       .y()
//                  / 2.0f);
//
//     const auto umbraRadius
//         = tanf(LightData.UmbraRadian) * LightData.MaxDistance;
//     spotLightObject->UmbraConeObject->RenderObjects[0]->SetScale(
//         math::Vector3Df(umbraRadius, LightData.MaxDistance, umbraRadius));
//     spotLightObject->UmbraConeObject->RenderObjects[0]->SetPos(spotLightPos);
//     spotLightObject->UmbraConeObject->RenderObjects[0]->SetRot(directionToRot);
//
//     const auto penumbraRadius
//         = tanf(LightData.PenumbraRadian) * LightData.MaxDistance;
//     spotLightObject->PenumbraConeObject->RenderObjects[0]->SetScale(
//         math::Vector3Df(penumbraRadius, LightData.MaxDistance,
//         penumbraRadius));
//     spotLightObject->PenumbraConeObject->RenderObjects[0]->SetPos(spotLightPos);
//     spotLightObject->PenumbraConeObject->RenderObjects[0]->SetRot(
//         directionToRot);
//   };
//   object->SkipShadowMapGen       = true;
//   object->SkipUpdateShadowVolume = true;
//   light->LightDebugObject        = object;
//   return object;
// }

FrustumPrimitive* CreateFrustumDebug(const Camera* targetCamera) {
  FrustumPrimitive* frustumPrimitive = new FrustumPrimitive(targetCamera);
  for (int32_t i = 0; i < 16; ++i) {
    frustumPrimitive->Segments[i]
        = CreateSegment(math::g_zeroVector<float, 3>(),
                        math::g_zeroVector<float, 3>(),
                        1.0f,
                        math::Vector4Df(1.0f));
    frustumPrimitive->Segments[i]->IsPostUpdate = false;
  }

  for (int32_t i = 0; i < 6; ++i) {
    frustumPrimitive->Plane[i] = CreateQuad(math::g_zeroVector<float, 3>(),
                                            math::g_oneVector<float, 3>(),
                                            math::g_oneVector<float, 3>(),
                                            math::Vector4Df(1.0f));
  }

  return frustumPrimitive;
}

Graph2D* CreateGraph2D(const math::Vector2Df&              pos,
                       const math::Vector2Df&              size,
                       const std::vector<math::Vector2Df>& points) {
  const math::Vector2Df point[4] = {
    {0.0f, 0.0f},
    {1.0f, 0.0f},
    {0.0f, 1.0f},
    {1.0f, 1.0f}
  };

  const int32_t elementCount = 4;

  // attribute
  auto vertexStreamData = std::make_shared<VertexStreamData>();

  {
    auto streamParam = std::make_shared<BufferAttributeStream<math::Vector2Df>>(
        Name("POSITION"),
        EBufferType::Static,
        math::Vector2Df::GetDataSize(),
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 2)},
        std::vector<math::Vector2Df>(point, point + std::size(point)));
    vertexStreamData->streams.push_back(streamParam);
  }

  {
    auto streamParam = std::make_shared<BufferAttributeStream<math::Matrix4f>>(
        Name("Transform"),
        EBufferType::Dynamic,
        math::Matrix4f::GetDataSize(),
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 4)},
        std::vector<math::Matrix4f>()
        //, 1 // InstanceDivisor // TODO probably not needed, remove
    );
    vertexStreamData->streams.push_back(streamParam);
  }

  vertexStreamData->PrimitiveType = EPrimitiveType::TRIANGLE_STRIP;
  vertexStreamData->elementCount  = elementCount;

  auto object       = new Graph2D();
  auto renderObject = new RenderObject();
  object->RenderObjectGeometryDataPtr
      = std::make_shared<RenderObjectGeometryData>(vertexStreamData, nullptr);
  renderObject->CreateRenderObject(object->RenderObjectGeometryDataPtr);
  object->RenderObjects.push_back(renderObject);
  object->SethPos(pos);
  object->SetGuardLineSize(size);
  object->SetPoints(points);

  return object;
}

// void DirectionalLightPrimitive::Update(float deltaTime) {
//   Object::Update(deltaTime);
//
//   if (BillboardObject) {
//     BillboardObject->Update(deltaTime);
//   }
//   if (ArrowSegementObject) {
//     ArrowSegementObject->Update(deltaTime);
//   }
// }
//
// void PointLightPrimitive::Update(float deltaTime) {
//   Object::Update(deltaTime);
//   BillboardObject->Update(deltaTime);
//   SphereObject->Update(deltaTime);
// }
//
// void SpotLightPrimitive::Update(float deltaTime) {
//   Object::Update(deltaTime);
//   BillboardObject->Update(deltaTime);
//   UmbraConeObject->Update(deltaTime);
//   PenumbraConeObject->Update(deltaTime);
// }

void SegmentPrimitive::UpdateSegment() {
  if (RenderObjectGeometryDataPtr->VertexStreamPtr->streams.size() < 2) {
    assert(0);
    return;
  }

  RenderObjectGeometryDataPtr->VertexStreamPtr->streams[0].reset();
  RenderObjectGeometryDataPtr->VertexStreamPtr->streams[1].reset();
  const auto currentEnd = GetCurrentEnd();

  const float vertices[] = {
    Start.x(),
    Start.y(),
    Start.z(),
    currentEnd.x(),
    currentEnd.y(),
    currentEnd.z(),
  };

  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("POSITION"),
        EBufferType::Dynamic,
        sizeof(float) * 3,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
        std::vector<float>(std::begin(vertices), std::end(vertices)));
    RenderObjectGeometryDataPtr->VertexStreamPtr->streams[0] = streamParam;
  }

  {
    auto streamParam = std::make_shared<BufferAttributeStream<float>>(
        Name("COLOR"),
        EBufferType::Static,
        sizeof(float) * 4,
        std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
            EBufferElementType::FLOAT, 0, sizeof(float) * 4)},
        GenerateColor(Color, 2));
    RenderObjectGeometryDataPtr->VertexStreamPtr->streams[1] = streamParam;
  }
}

void SegmentPrimitive::UpdateSegment(const math::Vector3Df& start,
                                     const math::Vector3Df& end,
                                     const math::Vector4Df& color,
                                     float                  time) {
  Start = start;
  End   = end;
  Color = color;
  Time  = time;
  UpdateSegment();
}

void SegmentPrimitive::Update(float deltaTime) {
  Object::Update(deltaTime);

  UpdateSegment();
}

void FrustumPrimitive::Update(float deltaTime) {
  math::Vector3Df far_lt;
  math::Vector3Df far_rt;
  math::Vector3Df far_lb;
  math::Vector3Df far_rb;

  math::Vector3Df near_lt;
  math::Vector3Df near_rt;
  math::Vector3Df near_lb;
  math::Vector3Df near_rb;

  const auto  origin = TargetCamera->Pos;
  const float n      = TargetCamera->Near;
  const float f      = TargetCamera->Far;

  if (TargetCamera->IsPerspectiveProjection) {
    const float InvAspect
        = ((float)TargetCamera->Width / (float)TargetCamera->Height);
    const float     length    = tanf(TargetCamera->FOVRad * 0.5f);
    math::Vector3Df targetVec = TargetCamera->GetForwardVector().normalize();
    math::Vector3Df rightVec
        = TargetCamera->GetRightVector() * length * InvAspect;
    math::Vector3Df upVec = TargetCamera->GetUpVector() * length;

    math::Vector3Df rightUp   = (targetVec + rightVec + upVec);
    math::Vector3Df leftUp    = (targetVec - rightVec + upVec);
    math::Vector3Df rightDown = (targetVec + rightVec - upVec);
    math::Vector3Df leftDown  = (targetVec - rightVec - upVec);

    far_lt = origin + leftUp * f;
    far_rt = origin + rightUp * f;
    far_lb = origin + leftDown * f;
    far_rb = origin + rightDown * f;

    near_lt = origin + leftUp * n;
    near_rt = origin + rightUp * n;
    near_lb = origin + leftDown * n;
    near_rb = origin + rightDown * n;

    if (PostPerspective) {
      auto ProjView = TargetCamera->Projection * TargetCamera->view;

      far_lt = math::g_transformPoint(far_rt, ProjView);
      far_rt = math::g_transformPoint(far_rt, ProjView);
      far_lb = math::g_transformPoint(far_lb, ProjView);
      far_rb = math::g_transformPoint(far_rb, ProjView);

      near_lt = math::g_transformPoint(near_lt, ProjView);
      near_rt = math::g_transformPoint(near_rt, ProjView);
      near_lb = math::g_transformPoint(near_lb, ProjView);
      near_rb = math::g_transformPoint(near_rb, ProjView);
    }
  } else {
    const float w = (float)TargetCamera->Width;
    const float h = (float)TargetCamera->Height;

    math::Vector3Df targetVec = TargetCamera->GetForwardVector().normalize();
    math::Vector3Df rightVec  = TargetCamera->GetRightVector().normalize();
    math::Vector3Df upVec     = TargetCamera->GetUpVector().normalize();

    far_lt = origin + targetVec * f - rightVec * w * 0.5f + upVec * h * 0.5f;
    far_rt = origin + targetVec * f + rightVec * w * 0.5f + upVec * h * 0.5f;
    far_lb = origin + targetVec * f - rightVec * w * 0.5f - upVec * h * 0.5f;
    far_rb = origin + targetVec * f + rightVec * w * 0.5f - upVec * h * 0.5f;

    near_lt = origin + targetVec * n - rightVec * w * 0.5f + upVec * h * 0.5f;
    near_rt = origin + targetVec * n + rightVec * w * 0.5f + upVec * h * 0.5f;
    near_lb = origin + targetVec * n - rightVec * w * 0.5f - upVec * h * 0.5f;
    near_rb = origin + targetVec * n + rightVec * w * 0.5f - upVec * h * 0.5f;
  }

  const math::Vector4Df baseColor
      = (TargetCamera->IsPerspectiveProjection
             ? math::Vector4Df(1.0f)
             : math::Vector4Df(0.0f, 0.0f, 1.0f, 1.0f));
  Segments[0]->UpdateSegment(near_rt, far_rt, baseColor);
  Segments[1]->UpdateSegment(near_lt, far_lt, baseColor);
  Segments[2]->UpdateSegment(near_rb, far_rb, baseColor);
  Segments[3]->UpdateSegment(near_lb, far_lb, baseColor);

  const math::Vector4Df green(0.0f, 1.0f, 0.0f, 1.0f);
  Segments[4]->UpdateSegment(near_lt, near_rt, green);
  Segments[5]->UpdateSegment(near_lb, near_rb, green);
  Segments[6]->UpdateSegment(
      near_lt, near_lb, math::Vector4Df(1.0f, 1.0f, 0.0f, 1.0f));
  Segments[7]->UpdateSegment(near_rt, near_rb, green);

  const math::Vector4Df red(1.0f, 0.0f, 0.0f, 1.0f);
  Segments[8]->UpdateSegment(far_lt, far_rt, red);
  Segments[9]->UpdateSegment(far_lb, far_rb, red);
  Segments[10]->UpdateSegment(
      far_lt, far_lb, math::Vector4Df(0.0f, 1.0f, 1.0f, 1.0f));
  Segments[11]->UpdateSegment(far_rt, far_rb, red);

  Segments[12]->UpdateSegment(far_rt, near_rt, baseColor);
  Segments[13]->UpdateSegment(far_rb, near_rb, baseColor);
  Segments[14]->UpdateSegment(far_lb, near_lb, baseColor);
  Segments[15]->UpdateSegment(far_rb, near_rb, baseColor);

  auto updateQuadFunc = [this](QuadPrimitive*         quad,
                               const math::Vector3Df& p1,
                               const math::Vector3Df& p2,
                               const math::Vector3Df& p3,
                               const math::Vector3Df& p4,
                               const math::Vector4Df& color) {
    float vertices[] = {
      p1.x(),
      p1.y(),
      p1.z(),
      p2.x(),
      p2.y(),
      p2.z(),
      p3.x(),
      p3.y(),
      p3.z(),
      p3.x(),
      p3.y(),
      p3.z(),
      p2.x(),
      p2.y(),
      p2.z(),
      p4.x(),
      p4.y(),
      p4.z(),
    };

    float colors[] = {
      color.x(), color.y(), color.z(), color.w(), color.x(), color.y(),
      color.z(), color.w(), color.x(), color.y(), color.z(), color.w(),
      color.x(), color.y(), color.z(), color.w(), color.x(), color.y(),
      color.z(), color.w(), color.x(), color.y(), color.z(), color.w(),
      color.x(), color.y(), color.z(), color.w(),
    };

    const int32_t elementCount = static_cast<int32_t>(std::size(vertices) / 3);

    // attribute
    auto vertexStreamData = std::make_shared<VertexStreamData>();

    {
      auto streamParam = std::make_shared<BufferAttributeStream<float>>(
          Name("POSITION"),
          EBufferType::Static,
          sizeof(float) * 3,
          std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
              EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
          std::vector<float>(std::begin(vertices), std::end(vertices)));
      vertexStreamData->streams.push_back(streamParam);
    }

    {
      auto streamParam = std::make_shared<BufferAttributeStream<float>>(
          Name("COLOR"),
          EBufferType::Static,
          sizeof(float) * 4,
          std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
              EBufferElementType::FLOAT, 0, sizeof(float) * 4)},
          GenerateColor(color, elementCount)  // Assuming this function returns
                                              // std::vector<float>
      );
      vertexStreamData->streams.push_back(streamParam);
    }

    {
      std::vector<float> normals(elementCount * 3);
      for (int32_t i = 0; i < elementCount; ++i) {
        normals[i * 3]     = 0.0f;
        normals[i * 3 + 1] = 1.0f;
        normals[i * 3 + 2] = 0.0f;
      }

      auto streamParam = std::make_shared<BufferAttributeStream<float>>(
          Name("NORMAL"),
          EBufferType::Static,
          sizeof(float) * 3,
          std::vector<IBufferAttribute::Attribute>{IBufferAttribute::Attribute(
              EBufferElementType::FLOAT, 0, sizeof(float) * 3)},
          normals);
      vertexStreamData->streams.push_back(streamParam);
    }

    vertexStreamData->PrimitiveType = EPrimitiveType::TRIANGLES;
    vertexStreamData->elementCount  = elementCount;

    quad->RenderObjectGeometryDataPtr->UpdateVertexStream(vertexStreamData);
  };

  updateQuadFunc(Plane[0],
                 far_lt,
                 near_lt,
                 far_lb,
                 near_lb,
                 math::Vector4Df(0.0f, 1.0f, 0.0f, 0.3f));
  updateQuadFunc(Plane[1],
                 near_rt,
                 far_rt,
                 near_rb,
                 far_rb,
                 math::Vector4Df(0.0f, 0.0f, 1.0f, 0.3f));
  updateQuadFunc(Plane[2],
                 far_lt,
                 far_rt,
                 near_lt,
                 near_rt,
                 math::Vector4Df(1.0f, 1.0f, 0.0f, 0.3f));
  updateQuadFunc(Plane[3],
                 near_lb,
                 near_rb,
                 far_lb,
                 far_rb,
                 math::Vector4Df(1.0f, 0.0f, 0.0f, 0.3f));
  updateQuadFunc(Plane[4],
                 near_lt,
                 near_rt,
                 near_lb,
                 near_rb,
                 math::Vector4Df(1.0f, 1.0f, 1.0f, 0.3f));
  updateQuadFunc(Plane[5],
                 far_lt,
                 far_rt,
                 far_lb,
                 far_rb,
                 math::Vector4Df(1.0f, 1.0f, 1.0f, 0.3f));

  for (int32_t i = 0; i < 16; ++i) {
    Segments[i]->RenderObjects[0]->SetPos(Offset);
    Segments[i]->RenderObjects[0]->SetScale(Scale);
    Segments[i]->Update(deltaTime);
  }

  for (int32_t i = 0; i < 6; ++i) {
    Plane[i]->RenderObjects[0]->SetPos(Offset);
    Plane[i]->RenderObjects[0]->SetScale(Scale);
    Plane[i]->Update(deltaTime);
  }
}

void Graph2D::Update(float deltaTime) {
  UpdateBuffer();
}

void Graph2D::SethPos(const math::Vector2Df& pos) {
  if (Pos != pos) {
    DirtyFlag = true;
    Pos       = pos;
  }
}

void Graph2D::SetPoints(const std::vector<math::Vector2Df>& points) {
  Points    = points;
  DirtyFlag = true;
}

void Graph2D::UpdateBuffer() {
  if (DirtyFlag) {
    DirtyFlag = false;

    ResultPoints.resize(Points.size());
    for (int32_t i = 0; i < ResultPoints.size(); ++i) {
      ResultPoints[i] = math::Vector2Df(Points[i].x(), -Points[i].y())
                      + math::Vector2Df(Pos.x(), Pos.y());
    }

    if (ResultPoints.size() > 1) {
      ResultMatrices.resize(ResultPoints.size() - 1 + 2);

      {
        auto hor = math::Matrix4f::Identity();
        hor.setBasisX(math::g_rightVector<float, 4>());
        hor.setBasisY(math::g_upVector<float, 4>());
        hor.setBasisZ(math::g_forwardVector<float, 4>());
        math::g_setTranslate(hor, Pos.x(), Pos.y(), 0.0f);
        hor = (hor * math::g_scale(GuardLineSize.x(), 1.0f, 1.0f));
        ResultMatrices[0] = (hor);

        auto ver = math::Matrix4f::Identity();
        ver.setBasisX(math::g_rightVector<float, 4>());
        ver.setBasisY(math::g_upVector<float, 4>());
        ver.setBasisZ(math::g_forwardVector<float, 4>());
        math::g_setTranslate(ver, Pos.x(), Pos.y(), 0.0f);
        ver = (ver * math::g_scale(1.0f, -GuardLineSize.y(), 1.0f));
        ResultMatrices[1] = (ver);
      }

      for (int i = 2; i < ResultMatrices.size(); ++i) {
        const auto& p2         = ResultPoints[i + 1];
        const auto& p1         = ResultPoints[i];
        auto        right      = math::Vector3Df(p2 - p1, 0.0f);
        const float lineLength = right.magnitude();
        right = right.normalize();  // here in-place normalize method would be
                                    // better
        right.z() = 0.0f;

        auto up = right.cross(math::g_forwardVector<float, 3>()).normalize();

        math::Matrix4f& tr = ResultMatrices[i];
        tr                 = math::Matrix4f::Identity();
        tr.setBasisX(math::Vector4Df(right, 0.0f));
        tr.setBasisY(math::Vector4Df(up, 0.0f));
        tr.setBasisZ(math::g_forwardVector<float, 4>());
        math::g_setTranslate(tr, p1.x(), p1.y(), 0.0f);
        tr = (tr * math::g_scale(lineLength, 1.0f, 1.0f));
      }

      if (RenderObjectGeometryDataPtr->VertexStreamPtr) {
        static Name TransformName("Transform");
        for (auto& iter :
             RenderObjectGeometryDataPtr->VertexStreamPtr->streams) {
          if (iter->name == TransformName) {
            auto matStreamParam
                = static_cast<BufferAttributeStream<math::Matrix4f>*>(
                    iter.get());
            matStreamParam->Data.resize(ResultMatrices.size());
            memcpy(&matStreamParam->Data[0],
                   &ResultMatrices[0],
                   ResultMatrices.size() * math::Matrix4f::GetDataSize());
            break;
          }
        }
      }
    } else {
      ResultMatrices.clear();
    }
  }
}

}  // namespace game_engine