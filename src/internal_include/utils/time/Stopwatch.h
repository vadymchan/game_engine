#ifndef UTILS_TIME_STOPWATCH_H
#define UTILS_TIME_STOPWATCH_H

#include <chrono>

namespace game_engine {

class Stopwatch {
  private:
  using Clock     = std::chrono::high_resolution_clock;
  using TimePoint = Clock::time_point;
  using Duration  = Clock::duration;

  std::chrono::high_resolution_clock::time_point m_startTime_;
  bool                                           m_isRunning_{false};

  public:
  void               start();
  void               stop();
  void               reset();
  [[nodiscard]] auto elapsedTime() const;
  [[nodiscard]] auto isRunning() const -> bool;
};

/// Represents the duration between two frames. Used for frame rate-independent
/// updates.
using DeltaTime = Stopwatch;

/// Represents the time taken to process a single frame. Used for performance
/// metrics.
using FrameTime = Stopwatch;

/// Represents the time elapsed since a significant event. Used for measuring
/// code execution durations.
using ElapsedTime = Stopwatch;

}  // namespace game_engine
#endif
