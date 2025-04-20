#ifndef GAME_ENGINE_GAME_H
#define GAME_ENGINE_GAME_H

#include "gfx/renderer/renderer.h"

#include <bitset>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iterator>
#include <vector>

namespace game_engine {

class Engine;

// TODO: consider renaming class (e.g. Application)
class Game {
  public:
  enum class SpawnedType {
    None = 0,
    TestPrimitive,
    CubePrimitive,
    InstancingPrimitive,
    IndirectDrawPrimitive,
  };

  enum class Action {
    MoveForward,
    MoveBackward,
    MoveLeft,
    MoveRight,
    MoveUp,
    MoveDown,
    Count,
  };

  Game() {}

  virtual ~Game() {}

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

  void update(float deltaTime);

  void linkWithEngine(const Engine& engine);

  void onMouseMove(int32_t xOffset, int32_t yOffset);

  void release();

  SpawnedType m_spawnedType_ = SpawnedType::None;

  Window* m_window_ = nullptr;

  float m_movingSpeed_ = 10.0f;
  // TODO: add key bindings for sensitivity change
  float m_mouseSensitivity_ = 0.1f;

  std::bitset<static_cast<size_t>(Action::Count)> m_actionStates_;

  float m_yaw_                       = 0.0f;
  float m_pitch_                     = 0.0f;
  bool  m_isRightMouseButtonPressed_ = false;

  private:
  static constexpr float s_speedChangeStep = 0.5f;
  static constexpr float s_minMovingSpeed  = 0.1f;

  Scene* m_scene_ = nullptr;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_GAME_H