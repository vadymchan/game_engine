#include "utils/time/timing_manager.h"

namespace arise {
void TimingManager::initialize() {
  m_totalElapsedTime_.start();
  m_deltaTime_.start();
}

void TimingManager::update() {
  m_deltaTime_.pause();

  m_cachedDeltaTime_ = m_deltaTime_.elapsedTime<DeltaTime::DurationFloat<>>();

  m_cachedFPS_ = (m_cachedDeltaTime_ > 0.0f) ? (1.0f / m_cachedDeltaTime_) : 0.0f;

  m_cachedFrameTime_ = m_deltaTime_.elapsedTime<DeltaTime::DurationFloat<std::milli>>();

  m_deltaTime_.reset();
  m_deltaTime_.start();
}
}  // namespace arise