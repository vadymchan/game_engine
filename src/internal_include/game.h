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
  enum class ESpawnedType {
    None = 0,
    TestPrimitive,
    CubePrimitive,
    InstancingPrimitive,
    IndirectDrawPrimitive,
  };

  Game(std::shared_ptr<Window> window)
      : m_window_(std::move(window)) {}

  ~Game() {}

  void processInput(/*float deltaTime*/) {
    // TODO: implement
    // static float MoveDistancePerSecond = 200.0f;
    //// static float MoveDistancePerSecond = 10.0f;
    // const float  CurrentDistance = MoveDistancePerSecond * deltaTime;

    //// Process Key Event
    // if (g_KeyState['a'] || g_KeyState['A']) {
    //   m_mainCamera_->moveShift(-CurrentDistance);
    // }
    // if (g_KeyState['d'] || g_KeyState['D']) {
    //   m_mainCamera_->moveShift(CurrentDistance);
    // }
    //// if (g_KeyState['1']) m_mainCamera_->rotateForwardAxis(-0.1f);
    //// if (g_KeyState['2']) m_mainCamera_->rotateForwardAxis(0.1f);
    //// if (g_KeyState['3']) m_mainCamera_->rotateUpAxis(-0.1f);
    //// if (g_KeyState['4']) m_mainCamera_->rotateUpAxis(0.1f);
    //// if (g_KeyState['5']) m_mainCamera_->rotateRightAxis(-0.1f);
    //// if (g_KeyState['6']) m_mainCamera_->rotateRightAxis(0.1f);
    // if (g_KeyState['w'] || g_KeyState['W']) {
    //   m_mainCamera_->moveForward(CurrentDistance);
    // }
    // if (g_KeyState['s'] || g_KeyState['S']) {
    //   m_mainCamera_->moveForward(-CurrentDistance);
    // }
    // if (g_KeyState['+']) {
    //   MoveDistancePerSecond = Max(MoveDistancePerSecond + 10.0f, 0.0f);
    // }
    // if (g_KeyState['-']) {
    //   MoveDistancePerSecond = Max(MoveDistancePerSecond - 10.0f, 0.0f);
    // }
  }

  void setup();

  void spawnObjects(ESpawnedType spawnType);

  void removeSpawnedObjects();

  void spawnTestPrimitives();

  void spawnGraphTestFunc();

  void spawnCubePrimitives();

  void spawnInstancingPrimitives();

  void spawnIndirectDrawPrimitives();

  void update(float deltaTime);

  void draw();

  // TODO: not used
  void onMouseButton() {}

  // TODO: not used
  void onMouseMove(int32_t xOffset, int32_t yOffset);

  void resize(int32_t width, int32_t height);

  void release();

  Camera* m_mainCamera_ = nullptr;

  std::vector<Object*> m_spawnedObjects_;

  ESpawnedType m_spawnedType_ = ESpawnedType::None;

  std::shared_ptr<Window> m_window_;

  // TODO: not used
  std::future<void>    m_resourceLoadCompleteEvent_;
  // TODO: not used
  std::vector<Object*> m_completedAsyncLoadObjects_;

  // TODO: not used
  MutexLock m_asyncLoadLock_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_GAME_H