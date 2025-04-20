#ifndef GAME_ENGINE_STOPWATCH_H
#define GAME_ENGINE_STOPWATCH_H

#include <chrono>

namespace game_engine {

class Stopwatch {
  public:
  using DurationNano  = std::chrono::nanoseconds;
  using DurationMicro = std::chrono::microseconds;
  using DurationMilli = std::chrono::milliseconds;
  using DurationSec   = std::chrono::seconds;
  using DurationMin   = std::chrono::minutes;
  using DurationHour  = std::chrono::hours;

  template <typename Type, typename Period = std::ratio<1>>
  using DurationCustom = std::chrono::duration<Type, Period>;

  // std::ratio<1> is seconds - nominator = 1, denominator = 1
  template <typename Period = std::ratio<1>>
  using DurationFloat = DurationCustom<float, Period>;

  template <typename Period = std::ratio<1>>
  using DurationDouble = DurationCustom<double, Period>;

  [[nodiscard]] auto isRunning() const -> bool;

  void start();
  void stop();
  void resume();
  void pause();
  void reset();

  /**
   * @tparam DurationType A std::chrono::duration type or a duration alias
   * defined in the Stopwatch class. Usage of standard std::chrono duration
   * types is discouraged to avoid tight coupling with the std::chrono library.
   */
  template <typename DurationType = DurationFloat<>>
  [[nodiscard]] auto elapsedTime() const {
    static_assert(std::is_base_of<std::chrono::duration<typename DurationType::rep, typename DurationType::period>,
                                  DurationType>::value,
                  "DurationType must be a std::chrono::duration type");

    Duration currentElapsed = m_isRunning_ ? (Clock::now() - m_startTime_) : (m_pausedTime_ - m_startTime_);
    return std::chrono::duration_cast<DurationType>(currentElapsed).count();
  }

  private:
  using Clock     = std::chrono::high_resolution_clock;
  using TimePoint = Clock::time_point;
  using Duration  = Clock::duration;

  TimePoint m_startTime_;
  TimePoint m_pausedTime_{};  // default initialize to epoch
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
#endif  // GAME_ENGINE_STOPWATCH_H
