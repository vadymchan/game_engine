#include "utils/time/Stopwatch.h"

namespace game_engine {

void Stopwatch::start() {
  m_isRunning_ = true;
  m_startTime_ = Clock::now();
}

void Stopwatch::stop() {
  m_isRunning_ = false;
  m_startTime_ = TimePoint{};
}

void Stopwatch::reset() {
  m_startTime_ = Clock::now();
}

auto Stopwatch::elapsedTime() const {
  Duration elapsed = m_isRunning_ ? (Clock::now() - m_startTime_) : Duration{};
  return std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
}

auto Stopwatch::isRunning() const -> bool {
  return m_isRunning_;
}

}  // namespace game_engine