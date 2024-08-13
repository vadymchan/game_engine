
#include "game.h"

#include "gfx/rhi/rhi.h"

namespace game_engine {

void Game::Setup() {
  // TODO: consider change to std::mt19937
  srand(static_cast<uint32_t>(time(NULL)));

  // Create main camera
  // const math::Vector3Df mainCameraPos(-111.6f, 17.49f, 3.11f);
  // const math::Vector3Df mainCameraTarget(282.378632f, 17.6663227f,
  // -1.00448179f);
  const math::Vector3Df mainCameraPos(0.0f, 0.0f, 0.0f);
  const math::Vector3Df mainCameraTarget(0.0f, 0.0f, 1.0f);
  MainCamera
      = Camera::CreateCamera(mainCameraPos,
                             mainCameraTarget,
                             mainCameraPos + math::Vector3Df(0.0, 1.0, 0.0),
                             math::g_degreeToRadian(45.0f),
                             1.0f,
                             100.0f,
                             (float)m_window_->getSize().width(),
                             (float)m_window_->getSize().height(),
                             true);
  Camera::AddCamera(0, MainCamera);

  // auto cube = CreateCube(math::Vector3Df(0.0f, 60.0f, 5.0f),
  // math::g_oneVector<float, 3>(), math::g_oneVector<float, 3>() * 10.f,
  // math::Vector4Df(0.7f, 0.7f, 0.7f, 1.0f)); Object::AddObject(cube);
  // SpawnedObjects.push_back(cube);

  // Select spawning object type
  SpawnObjects(ESpawnedType::TestPrimitive);

  // SpawnObjects(ESpawnedType::InstancingPrimitive);
  // SpawnObjects(ESpawnedType::IndirectDrawPrimitive);

  // ResourceLoadCompleteEvent = std::async(std::launch::async, [&]()
  //{

  //{
  //	ScopedLock s(&AsyncLoadLock);
  //	CompletedAsyncLoadObjects.push_back(Sponza);
  //}
  //});

  g_rhi->Finish();  // todo : Instead of this, it needs UAV barrier here
}

void Game::SpawnObjects(ESpawnedType spawnType) {
  if (spawnType != SpawnedType) {
    SpawnedType = spawnType;
    switch (SpawnedType) {
      case ESpawnedType::TestPrimitive:
        SpawnTestPrimitives();
        break;
      case ESpawnedType::CubePrimitive:
        SapwnCubePrimitives();
        break;
      case ESpawnedType::InstancingPrimitive:
        SpawnInstancingPrimitives();
        break;
      case ESpawnedType::IndirectDrawPrimitive:
        SpawnIndirectDrawPrimitives();
        break;
    }
  }
}

void Game::RemoveSpawnedObjects() {
  for (auto& iter : SpawnedObjects) {
    assert(iter);
    Object::RemoveObject(iter);
    delete iter;
  }
  SpawnedObjects.clear();
}

void Game::SpawnTestPrimitives() {
  RemoveSpawnedObjects();

  auto triangle            = CreateTriangle(math::Vector3Df(0.0, 0.0, 5.0),
                                 math::g_oneVector<float, 3>(),
                                 math::Vector3Df(1.0, 1.0, 1.0),
                                 math::Vector4Df(1.0f, 1.0f, 1.0f, 1.0f));
  triangle->PostUpdateFunc = [](Object* thisObject, float deltaTime) {
    //thisObject->RenderObjects[0]->SetRot(thisObject->RenderObjects[0]->GetRot()
    //                                     + math::Vector3Df(5.0f, 0.0f, 0.0f)
    //                                           * deltaTime);
  };
  Object::AddObject(triangle);
  SpawnedObjects.push_back(triangle);

  //auto quad = CreateQuad(math::Vector3Df(1.0f, 1.0f, 1.0f),
  //                       math::Vector3Df(1.0f),
  //                       math::Vector3Df(1000.0f, 1000.0f, 1000.0f),
  //                       math::Vector4Df(1.0f, 1.0f, 1.0f, 1.0f));
  //quad->SetPlane(math::Plane(math::Vector3Df(0.0, 1.0, 0.0), -0.1f));
  //quad->SkipUpdateShadowVolume = true;
  //Object::AddObject(quad);
  //SpawnedObjects.push_back(quad);

  //auto gizmo              = CreateGizmo(math::g_zeroVector<float, 3>(),
  //                         math::g_zeroVector<float, 3>(),
  //                         math::g_oneVector<float, 3>());
  //gizmo->SkipShadowMapGen = true;
  //Object::AddObject(gizmo);
  //SpawnedObjects.push_back(gizmo);

  //auto triangle            = CreateTriangle(math::Vector3Df(60.0, 100.0, 20.0),
  //                               math::g_oneVector<float, 3>(),
  //                               math::Vector3Df(40.0, 40.0, 40.0),
  //                               math::Vector4Df(0.5f, 0.1f, 1.0f, 1.0f));
  //triangle->PostUpdateFunc = [](Object* thisObject, float deltaTime) {
  //  thisObject->RenderObjects[0]->SetRot(thisObject->RenderObjects[0]->GetRot()
  //                                       + math::Vector3Df(5.0f, 0.0f, 0.0f)
  //                                             * deltaTime);
  //};
  //Object::AddObject(triangle);
  //SpawnedObjects.push_back(triangle);

  //auto cube            = CreateCube(math::Vector3Df(-60.0f, 55.0f, -20.0f),
  //                       math::g_oneVector<float, 3>(),
  //                       math::Vector3Df(50.0f, 50.0f, 50.0f),
  //                       math::Vector4Df(0.7f, 0.7f, 0.7f, 1.0f));
  //cube->PostUpdateFunc = [](Object* thisObject, float deltaTime) {
  //  thisObject->RenderObjects[0]->SetRot(thisObject->RenderObjects[0]->GetRot()
  //                                       + math::Vector3Df(0.0f, 0.0f, 0.5f)
  //                                             * deltaTime);
  //};
  //Object::AddObject(cube);
  //SpawnedObjects.push_back(cube);

  //auto cube2 = CreateCube(math::Vector3Df(-65.0f, 35.0f, 10.0f),
  //                        math::g_oneVector<float, 3>(),
  //                        math::Vector3Df(50.0f, 50.0f, 50.0f),
  //                        math::Vector4Df(0.7f, 0.7f, 0.7f, 1.0f));
  //Object::AddObject(cube2);
  //SpawnedObjects.push_back(cube2);

  //auto capsule            = CreateCapsule(math::Vector3Df(30.0f, 30.0f, -80.0f),
  //                             40.0f,
  //                             10.0f,
  //                             20,
  //                             math::Vector3Df(1.0f),
  //                             math::Vector4Df(1.0f, 1.0f, 0.0f, 1.0f));
  //capsule->PostUpdateFunc = [](Object* thisObject, float deltaTime) {
  //  thisObject->RenderObjects[0]->SetRot(thisObject->RenderObjects[0]->GetRot()
  //                                       + math::Vector3Df(-1.0f, 0.0f, 0.0f)
  //                                             * deltaTime);
  //};
  //Object::AddObject(capsule);
  //SpawnedObjects.push_back(capsule);

  //auto cone            = CreateCone(math::Vector3Df(0.0f, 50.0f, 60.0f),
  //                       40.0f,
  //                       20.0f,
  //                       15,
  //                       math::g_oneVector<float, 3>(),
  //                       math::Vector4Df(1.0f, 1.0f, 0.0f, 1.0f));
  //cone->PostUpdateFunc = [](Object* thisObject, float deltaTime) {
  //  thisObject->RenderObjects[0]->SetRot(thisObject->RenderObjects[0]->GetRot()
  //                                       + math::Vector3Df(0.0f, 3.0f, 0.0f)
  //                                             * deltaTime);
  //};
  //Object::AddObject(cone);
  //SpawnedObjects.push_back(cone);

  //auto cylinder = CreateCylinder(math::Vector3Df(-30.0f, 60.0f, -60.0f),
  //                               20.0f,
  //                               10.0f,
  //                               20,
  //                               math::g_oneVector<float, 3>(),
  //                               math::Vector4Df(0.0f, 0.0f, 1.0f, 1.0f));
  //cylinder->PostUpdateFunc = [](Object* thisObject, float deltaTime) {
  //  thisObject->RenderObjects[0]->SetRot(thisObject->RenderObjects[0]->GetRot()
  //                                       + math::Vector3Df(5.0f, 0.0f, 0.0f)
  //                                             * deltaTime);
  //};
  //Object::AddObject(cylinder);
  //SpawnedObjects.push_back(cylinder);

  //auto quad2            = CreateQuad(math::Vector3Df(-20.0f, 80.0f, 40.0f),
  //                        math::g_oneVector<float, 3>(),
  //                        math::Vector3Df(20.0f, 20.0f, 20.0f),
  //                        math::Vector4Df(0.0f, 0.0f, 1.0f, 1.0f));
  //quad2->PostUpdateFunc = [](Object* thisObject, float deltaTime) {
  //  thisObject->RenderObjects[0]->SetRot(thisObject->RenderObjects[0]->GetRot()
  //                                       + math::Vector3Df(0.0f, 0.0f, 8.0f)
  //                                             * deltaTime);
  //};
  //Object::AddObject(quad2);
  //SpawnedObjects.push_back(quad2);

  //auto sphere            = CreateSphere(math::Vector3Df(65.0f, 35.0f, 10.0f),
  //                           1.0,
  //                           150,
  //                           75,
  //                           math::Vector3Df(30.0f),
  //                           math::Vector4Df(0.8f, 0.0f, 0.0f, 1.0f));
  //sphere->PostUpdateFunc = [](Object* thisObject, float deltaTime) {
  //  float RotationSpeed = 100.0f;
  //  thisObject->RenderObjects[0]->SetRot(
  //      thisObject->RenderObjects[0]->GetRot()
  //      + math::Vector3Df(0.0f, 0.0f, math::g_degreeToRadian(180.0f))
  //            * RotationSpeed * deltaTime);
  //};
  //Object::AddObject(sphere);
  //SpawnedObjects.push_back(sphere);

  //auto sphere2            = CreateSphere(math::Vector3Df(150.0f, 5.0f, 0.0f),
  //                            1.0,
  //                            150,
  //                            75,
  //                            math::Vector3Df(10.0f),
  //                            math::Vector4Df(0.8f, 0.4f, 0.6f, 1.0f));
  //sphere2->PostUpdateFunc = [](Object* thisObject, float deltaTime) {
  //  const float startY  = 5.0f;
  //  const float endY    = 100;
  //  const float speed   = 150.0f * deltaTime;
  //  static bool dir     = true;
  //  auto        Pos     = thisObject->RenderObjects[0]->GetPos();
  //  Pos.y()            += dir ? speed : -speed;
  //  if (Pos.y() < startY || Pos.y() > endY) {
  //    dir      = !dir;
  //    Pos.y() += dir ? speed : -speed;
  //  }
  //  thisObject->RenderObjects[0]->SetPos(Pos);
  //};
  //Object::AddObject(sphere2);
  //SpawnedObjects.push_back(sphere2);

  //auto billboard = CreateBillobardQuad(math::Vector3Df(0.0f, 60.0f, 80.0f),
  //                                     math::g_oneVector<float, 3>(),
  //                                     math::Vector3Df(20.0f, 20.0f, 20.0f),
  //                                     math::Vector4Df(1.0f, 0.0f, 1.0f, 1.0f),
  //                                     MainCamera);
  //Object::AddObject(billboard);
  //SpawnedObjects.push_back(billboard);

  // const float Size = 20.0f;

  // for (int32_t i = 0; i < 10; ++i)
  //{
  //	for (int32_t j = 0; j < 10; ++j)
  //	{
  //		for (int32_t k = 0; k < 5; ++k)
  //		{
  //			auto cube = CreateCube(math::Vector3Df(i * 25.0f,
  // k * 25.0f, j * 25.0f), math::g_oneVector<float, 3>(),
  // math::Vector3Df(Size), math::Vector4Df(0.7f, 0.7f, 0.7f, 1.0f));
  // Object::AddObject(cube);
  //			SpawnedObjects.push_back(cube);
  //		}
  //	}
  // }
}

// difference between perspective and orthographic projection
void Game::SpawnGraphTestFunc() {
  math::Vector3Df PerspectiveVector[90];
  math::Vector3Df OrthographicVector[90];
  {
    {
      static Camera* pCamera
          = Camera::CreateCamera(math::Vector3Df(0.0),
                                 math::Vector3Df(0.0, 0.0, 1.0),
                                 math::Vector3Df(0.0, 1.0, 0.0),
                                 math::g_degreeToRadian(90.0f),
                                 10.0,
                                 100.0,
                                 100.0,
                                 100.0,
                                 true);
      pCamera->UpdateCamera();
      int  cnt = 0;
      auto MV  = pCamera->Projection * pCamera->view;
      for (int i = 0; i < 90; ++i) {
        PerspectiveVector[cnt++] = math::g_transformPoint(
            math::Vector3Df({0.0f, 0.0f, 10.0f + static_cast<float>(i)}), MV);
      }

      for (int i = 0; i < std::size(PerspectiveVector); ++i) {
        PerspectiveVector[i].z() = (PerspectiveVector[i].z() + 1.0f) * 0.5f;
      }
    }
    {
      static Camera* pCamera
          = Camera::CreateCamera(math::Vector3Df(0.0),
                                 math::Vector3Df(0.0, 0.0, 1.0),
                                 math::Vector3Df(0.0, 1.0, 0.0),
                                 math::g_degreeToRadian(90.0f),
                                 10.0,
                                 100.0,
                                 100.0,
                                 100.0,
                                 false);
      pCamera->UpdateCamera();
      int  cnt = 0;
      auto MV  = pCamera->Projection * pCamera->view;
      for (int i = 0; i < 90; ++i) {
        OrthographicVector[cnt++] = math::g_transformPoint(
            math::Vector3Df({0.0f, 0.0f, 10.0f + static_cast<float>(i)}), MV);
      }

      for (int i = 0; i < std::size(OrthographicVector); ++i) {
        OrthographicVector[i].z() = (OrthographicVector[i].z() + 1.0f) * 0.5f;
      }
    }
  }
  std::vector<math::Vector2Df> graph1;
  std::vector<math::Vector2Df> graph2;

  float scale = 100.0f;
  for (int i = 0; i < std::size(PerspectiveVector); ++i) {
    graph1.push_back(math::Vector2Df(static_cast<float>(i * 2),
                                     PerspectiveVector[i].z() * scale));
  }
  for (int i = 0; i < std::size(OrthographicVector); ++i) {
    graph2.push_back(math::Vector2Df(static_cast<float>(i * 2),
                                     OrthographicVector[i].z() * scale));
  }

  auto graphObj1 = CreateGraph2D({360, 350}, {360, 300}, graph1);
  Object::AddUIDebugObject(graphObj1);

  auto graphObj2 = CreateGraph2D({360, 700}, {360, 300}, graph2);
  Object::AddUIDebugObject(graphObj2);
}

void Game::SapwnCubePrimitives() {
  RemoveSpawnedObjects();

  for (int i = 0; i < 20; ++i) {
    float height = 5.0f * i;
    auto  cube
        = CreateCube(math::Vector3Df(-500.0f + i * 50.0f, height / 2.0f, 20.0f),
                     math::g_oneVector<float, 3>(),
                     math::Vector3Df(10.0f, height, 20.0f),
                     math::Vector4Df(0.7f, 0.7f, 0.7f, 1.0f));
    Object::AddObject(cube);
    SpawnedObjects.push_back(cube);
    cube = CreateCube(
        math::Vector3Df(-500.0f + i * 50.0f, height / 2.0f, 20.0f + i * 20.0f),
        math::g_oneVector<float, 3>(),
        math::Vector3Df(10.0f, height, 10.0f),
        math::Vector4Df(0.7f, 0.7f, 0.7f, 1.0f));
    Object::AddObject(cube);
    SpawnedObjects.push_back(cube);
    cube = CreateCube(
        math::Vector3Df(-500.0f + i * 50.0f, height / 2.0f, 20.0f - i * 20.0f),
        math::g_oneVector<float, 3>(),
        math::Vector3Df(20.0f, height, 10.0f),
        math::Vector4Df(0.7f, 0.7f, 0.7f, 1.0f));
    Object::AddObject(cube);
    SpawnedObjects.push_back(cube);
  }

  auto quad = CreateQuad(math::Vector3Df(1.0f, 1.0f, 1.0f),
                         math::Vector3Df(1.0f),
                         math::Vector3Df(1000.0f, 1000.0f, 1000.0f),
                         math::Vector4Df(1.0f, 1.0f, 1.0f, 1.0f));
  quad->SetPlane(math::Plane(math::Vector3Df(0.0, 1.0, 0.0), -0.1f));
  Object::AddObject(quad);
  SpawnedObjects.push_back(quad);
}

void Game::SpawnInstancingPrimitives() {
  struct InstanceData {
    math::Vector4Df Color;
    math::Vector3Df W;
  };

  const int    numInstances = 100;
  InstanceData instanceData[numInstances];

  const float     colorStep = 1.0f / (float)sqrt(std::size(instanceData));
  math::Vector4Df curStep
      = math::Vector4Df(colorStep, colorStep, colorStep, 1.0f);

  for (int32_t i = 0; i < std::size(instanceData); ++i) {
    float x               = (float)(i / 10);
    float y               = (float)(i % 10);
    instanceData[i].W     = math::Vector3Df(y * 10.0f, x * 10.0f, 0.0f);
    instanceData[i].Color = curStep;
    if (i < std::size(instanceData) / 3) {
      curStep.x() += colorStep;
    } else if (i < std::size(instanceData) / 2) {
      curStep.y() += colorStep;
    } else if (i < std::size(instanceData)) {
      curStep.z() += colorStep;
    }
  }

  {
    auto obj         = CreateTriangle(math::Vector3Df(0.0f, 0.0f, 0.0f),
                              math::g_oneVector<float, 3>() * 8.0f,
                              math::g_oneVector<float, 3>(),
                              math::Vector4Df(1.0f, 0.0f, 0.0f, 1.0f));
    auto streamParam = std::make_shared<BufferAttributeStream<InstanceData>>(
        Name("InstanceData"),
        EBufferType::Static,
        sizeof(InstanceData),
        std::vector<IBufferAttribute::Attribute>{
          IBufferAttribute::Attribute(
              EBufferElementType::FLOAT, 0, math::Vector4Df::GetDataSize()),
          IBufferAttribute::Attribute(EBufferElementType::FLOAT,
                                      math::Vector4Df::GetDataSize(),
                                      math::Vector3Df::GetDataSize())},
        std::vector<InstanceData>(instanceData, instanceData + numInstances));

    // TODO: remove
    // auto streamParam =
    // std::make_shared<BufferAttributeStream<InstanceData>>();
    // streamParam->BufferType = EBufferType::Static;
    // streamParam->Attributes.push_back(IBufferAttribute::Attribute(
    //    EBufferElementType::FLOAT, 0, math::Vector4Df::GetDataSize()));
    // streamParam->Attributes.push_back(IBufferAttribute::Attribute(
    //    EBufferElementType::FLOAT, math::Vector3Df::GetDataSize()));
    // streamParam->Stride = sizeof(InstanceData);
    // streamParam->name   = Name("InstanceData");
    // streamParam->Data.resize(numInstances);
    // memcpy(&streamParam->Data[0], instanceData, sizeof(instanceData));

    auto& GeometryDataPtr = obj->RenderObjects[0]->GeometryDataPtr;

    GeometryDataPtr->VertexStream_InstanceDataPtr
        = std::make_shared<VertexStreamData>();
    GeometryDataPtr->VertexStream_InstanceDataPtr->elementCount
        = std::size(instanceData);
    GeometryDataPtr->VertexStream_InstanceDataPtr->startLocation
        = (int32_t)GeometryDataPtr->VertexStreamPtr->GetEndLocation();
    GeometryDataPtr->VertexStream_InstanceDataPtr->bindingIndex
        = (int32_t)GeometryDataPtr->VertexStreamPtr->streams.size();
    GeometryDataPtr->VertexStream_InstanceDataPtr->VertexInputRate
        = EVertexInputRate::INSTANCE;
    GeometryDataPtr->VertexStream_InstanceDataPtr->streams.push_back(
        streamParam);
    GeometryDataPtr->VertexBuffer_InstanceDataPtr
        = g_rhi->CreateVertexBuffer(
            GeometryDataPtr->VertexStream_InstanceDataPtr);

    Object::AddObject(obj);
    SpawnedObjects.push_back(obj);
  }
}

void Game::SpawnIndirectDrawPrimitives() {
  struct InstanceData {
    math::Vector4Df Color;
    math::Vector3Df W;
  };

  const int    numInstances = 100;
  InstanceData instanceData[numInstances];

  const float     colorStep = 1.0f / (float)sqrt(std::size(instanceData));
  math::Vector4Df curStep
      = math::Vector4Df(colorStep, colorStep, colorStep, 1.0f);

  for (int32_t i = 0; i < std::size(instanceData); ++i) {
    float x               = (float)(i / 10);
    float y               = (float)(i % 10);
    instanceData[i].W     = math::Vector3Df(y * 10.0f, x * 10.0f, 0.0f);
    instanceData[i].Color = curStep;
    if (i < std::size(instanceData) / 3) {
      curStep.x() += colorStep;
    } else if (i < std::size(instanceData) / 2) {
      curStep.y() += colorStep;
    } else if (i < std::size(instanceData)) {
      curStep.z() += colorStep;
    }
  }

  {
    auto obj = CreateTriangle(math::Vector3Df(0.0f, 0.0f, 0.0f),
                              math::g_oneVector<float, 3>() * 8.0f,
                              math::g_oneVector<float, 3>(),
                              math::Vector4Df(1.0f, 0.0f, 0.0f, 1.0f));

    auto streamParam = std::make_shared<BufferAttributeStream<InstanceData>>(
        Name("InstanceData"),
        EBufferType::Static,
        sizeof(InstanceData),
        std::vector<IBufferAttribute::Attribute>{
          IBufferAttribute::Attribute(
              EBufferElementType::FLOAT, 0, math::Vector4Df::GetDataSize()),
          IBufferAttribute::Attribute(EBufferElementType::FLOAT,
                                      math::Vector4Df::GetDataSize(),
                                      math::Vector3Df::GetDataSize())},
        std::vector<InstanceData>(instanceData, instanceData + numInstances));

    // TODO: remove
    // auto streamParam =
    // std::make_shared<BufferAttributeStream<InstanceData>>();
    // streamParam->BufferType = EBufferType::Static;
    // streamParam->Attributes.push_back(IBufferAttribute::Attribute(
    //    EBufferElementType::FLOAT, math::Vector4Df::GetDataSize()));
    // streamParam->Attributes.push_back(IBufferAttribute::Attribute(
    //    EBufferElementType::FLOAT, math::Vector3Df::GetDataSize()));
    // streamParam->Stride = sizeof(InstanceData);
    // streamParam->name   = Name("InstanceData");
    // streamParam->Data.resize(numInstances);
    // memcpy(&streamParam->Data[0], instanceData, sizeof(instanceData));

    auto& GeometryDataPtr = obj->RenderObjects[0]->GeometryDataPtr;
    GeometryDataPtr->VertexStream_InstanceDataPtr
        = std::make_shared<VertexStreamData>();
    GeometryDataPtr->VertexStream_InstanceDataPtr->elementCount
        = std::size(instanceData);
    GeometryDataPtr->VertexStream_InstanceDataPtr->startLocation
        = (int32_t)GeometryDataPtr->VertexStreamPtr->GetEndLocation();
    GeometryDataPtr->VertexStream_InstanceDataPtr->bindingIndex
        = (int32_t)GeometryDataPtr->VertexStreamPtr->streams.size();
    GeometryDataPtr->VertexStream_InstanceDataPtr->VertexInputRate
        = EVertexInputRate::INSTANCE;
    GeometryDataPtr->VertexStream_InstanceDataPtr->streams.push_back(
        streamParam);
    GeometryDataPtr->VertexBuffer_InstanceDataPtr
        = g_rhi->CreateVertexBuffer(
            GeometryDataPtr->VertexStream_InstanceDataPtr);

    // Create indirect draw buffer
    // TODO: Refactor this code to decouple it from Vulkan-specific classes and
    // functions (e.g., VkDrawIndirectCommand, g_rhi->CreateVertexBuffer).
    // Consider using an abstract rendering interface to handle draw calls and
    // resource management.
    {
      assert(GeometryDataPtr->VertexStream_InstanceDataPtr);

      std::vector<VkDrawIndirectCommand> indrectCommands;

      const int32_t instanceCount
          = GeometryDataPtr->VertexStream_InstanceDataPtr->elementCount;
      const int32_t vertexCount
          = GeometryDataPtr->VertexStreamPtr->elementCount;
      for (int32_t i = 0; i < instanceCount; ++i) {
        VkDrawIndirectCommand command;
        command.vertexCount   = vertexCount;
        command.instanceCount = 1;
        command.firstVertex   = 0;
        command.firstInstance = i;
        indrectCommands.emplace_back(command);
      }

      const size_t bufferSize
          = indrectCommands.size() * sizeof(VkDrawIndirectCommand);

      assert(!GeometryDataPtr->IndirectCommandBufferPtr);
      GeometryDataPtr->IndirectCommandBufferPtr
          = g_rhi->CreateStructuredBuffer(bufferSize,
                                             0,
                                             sizeof(VkDrawIndirectCommand),
                                             EBufferCreateFlag::IndirectCommand,
                                             EResourceLayout::TRANSFER_DST,
                                             indrectCommands.data(),
                                             bufferSize);
    }

    Object::AddObject(obj);
    SpawnedObjects.push_back(obj);
  }
}

void Game::Update(float deltaTime) {
  // if (CompletedAsyncLoadObjects.size() > 0)
  //{
  //       ScopedLock s(&AsyncLoadLock);
  //	for (auto iter : CompletedAsyncLoadObjects)
  //	{
  //           Object::AddObject(iter);
  //           SpawnedObjects.push_back(iter);
  //	}
  //	CompletedAsyncLoadObjects.clear();
  //}

  // Update application property by using UI Pannel.
  // UpdateAppSetting();

  // Update main camera
  if (MainCamera) {
    MainCamera->UpdateCamera();
  }

  // gOptions.CameraPos = MainCamera->Pos;

  // for (auto iter : Object::GetStaticObject())
  //	iter->Update(deltaTime);

  // for (auto& iter : Object::GetBoundBoxObject())
  //	iter->Update(deltaTime);

  // for (auto& iter : Object::GetBoundSphereObject())
  //	iter->Update(deltaTime);

  // for (auto& iter : Object::GetDebugObject())
  //	iter->Update(deltaTime);

  // Update object which have dirty flag
  Object::FlushDirtyState();

  //// Render all objects by using selected renderer
  // Renderer->Render(MainCamera);

  for (auto& iter : Object::GetStaticObject()) {
    iter->Update(deltaTime);

    for (auto& RenderObject : iter->RenderObjects) {
      RenderObject->UpdateWorldMatrix();
    }
  }

  for (auto& iter : Object::GetDebugObject()) {
    iter->Update(deltaTime);

    for (auto& RenderObject : iter->RenderObjects) {
      RenderObject->UpdateWorldMatrix();
    }
  }
}

void Game::Draw() {
  {
    std::shared_ptr<RenderFrameContext> renderFrameContext
        = g_rhi->BeginRenderFrame();
    if (!renderFrameContext) {
      // TODO: log error
      return;
    }

    View view(MainCamera);
    view.PrepareViewUniformBufferShaderBindingInstance();

    Renderer renderer(renderFrameContext, view, m_window_);
    renderer.Render();

    g_rhi->EndRenderFrame(renderFrameContext);
  }
  MemStack::Get()->Flush();
}

void Game::Resize(int32_t width, int32_t height) {
  if (MainCamera) {
    MainCamera->Width  = width;
    MainCamera->Height = height;
  }
}

void Game::Release() {
  g_rhi->Flush();

  for (Object* iter : SpawnedObjects) {
    delete iter;
  }
  SpawnedObjects.clear();

  delete MainCamera;
}

}  // namespace game_engine
