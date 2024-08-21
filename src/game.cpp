
#include "game.h"

#include "gfx/rhi/rhi.h"

namespace game_engine {

void Game::setup() {
  // TODO: consider change to std::mt19937
  srand(static_cast<uint32_t>(time(NULL)));

  // Create main camera
  // const math::Vector3Df mainCameraPos(-111.6f, 17.49f, 3.11f);
  // const math::Vector3Df mainCameraTarget(282.378632f, 17.6663227f,
  // -1.00448179f);
  const math::Vector3Df mainCameraPos(0.0f, 0.0f, 0.0f);
  const math::Vector3Df mainCameraTarget(0.0f, 0.0f, 1.0f);
  m_mainCamera_
      = Camera::s_createCamera(mainCameraPos,
                             mainCameraTarget,
                             mainCameraPos + math::Vector3Df(0.0, 1.0, 0.0),
                             math::g_degreeToRadian(45.0f),
                             1.0f,
                             100.0f,
                             (float)m_window_->getSize().width(),
                             (float)m_window_->getSize().height(),
                             true);
  Camera::s_addCamera(0, m_mainCamera_);

  // auto cube = g_createCube(math::Vector3Df(0.0f, 60.0f, 5.0f),
  // math::g_oneVector<float, 3>(), math::g_oneVector<float, 3>() * 10.f,
  // math::Vector4Df(0.7f, 0.7f, 0.7f, 1.0f)); Object::s_addObject(cube);
  // m_spawnedObjects_.push_back(cube);

  // Select spawning object type
  spawnObjects(ESpawnedType::TestPrimitive);

  // spawnObjects(ESpawnedType::InstancingPrimitive);
  // spawnObjects(ESpawnedType::IndirectDrawPrimitive);

  // m_resourceLoadCompleteEvent_ = std::async(std::launch::async, [&]()
  //{

  //{
  //	ScopedLock s(&m_asyncLoadLock_);
  //	m_completedAsyncLoadObjects_.push_back(Sponza);
  //}
  //});

  g_rhi->finish();  // todo : Instead of this, it needs UAV barrier here
}

void Game::spawnObjects(ESpawnedType spawnType) {
  if (spawnType != m_spawnedType_) {
    m_spawnedType_ = spawnType;
    switch (m_spawnedType_) {
      case ESpawnedType::TestPrimitive:
        spawnTestPrimitives();
        break;
      case ESpawnedType::CubePrimitive:
        spawnCubePrimitives();
        break;
      case ESpawnedType::InstancingPrimitive:
        spawnInstancingPrimitives();
        break;
      case ESpawnedType::IndirectDrawPrimitive:
        spawnIndirectDrawPrimitives();
        break;
    }
  }
}

void Game::removeSpawnedObjects() {
  for (auto& iter : m_spawnedObjects_) {
    assert(iter);
    Object::s_removeObject(iter);
    delete iter;
  }
  m_spawnedObjects_.clear();
}

void Game::spawnTestPrimitives() {
  removeSpawnedObjects();

  auto triangle            = g_createTriangle(math::Vector3Df(0.0, 0.0, 5.0),
                                 math::g_oneVector<float, 3>(),
                                 math::Vector3Df(1.0, 1.0, 1.0),
                                 math::Vector4Df(1.0f, 1.0f, 1.0f, 1.0f));
  triangle->m_postUpdateFunc_ = [](Object* thisObject, float deltaTime) {
    //thisObject->m_renderObjects_[0]->SetRot(thisObject->m_renderObjects_[0]->GetRot()
    //                                     + math::Vector3Df(5.0f, 0.0f, 0.0f)
    //                                           * deltaTime);
  };
  Object::s_addObject(triangle);
  m_spawnedObjects_.push_back(triangle);

  //auto quad = createQuad(math::Vector3Df(1.0f, 1.0f, 1.0f),
  //                       math::Vector3Df(1.0f),
  //                       math::Vector3Df(1000.0f, 1000.0f, 1000.0f),
  //                       math::Vector4Df(1.0f, 1.0f, 1.0f, 1.0f));
  //quad->setPlane(math::Plane(math::Vector3Df(0.0, 1.0, 0.0), -0.1f));
  //quad->m_skipUpdateShadowVolume_ = true;
  //Object::s_addObject(quad);
  //m_spawnedObjects_.push_back(quad);

  //auto gizmo              = g_createGizmo(math::g_zeroVector<float, 3>(),
  //                         math::g_zeroVector<float, 3>(),
  //                         math::g_oneVector<float, 3>());
  //gizmo->m_skipShadowMapGen_ = true;
  //Object::s_addObject(gizmo);
  //m_spawnedObjects_.push_back(gizmo);

  //auto triangle            = g_createTriangle(math::Vector3Df(60.0, 100.0, 20.0),
  //                               math::g_oneVector<float, 3>(),
  //                               math::Vector3Df(40.0, 40.0, 40.0),
  //                               math::Vector4Df(0.5f, 0.1f, 1.0f, 1.0f));
  //triangle->m_postUpdateFunc_ = [](Object* thisObject, float deltaTime) {
  //  thisObject->m_renderObjects_[0]->SetRot(thisObject->m_renderObjects_[0]->GetRot()
  //                                       + math::Vector3Df(5.0f, 0.0f, 0.0f)
  //                                             * deltaTime);
  //};
  //Object::s_addObject(triangle);
  //m_spawnedObjects_.push_back(triangle);

  //auto cube            = g_createCube(math::Vector3Df(-60.0f, 55.0f, -20.0f),
  //                       math::g_oneVector<float, 3>(),
  //                       math::Vector3Df(50.0f, 50.0f, 50.0f),
  //                       math::Vector4Df(0.7f, 0.7f, 0.7f, 1.0f));
  //cube->m_postUpdateFunc_ = [](Object* thisObject, float deltaTime) {
  //  thisObject->m_renderObjects_[0]->SetRot(thisObject->m_renderObjects_[0]->GetRot()
  //                                       + math::Vector3Df(0.0f, 0.0f, 0.5f)
  //                                             * deltaTime);
  //};
  //Object::s_addObject(cube);
  //m_spawnedObjects_.push_back(cube);

  //auto cube2 = g_createCube(math::Vector3Df(-65.0f, 35.0f, 10.0f),
  //                        math::g_oneVector<float, 3>(),
  //                        math::Vector3Df(50.0f, 50.0f, 50.0f),
  //                        math::Vector4Df(0.7f, 0.7f, 0.7f, 1.0f));
  //Object::s_addObject(cube2);
  //m_spawnedObjects_.push_back(cube2);

  //auto capsule            = g_createCapsule(math::Vector3Df(30.0f, 30.0f, -80.0f),
  //                             40.0f,
  //                             10.0f,
  //                             20,
  //                             math::Vector3Df(1.0f),
  //                             math::Vector4Df(1.0f, 1.0f, 0.0f, 1.0f));
  //capsule->m_postUpdateFunc_ = [](Object* thisObject, float deltaTime) {
  //  thisObject->m_renderObjects_[0]->SetRot(thisObject->m_renderObjects_[0]->GetRot()
  //                                       + math::Vector3Df(-1.0f, 0.0f, 0.0f)
  //                                             * deltaTime);
  //};
  //Object::s_addObject(capsule);
  //m_spawnedObjects_.push_back(capsule);

  //auto cone            = g_createCone(math::Vector3Df(0.0f, 50.0f, 60.0f),
  //                       40.0f,
  //                       20.0f,
  //                       15,
  //                       math::g_oneVector<float, 3>(),
  //                       math::Vector4Df(1.0f, 1.0f, 0.0f, 1.0f));
  //cone->m_postUpdateFunc_ = [](Object* thisObject, float deltaTime) {
  //  thisObject->m_renderObjects_[0]->SetRot(thisObject->m_renderObjects_[0]->GetRot()
  //                                       + math::Vector3Df(0.0f, 3.0f, 0.0f)
  //                                             * deltaTime);
  //};
  //Object::s_addObject(cone);
  //m_spawnedObjects_.push_back(cone);

  //auto cylinder = g_createCylinder(math::Vector3Df(-30.0f, 60.0f, -60.0f),
  //                               20.0f,
  //                               10.0f,
  //                               20,
  //                               math::g_oneVector<float, 3>(),
  //                               math::Vector4Df(0.0f, 0.0f, 1.0f, 1.0f));
  //cylinder->m_postUpdateFunc_ = [](Object* thisObject, float deltaTime) {
  //  thisObject->m_renderObjects_[0]->SetRot(thisObject->m_renderObjects_[0]->GetRot()
  //                                       + math::Vector3Df(5.0f, 0.0f, 0.0f)
  //                                             * deltaTime);
  //};
  //Object::s_addObject(cylinder);
  //m_spawnedObjects_.push_back(cylinder);

  //auto quad2            = createQuad(math::Vector3Df(-20.0f, 80.0f, 40.0f),
  //                        math::g_oneVector<float, 3>(),
  //                        math::Vector3Df(20.0f, 20.0f, 20.0f),
  //                        math::Vector4Df(0.0f, 0.0f, 1.0f, 1.0f));
  //quad2->m_postUpdateFunc_ = [](Object* thisObject, float deltaTime) {
  //  thisObject->m_renderObjects_[0]->SetRot(thisObject->m_renderObjects_[0]->GetRot()
  //                                       + math::Vector3Df(0.0f, 0.0f, 8.0f)
  //                                             * deltaTime);
  //};
  //Object::s_addObject(quad2);
  //m_spawnedObjects_.push_back(quad2);

  //auto sphere            = g_createSphere(math::Vector3Df(65.0f, 35.0f, 10.0f),
  //                           1.0,
  //                           150,
  //                           75,
  //                           math::Vector3Df(30.0f),
  //                           math::Vector4Df(0.8f, 0.0f, 0.0f, 1.0f));
  //sphere->m_postUpdateFunc_ = [](Object* thisObject, float deltaTime) {
  //  float RotationSpeed = 100.0f;
  //  thisObject->m_renderObjects_[0]->SetRot(
  //      thisObject->m_renderObjects_[0]->GetRot()
  //      + math::Vector3Df(0.0f, 0.0f, math::g_degreeToRadian(180.0f))
  //            * RotationSpeed * deltaTime);
  //};
  //Object::s_addObject(sphere);
  //m_spawnedObjects_.push_back(sphere);

  //auto sphere2            = g_createSphere(math::Vector3Df(150.0f, 5.0f, 0.0f),
  //                            1.0,
  //                            150,
  //                            75,
  //                            math::Vector3Df(10.0f),
  //                            math::Vector4Df(0.8f, 0.4f, 0.6f, 1.0f));
  //sphere2->m_postUpdateFunc_ = [](Object* thisObject, float deltaTime) {
  //  const float startY  = 5.0f;
  //  const float endY    = 100;
  //  const float speed   = 150.0f * deltaTime;
  //  static bool dir     = true;
  //  auto        m_position_     = thisObject->m_renderObjects_[0]->GetPos();
  //  m_position_.y()            += dir ? speed : -speed;
  //  if (m_position_.y() < startY || m_position_.y() > endY) {
  //    dir      = !dir;
  //    m_position_.y() += dir ? speed : -speed;
  //  }
  //  thisObject->m_renderObjects_[0]->setPosition(m_position_);
  //};
  //Object::s_addObject(sphere2);
  //m_spawnedObjects_.push_back(sphere2);

  //auto billboard = g_createBillobardQuad(math::Vector3Df(0.0f, 60.0f, 80.0f),
  //                                     math::g_oneVector<float, 3>(),
  //                                     math::Vector3Df(20.0f, 20.0f, 20.0f),
  //                                     math::Vector4Df(1.0f, 0.0f, 1.0f, 1.0f),
  //                                     m_mainCamera_);
  //Object::s_addObject(billboard);
  //m_spawnedObjects_.push_back(billboard);

  // const float Size = 20.0f;

  // for (int32_t i = 0; i < 10; ++i)
  //{
  //	for (int32_t j = 0; j < 10; ++j)
  //	{
  //		for (int32_t k = 0; k < 5; ++k)
  //		{
  //			auto cube = g_createCube(math::Vector3Df(i * 25.0f,
  // k * 25.0f, j * 25.0f), math::g_oneVector<float, 3>(),
  // math::Vector3Df(Size), math::Vector4Df(0.7f, 0.7f, 0.7f, 1.0f));
  // Object::s_addObject(cube);
  //			m_spawnedObjects_.push_back(cube);
  //		}
  //	}
  // }
}

// difference between perspective and orthographic projection
void Game::spawnGraphTestFunc() {
  math::Vector3Df PerspectiveVector[90];
  math::Vector3Df OrthographicVector[90];
  {
    {
      static Camera* pCamera
          = Camera::s_createCamera(math::Vector3Df(0.0),
                                 math::Vector3Df(0.0, 0.0, 1.0),
                                 math::Vector3Df(0.0, 1.0, 0.0),
                                 math::g_degreeToRadian(90.0f),
                                 10.0,
                                 100.0,
                                 100.0,
                                 100.0,
                                 true);
      pCamera->updateCamera();
      int  cnt = 0;
      auto MV  = pCamera->m_projection_ * pCamera->m_view_;
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
          = Camera::s_createCamera(math::Vector3Df(0.0),
                                 math::Vector3Df(0.0, 0.0, 1.0),
                                 math::Vector3Df(0.0, 1.0, 0.0),
                                 math::g_degreeToRadian(90.0f),
                                 10.0,
                                 100.0,
                                 100.0,
                                 100.0,
                                 false);
      pCamera->updateCamera();
      int  cnt = 0;
      auto MV  = pCamera->m_projection_ * pCamera->m_view_;
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

  auto graphObj1 = g_createGraph2D({360, 350}, {360, 300}, graph1);
  Object::s_addUIDebugObject(graphObj1);

  auto graphObj2 = g_createGraph2D({360, 700}, {360, 300}, graph2);
  Object::s_addUIDebugObject(graphObj2);
}

void Game::spawnCubePrimitives() {
  removeSpawnedObjects();

  for (int i = 0; i < 20; ++i) {
    float height = 5.0f * i;
    auto  cube
        = g_createCube(math::Vector3Df(-500.0f + i * 50.0f, height / 2.0f, 20.0f),
                     math::g_oneVector<float, 3>(),
                     math::Vector3Df(10.0f, height, 20.0f),
                     math::Vector4Df(0.7f, 0.7f, 0.7f, 1.0f));
    Object::s_addObject(cube);
    m_spawnedObjects_.push_back(cube);
    cube = g_createCube(
        math::Vector3Df(-500.0f + i * 50.0f, height / 2.0f, 20.0f + i * 20.0f),
        math::g_oneVector<float, 3>(),
        math::Vector3Df(10.0f, height, 10.0f),
        math::Vector4Df(0.7f, 0.7f, 0.7f, 1.0f));
    Object::s_addObject(cube);
    m_spawnedObjects_.push_back(cube);
    cube = g_createCube(
        math::Vector3Df(-500.0f + i * 50.0f, height / 2.0f, 20.0f - i * 20.0f),
        math::g_oneVector<float, 3>(),
        math::Vector3Df(20.0f, height, 10.0f),
        math::Vector4Df(0.7f, 0.7f, 0.7f, 1.0f));
    Object::s_addObject(cube);
    m_spawnedObjects_.push_back(cube);
  }

  auto quad = g_createQuad(math::Vector3Df(1.0f, 1.0f, 1.0f),
                         math::Vector3Df(1.0f),
                         math::Vector3Df(1000.0f, 1000.0f, 1000.0f),
                         math::Vector4Df(1.0f, 1.0f, 1.0f, 1.0f));
  quad->setPlane(math::Plane(math::Vector3Df(0.0, 1.0, 0.0), -0.1f));
  Object::s_addObject(quad);
  m_spawnedObjects_.push_back(quad);
}

void Game::spawnInstancingPrimitives() {
  struct InstanceData {
    math::Vector4Df m_color;
    math::Vector3Df m_w;
  };

  const int    numInstances = 100;
  InstanceData instanceData[numInstances];

  const float     colorStep = 1.0f / (float)sqrt(std::size(instanceData));
  math::Vector4Df curStep
      = math::Vector4Df(colorStep, colorStep, colorStep, 1.0f);

  for (int32_t i = 0; i < std::size(instanceData); ++i) {
    float x               = (float)(i / 10);
    float y               = (float)(i % 10);
    instanceData[i].m_w     = math::Vector3Df(y * 10.0f, x * 10.0f, 0.0f);
    instanceData[i].m_color = curStep;
    if (i < std::size(instanceData) / 3) {
      curStep.x() += colorStep;
    } else if (i < std::size(instanceData) / 2) {
      curStep.y() += colorStep;
    } else if (i < std::size(instanceData)) {
      curStep.z() += colorStep;
    }
  }

  {
    auto obj         = g_createTriangle(math::Vector3Df(0.0f, 0.0f, 0.0f),
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

    auto& GeometryDataPtr = obj->m_renderObjects_[0]->m_geometryDataPtr_;

    GeometryDataPtr->m_vertexStreamInstanceDataPtr_
        = std::make_shared<VertexStreamData>();
    GeometryDataPtr->m_vertexStreamInstanceDataPtr_->m_elementCount_
        = std::size(instanceData);
    GeometryDataPtr->m_vertexStreamInstanceDataPtr_->m_startLocation_
        = (int32_t)GeometryDataPtr->m_vertexStreamPtr_->getEndLocation();
    GeometryDataPtr->m_vertexStreamInstanceDataPtr_->m_bindingIndex_
        = (int32_t)GeometryDataPtr->m_vertexStreamPtr_->m_streams_.size();
    GeometryDataPtr->m_vertexStreamInstanceDataPtr_->m_vertexInputRate_
        = EVertexInputRate::INSTANCE;
    GeometryDataPtr->m_vertexStreamInstanceDataPtr_->m_streams_.push_back(
        streamParam);
    GeometryDataPtr->m_vertexBufferInstanceDataPtr_
        = g_rhi->createVertexBuffer(
            GeometryDataPtr->m_vertexStreamInstanceDataPtr_);

    Object::s_addObject(obj);
    m_spawnedObjects_.push_back(obj);
  }
}

void Game::spawnIndirectDrawPrimitives() {
  struct InstanceData {
    math::Vector4Df m_color;
    math::Vector3Df m_w;
  };

  const int    numInstances = 100;
  InstanceData instanceData[numInstances];

  const float     colorStep = 1.0f / (float)sqrt(std::size(instanceData));
  math::Vector4Df curStep
      = math::Vector4Df(colorStep, colorStep, colorStep, 1.0f);

  for (int32_t i = 0; i < std::size(instanceData); ++i) {
    float x               = (float)(i / 10);
    float y               = (float)(i % 10);
    instanceData[i].m_w     = math::Vector3Df(y * 10.0f, x * 10.0f, 0.0f);
    instanceData[i].m_color = curStep;
    if (i < std::size(instanceData) / 3) {
      curStep.x() += colorStep;
    } else if (i < std::size(instanceData) / 2) {
      curStep.y() += colorStep;
    } else if (i < std::size(instanceData)) {
      curStep.z() += colorStep;
    }
  }

  {
    auto obj = g_createTriangle(math::Vector3Df(0.0f, 0.0f, 0.0f),
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

    auto& GeometryDataPtr = obj->m_renderObjects_[0]->m_geometryDataPtr_;
    GeometryDataPtr->m_vertexStreamInstanceDataPtr_
        = std::make_shared<VertexStreamData>();
    GeometryDataPtr->m_vertexStreamInstanceDataPtr_->m_elementCount_
        = std::size(instanceData);
    GeometryDataPtr->m_vertexStreamInstanceDataPtr_->m_startLocation_
        = (int32_t)GeometryDataPtr->m_vertexStreamPtr_->getEndLocation();
    GeometryDataPtr->m_vertexStreamInstanceDataPtr_->m_bindingIndex_
        = (int32_t)GeometryDataPtr->m_vertexStreamPtr_->m_streams_.size();
    GeometryDataPtr->m_vertexStreamInstanceDataPtr_->m_vertexInputRate_
        = EVertexInputRate::INSTANCE;
    GeometryDataPtr->m_vertexStreamInstanceDataPtr_->m_streams_.push_back(
        streamParam);
    GeometryDataPtr->m_vertexBufferInstanceDataPtr_
        = g_rhi->createVertexBuffer(
            GeometryDataPtr->m_vertexStreamInstanceDataPtr_);

    // Create indirect draw buffer
    // TODO: Refactor this code to decouple it from Vulkan-specific classes and
    // functions (e.g., VkDrawIndirectCommand, g_rhi->createVertexBuffer).
    // Consider using an abstract rendering interface to handle draw calls and
    // resource management.
    {
      assert(GeometryDataPtr->m_vertexStreamInstanceDataPtr_);

      std::vector<VkDrawIndirectCommand> indrectCommands;

      const int32_t instanceCount
          = GeometryDataPtr->m_vertexStreamInstanceDataPtr_->m_elementCount_;
      const int32_t vertexCount
          = GeometryDataPtr->m_vertexStreamPtr_->m_elementCount_;
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

      assert(!GeometryDataPtr->m_indirectCommandBufferPtr_);
      GeometryDataPtr->m_indirectCommandBufferPtr_
          = g_rhi->createStructuredBuffer(bufferSize,
                                             0,
                                             sizeof(VkDrawIndirectCommand),
                                             EBufferCreateFlag::IndirectCommand,
                                             EResourceLayout::TRANSFER_DST,
                                             indrectCommands.data(),
                                             bufferSize);
    }

    Object::s_addObject(obj);
    m_spawnedObjects_.push_back(obj);
  }
}

void Game::update(float deltaTime) {
  // if (m_completedAsyncLoadObjects_.size() > 0)
  //{
  //       ScopedLock s(&m_asyncLoadLock_);
  //	for (auto iter : m_completedAsyncLoadObjects_)
  //	{
  //           Object::s_addObject(iter);
  //           m_spawnedObjects_.push_back(iter);
  //	}
  //	m_completedAsyncLoadObjects_.clear();
  //}

  // update application property by using UI Pannel.
  // UpdateAppSetting();

  // update main camera
  if (m_mainCamera_) {
    m_mainCamera_->updateCamera();
  }

  // gOptions.CameraPos = m_mainCamera_->m_position_;

  // for (auto iter : Object::s_getStaticObject())
  //	iter->update(deltaTime);

  // for (auto& iter : Object::s_getBoundBoxObject())
  //	iter->update(deltaTime);

  // for (auto& iter : Object::GetBoundSphereObject())
  //	iter->update(deltaTime);

  // for (auto& iter : Object::s_getDebugObject())
  //	iter->update(deltaTime);

  // update object which have dirty flag
  Object::s_flushDirtyState();

  //// render all objects by using selected renderer
  // Renderer->render(m_mainCamera_);

  for (auto& iter : Object::s_getStaticObject()) {
    iter->update(deltaTime);

    for (auto& RenderObject : iter->m_renderObjects_) {
      RenderObject->updateWorldMatrix();
    }
  }

  for (auto& iter : Object::s_getDebugObject()) {
    iter->update(deltaTime);

    for (auto& RenderObject : iter->m_renderObjects_) {
      RenderObject->updateWorldMatrix();
    }
  }
}

void Game::draw() {
  {
    std::shared_ptr<RenderFrameContext> renderFrameContext
        = g_rhi->beginRenderFrame();
    if (!renderFrameContext) {
      // TODO: log error
      return;
    }

    View view(m_mainCamera_);
    view.prepareViewUniformBufferShaderBindingInstance();

    Renderer renderer(renderFrameContext, view, m_window_);
    renderer.render();

    g_rhi->endRenderFrame(renderFrameContext);
  }
  MemStack::get()->flush();
}

void Game::resize(int32_t width, int32_t height) {
  if (m_mainCamera_) {
    m_mainCamera_->m_width_  = width;
    m_mainCamera_->m_height_ = height;
  }
}

void Game::release() {
  g_rhi->flush();

  for (Object* iter : m_spawnedObjects_) {
    delete iter;
  }
  m_spawnedObjects_.clear();

  delete m_mainCamera_;
}

}  // namespace game_engine
