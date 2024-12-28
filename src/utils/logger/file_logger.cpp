#include "utils/logger/file_logger.h"

#include <spdlog/sinks/basic_file_sink.h>

namespace game_engine {

namespace {

auto createFileLogger(const std::string& loggerName,
                      const std::string& filePath,
                      bool               multiThreaded,
                      bool truncateOnOpen) -> std::shared_ptr<spdlog::logger> {
  std::shared_ptr<spdlog::sinks::sink> fileSink;
  if (multiThreaded) {
    fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        filePath, truncateOnOpen);
  } else {
    fileSink = std::make_shared<spdlog::sinks::basic_file_sink_st>(
        filePath, truncateOnOpen);
  }

  auto logger = std::make_shared<spdlog::logger>(loggerName, fileSink);
  return logger;
}

auto toSpdlogLevel(LogLevel level) -> spdlog::level::level_enum {
  switch (level) {
    case LogLevel::Trace:
      return spdlog::level::trace;
    case LogLevel::Debug:
      return spdlog::level::debug;
    case LogLevel::Info:
      return spdlog::level::info;
    case LogLevel::Warning:
      return spdlog::level::warn;
    case LogLevel::Error:
      return spdlog::level::err;
    case LogLevel::Fatal:
      return spdlog::level::critical;
    case LogLevel::Off:
      return spdlog::level::off;
    default:
      return spdlog::level::n_levels;
  }
}

}  // namespace

FileLogger::FileLogger(const std::string& loggerName,
                       LogLevel           logLevel,
                       const std::string& pattern,
                       const std::string& filePath,
                       bool               multiThreaded,
                       bool               truncateOnOpen)
    : m_logger_(
          createFileLogger(loggerName, filePath, multiThreaded, truncateOnOpen))
    , m_pattern_(pattern)
    , m_filePath_(filePath)
    , m_logLevel_(logLevel)
    , m_isMultithreaded_(multiThreaded)
    , m_truncateOnOpen_(truncateOnOpen) {
  m_logger_->set_level(toSpdlogLevel(logLevel));
  m_logger_->set_pattern(m_pattern_);
}

void FileLogger::log(LogLevel logLevel, const std::string& message) {
  if (!m_logger_) {
    return;
  }

  switch (logLevel) {
    case LogLevel::Fatal:
      m_logger_->critical(message);
      break;
    case LogLevel::Error:
      m_logger_->error(message);
      break;
    case LogLevel::Warning:
      m_logger_->warn(message);
      break;
    case LogLevel::Info:
      m_logger_->info(message);
      break;
    case LogLevel::Debug:
      m_logger_->debug(message);
      break;
    case LogLevel::Trace:
      m_logger_->trace(message);
      break;
    case LogLevel::Off:
      // Do nothing
      break;
    default:
      m_logger_->info(message);
      break;
  }
}

auto FileLogger::getLoggerName() const -> const std::string& {
  return m_logger_->name();
}

auto FileLogger::getPattern() const -> const std::string& {
  return m_pattern_;
}

auto FileLogger::getLogLevel() const -> LogLevel {
  return m_logLevel_;
}

auto FileLogger::getFilePath() const -> const std::string& {
  return m_filePath_;
}

// Setters
void FileLogger::setLoggerName(const std::string& name) {
  // Recreate the logger to change the name
  m_logger_ = createFileLogger(
      name, m_filePath_, m_isMultithreaded_, m_truncateOnOpen_);
  m_logger_->set_level(toSpdlogLevel(m_logLevel_));
  m_logger_->set_pattern(m_pattern_);
}

void FileLogger::setPattern(const std::string& pattern) {
  m_pattern_ = pattern;
  if (m_logger_) {
    m_logger_->set_pattern(pattern);
  }
}

void FileLogger::setLogLevel(LogLevel level) {
  m_logLevel_ = level;
  if (m_logger_) {
    m_logger_->set_level(toSpdlogLevel(level));
  }
}

void FileLogger::setFilePath(const std::string& filePath) {
  // TODO: for simplicity, we'll just store the new path. Consider
  // reinitializing the logger.
  m_filePath_ = filePath;
}

}  // namespace game_engine
