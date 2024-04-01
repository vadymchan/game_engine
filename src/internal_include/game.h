#ifndef GAME_ENGINE_GAME_H
#define GAME_ENGINE_GAME_H

#include "gfx/renderer/primitive_util.h"
#include "gfx/renderer/renderer.h"
#include "gfx/scene/camera.h"
#include "gfx/scene/object.h"
#include "gfx/scene/view.h"

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iterator>
#include <vector>

namespace game_engine {

// TODO: consider renaming class (e.g. Application)
class Game {
  public:
  Game(std::shared_ptr<Window> window)
      : m_window_(std::move(window)) {}

  ~Game() {}

  void ProcessInput(/*float deltaTime*/) {
    // TODO: implement
    // static float MoveDistancePerSecond = 200.0f;
    //// static float MoveDistancePerSecond = 10.0f;
    // const float  CurrentDistance = MoveDistancePerSecond * deltaTime;

    //// Process Key Event
    // if (g_KeyState['a'] || g_KeyState['A']) {
    //   MainCamera->MoveShift(-CurrentDistance);
    // }
    // if (g_KeyState['d'] || g_KeyState['D']) {
    //   MainCamera->MoveShift(CurrentDistance);
    // }
    //// if (g_KeyState['1']) MainCamera->RotateForwardAxis(-0.1f);
    //// if (g_KeyState['2']) MainCamera->RotateForwardAxis(0.1f);
    //// if (g_KeyState['3']) MainCamera->RotateUpAxis(-0.1f);
    //// if (g_KeyState['4']) MainCamera->RotateUpAxis(0.1f);
    //// if (g_KeyState['5']) MainCamera->RotateRightAxis(-0.1f);
    //// if (g_KeyState['6']) MainCamera->RotateRightAxis(0.1f);
    // if (g_KeyState['w'] || g_KeyState['W']) {
    //   MainCamera->MoveForward(CurrentDistance);
    // }
    // if (g_KeyState['s'] || g_KeyState['S']) {
    //   MainCamera->MoveForward(-CurrentDistance);
    // }
    // if (g_KeyState['+']) {
    //   MoveDistancePerSecond = Max(MoveDistancePerSecond + 10.0f, 0.0f);
    // }
    // if (g_KeyState['-']) {
    //   MoveDistancePerSecond = Max(MoveDistancePerSecond - 10.0f, 0.0f);
    // }
  }

  void Setup();

  enum class ESpawnedType {
    None = 0,
    TestPrimitive,
    CubePrimitive,
    InstancingPrimitive,
    IndirectDrawPrimitive,
  };

  void SpawnObjects(ESpawnedType spawnType);

  void RemoveSpawnedObjects();

  void SpawnTestPrimitives();

  void SpawnGraphTestFunc();

  void SapwnCubePrimitives();

  void SpawnInstancingPrimitives();

  void SpawnIndirectDrawPrimitives();

  ESpawnedType SpawnedType = ESpawnedType::None;

  void Update(float deltaTime);

  void Draw();

  void OnMouseButton() {}

  void OnMouseMove(int32_t xOffset, int32_t yOffset);

  void Resize(int32_t width, int32_t height);

  void Release();

  Camera* MainCamera = nullptr;

  std::vector<Object*> SpawnedObjects;

  std::shared_ptr<Window> m_window_;

  std::future<void>    ResourceLoadCompleteEvent;
  std::vector<Object*> CompletedAsyncLoadObjects;
  MutexLock            AsyncLoadLock;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_GAME_H