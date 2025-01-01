#include "utils/logger/memory_logger.h"

#include <chrono>

namespace game_engine {

void MemoryLogger::log(LogLevel logLevel, const std::string& message) {
  LogEntry entry{std::chrono::system_clock::now(), logLevel, message};

  m_entries_.push_back(std::move(entry));
  if (m_maxEntries_ > 0 && m_entries_.size() > m_maxEntries_) {
    // If we exceed the max, remove the oldest entry
    m_entries_.erase(m_entries_.begin());
  }
}

const std::vector<LogEntry>& MemoryLogger::getLogEntries() const {
  return m_entries_;
}

void MemoryLogger::setMaxEntries(size_t maxEntries) {
  m_maxEntries_ = maxEntries;
}

size_t MemoryLogger::getMaxEntries() const {
  return m_maxEntries_;
}

}  // namespace game_engine
