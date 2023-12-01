#ifndef GAME_ENGINE_ENGINE_H
#define GAME_ENGINE_ENGINE_H

#include "platform/common/window.h"
#include "utils/logger/console_logger.h"
#include "utils/time/stopwatch.h"
#include "input/input_manager.h"

namespace game_engine {

class Engine {
  public:
  Engine() = default;

  Engine(const Engine&)                     = delete;
  auto operator=(const Engine&) -> Engine&  = delete;
  Engine(Engine&&)                          = delete;
  auto operator=(const Engine&&) -> Engine& = delete;

  ~Engine() = default;

  auto init() -> bool {
    bool successfullyInited{true};
	  m_inputManager_.routeEvent(e);
  
    return successfullyInited;
  }
  void run() {
  }

  private:
  void update_(float deltaTime) {}

  Window        m_window_;
  DeltaTime     m_deltaTime_;
  ConsoleLogger m_consoleLogger_;
  InputManager  m_inputManager_;

  // TODO: logger
  // TODO: graphics
  // TODO: time (clock, stopwatch)
  // TODO: event
  // TODO: intput

  // TODO: memory manager
};

}  // namespace game_engine

#endif