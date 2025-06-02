#include "utils/logger/file_logger.h"

#include <spdlog/sinks/basic_file_sink.h>

#include <filesystem>

namespace arise {

namespace {

auto createFileLogger(const std::string& loggerName,
                      const std::string& filePath,
                      bool               multiThreaded,
                      bool               truncateOnOpen) -> std::shared_ptr<spdlog::logger> {
  std::shared_ptr<spdlog::sinks::sink> fileSink;
  if (multiThreaded) {
    fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filePath, truncateOnOpen);
  } else {
    fileSink = std::make_shared<spdlog::sinks::basic_file_sink_st>(filePath, truncateOnOpen);
  }

  auto logger = std::make_shared<spdlog::logger>(loggerName, fileSink);

  spdlog::register_logger(logger);

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
    : ILogger(loggerName)
    , m_logger_(createFileLogger(loggerName, filePath, multiThreaded, truncateOnOpen))
    , m_pattern_(pattern)
    , m_filePath_(filePath)
    , m_logLevel_(logLevel)
    , m_isMultithreaded_(multiThreaded)
    , m_truncateOnOpen_(truncateOnOpen) {
  if (m_logger_) {
    m_logger_->set_level(toSpdlogLevel(logLevel));
    m_logger_->set_pattern(m_pattern_);
  }
}

FileLogger::~FileLogger() {
  if (m_logger_) {
    spdlog::drop(m_logger_->name());
  }
}

void FileLogger::log(LogLevel logLevel, const std::string& message, const std::source_location& loc) {
  if (!m_logger_) {
    return;
  }

  auto file = std::filesystem::path(loc.file_name()).filename().string();

  std::string_view functionName{loc.function_name()};
  // strip off the parameter list (remove everything from '(' onward)
  if (auto p = functionName.find('('); p != std::string_view::npos) {
    functionName.remove_suffix(functionName.size() - p);
  }
  // strip off any namespace qualifiers (remove up to the last "::")
  if (auto ns = functionName.rfind("::"); ns != std::string_view::npos) {
    functionName.remove_prefix(ns + 2);
  }

  auto fullMsg = fmt::format("{}:{} | {}() | {}", file, loc.line(), functionName, message);

  switch (logLevel) {
    case LogLevel::Fatal:
      m_logger_->critical(fullMsg);
      break;
    case LogLevel::Error:
      m_logger_->error(fullMsg);
      break;
    case LogLevel::Warning:
      m_logger_->warn(fullMsg);
      break;
    case LogLevel::Info:
      m_logger_->info(fullMsg);
      break;
    case LogLevel::Debug:
      m_logger_->debug(fullMsg);
      break;
    case LogLevel::Trace:
      m_logger_->trace(fullMsg);
      break;
    case LogLevel::Off:
      // Do nothing
      break;
    default:
      m_logger_->info(fullMsg);
      break;
  }
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

void FileLogger::setLoggerName(const std::string& name) {
  if (m_logger_) {
    spdlog::drop(m_logger_->name());
  }

  m_logger_ = createFileLogger(name, m_filePath_, m_isMultithreaded_, m_truncateOnOpen_);

  if (m_logger_) {
    m_logger_->set_level(toSpdlogLevel(m_logLevel_));
    m_logger_->set_pattern(m_pattern_);
  }
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
  m_filePath_ = filePath;

  if (m_logger_) {
    spdlog::drop(m_logger_->name());

    m_logger_ = createFileLogger(getLoggerName(), filePath, m_isMultithreaded_, m_truncateOnOpen_);

    if (m_logger_) {
      m_logger_->set_level(toSpdlogLevel(m_logLevel_));
      m_logger_->set_pattern(m_pattern_);
    }
  }
}

}  // namespace arise