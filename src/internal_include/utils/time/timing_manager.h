#ifndef GAME_ENGINE_TIMING_MANAGER_H
#define GAME_ENGINE_TIMING_MANAGER_H

#include "utils/time/stopwatch.h"

#include <chrono>

namespace game_engine {

class TimingManager {
  public:
  /**
   * Initializes all timing components and starts measuring total time.
   */
  void initialize() {
    m_totalElapsedTime_.start();
    m_deltaTime_.start();
  }

  /**
   * Should be called once per frame (in the main game loop).
   * Updates delta time and other timing statistics.
   */
  void update() {
    m_deltaTime_.pause();

    m_cachedDeltaTime_ = m_deltaTime_.elapsedTime<DeltaTime::DurationFloat<>>();

    m_cachedFPS_
        = (m_cachedDeltaTime_ > 0.0f) ? (1.0f / m_cachedDeltaTime_) : 0.0f;

    m_cachedFrameTime_
        = m_deltaTime_.elapsedTime<DeltaTime::DurationFloat<std::milli>>();

    m_deltaTime_.reset();
    m_deltaTime_.start();
  }

  /**
   * Returns the time taken to process the last frame in milliseconds.
   * Derived from DeltaTime.
   */
  float getFrameTime() const { return m_cachedFrameTime_; }

  /**
   * Returns the time in seconds since the previous frame.
   */
  float getDeltaTime() const { return m_cachedDeltaTime_; }

  /**
   * Returns the instantaneous frames-per-second based on the last frame.
   */
  float getFPS() const { return m_cachedFPS_; }

  /**
   * Returns the total time in seconds since the start of the application.
   */
  [[nodiscard]] float getTotalElapsedTime() const {
    return m_totalElapsedTime_.elapsedTime<ElapsedTime::DurationFloat<>>();
  }

  private:
  ElapsedTime m_totalElapsedTime_;

  DeltaTime m_deltaTime_;

  float m_cachedDeltaTime_;
  float m_cachedFPS_;
  float m_cachedFrameTime_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_TIMING_MANAGER_H
