#ifndef GAME_ENGINE_STOPWATCH_H
#define GAME_ENGINE_STOPWATCH_H

#include <chrono>

namespace game_engine {

class Stopwatch {
  public:
  using Clock     = std::chrono::high_resolution_clock;
  using TimePoint = Clock::time_point;
  using Duration  = Clock::duration;

  void               start();
  void               stop();
  void               resume();
  void               pause();
  void               reset();
  [[nodiscard]] auto elapsedTime() const;
  [[nodiscard]] auto isRunning() const -> bool;

  private:
  TimePoint m_startTime_;
  TimePoint m_pausedTime_{}; // default init to epoch
  bool      m_isRunning_{false};
};

/// Represents the duration between two frames. Used for frame
/// rate-independent updates.
using DeltaTime = Stopwatch;

/// Represents the time taken to process a single frame. Used for performance
/// metrics.
using FrameTime = Stopwatch;

/// Represents the time elapsed since a significant event. Used for measuring
/// code execution durations.
using ElapsedTime = Stopwatch;

}  // namespace game_engine
#endif // GAME_ENGINE_STOPWATCH_H
