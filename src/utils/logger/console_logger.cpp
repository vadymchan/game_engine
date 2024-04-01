
#include "utils/logger/console_logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/stdout_sinks.h>

namespace game_engine {

namespace {

auto createStdOutLogger(const std::string& loggerName,
                        bool               multiThreaded,
                        bool colored) -> std::shared_ptr<spdlog::logger> {
  if (colored) {
    return multiThreaded ? spdlog::stdout_color_mt(loggerName)
                          : spdlog::stdout_color_st(loggerName);
  }
  return multiThreaded ? spdlog::stdout_logger_mt(loggerName)
                        : spdlog::stdout_logger_st(loggerName);
}

auto createStdErrLogger(const std::string& loggerName,
                        bool               multiThreaded,
                        bool colored) -> std::shared_ptr<spdlog::logger> {
  if (colored) {
    return multiThreaded ? spdlog::stderr_color_mt(loggerName)
                          : spdlog::stderr_color_st(loggerName);
  }
  return multiThreaded ? spdlog::stderr_logger_mt(loggerName)
                        : spdlog::stderr_logger_st(loggerName);
}

auto createLogger(const std::string& loggerName,
                  ConsoleStreamType  consoleType,
                  bool               multiThreaded,
                  bool colored) -> std::shared_ptr<spdlog::logger> {
  switch (consoleType) {
    case ConsoleStreamType::StdOut:
      return createStdOutLogger(loggerName, multiThreaded, colored);
    case ConsoleStreamType::StdErr:
      return createStdErrLogger(loggerName, multiThreaded, colored);
    default:
      return nullptr;
  }
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

ConsoleLogger::ConsoleLogger(const std::string& loggerName,
                             LogLevel           logLevel,
                             const std::string& pattern,
                             ConsoleStreamType  consoleType,
                             bool               isMultithreaded,
                             bool               isColored)
    : m_logger_(
        createLogger(loggerName, consoleType, isMultithreaded, isColored))
    , m_logLevel_(logLevel)
    , m_pattern_(pattern)
    , m_consoleType_(consoleType)
    , m_isMultithreaded_(isMultithreaded)
    , m_isColored_(isColored) {
  m_logger_->set_level(toSpdlogLevel(logLevel));
  m_logger_->set_pattern(pattern);
}

void ConsoleLogger::log(LogLevel logLevel, const std::string& message) {
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
  }
}

// Getters
auto ConsoleLogger::getLoggerName() const -> const std::string& {
  return m_logger_->name();
}

auto ConsoleLogger::getPattern() const -> const std::string& {
  return m_pattern_;
}

auto ConsoleLogger::getLogLevel() const -> LogLevel {
  return m_logLevel_;
}

// Setters
void ConsoleLogger::setLoggerName(const std::string& name) {
  m_logger_
      = createLogger(name, m_consoleType_, m_isMultithreaded_, m_isColored_);
}

void ConsoleLogger::setPattern(const std::string& pattern) {
  m_pattern_ = pattern;
  m_logger_->set_pattern(pattern);
}

void ConsoleLogger::setLogLevel(LogLevel level) {
  m_logLevel_ = level;
  m_logger_->set_level(toSpdlogLevel(level));
}

}  // namespace game_engine