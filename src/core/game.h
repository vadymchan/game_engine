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

  void processInput() {}

  void setup();

  void update(float deltaTime);

  void linkWithEngine(const Engine& engine);

  void onMouseMove(int32_t xOffset, int32_t yOffset);

  void release();

  private:
  void setupInputHandlers();

  static constexpr float s_speedChangeStep = 0.5f;
  static constexpr float s_minMovingSpeed  = 0.1f;

  Window* m_window_ = nullptr;

  float m_movingSpeed_ = 10.0f;
  // TODO: add key bindings for sensitivity change
  float m_mouseSensitivity_ = 0.1f;

  std::bitset<static_cast<size_t>(Action::Count)> m_actionStates_;

  float m_yaw_                       = 0.0f;
  float m_pitch_                     = 0.0f;
  bool  m_isRightMouseButtonPressed_ = false;

  Scene* m_scene_ = nullptr;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_GAME_H