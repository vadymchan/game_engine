#ifndef ARISE_LOG_HELPERS_H
#define ARISE_LOG_HELPERS_H

#include "utils/logger/global_logger.h"

#include <source_location>

namespace arise {

// -----------------------------------------------------------------------------
// Level: Trace
// -----------------------------------------------------------------------------

inline void LogTrace(const std::string& msg, const std::source_location& loc = std::source_location::current()) {
  GlobalLogger::Log(LogLevel::Trace, msg, loc);
}

template <typename... Args>
inline void LogTrace(fmt::format_string<Args...> fmtStr,
                     Args&&... args,
                     const std::source_location& loc = std::source_location::current()) {
  GlobalLogger::Log(LogLevel::Trace, fmt::format(fmtStr, std::forward<Args>(args)...), loc);
}

// -----------------------------------------------------------------------------
// Level: Debug
// -----------------------------------------------------------------------------

inline void LogDebug(const std::string& msg, const std::source_location& loc = std::source_location::current()) {
  GlobalLogger::Log(LogLevel::Debug, msg, loc);
}

template <typename... Args>
inline void LogDebug(fmt::format_string<Args...> fmtStr,
                     Args&&... args,
                     const std::source_location& loc = std::source_location::current()) {
  GlobalLogger::Log(LogLevel::Debug, fmt::format(fmtStr, std::forward<Args>(args)...), loc);
}

// -----------------------------------------------------------------------------
// Level: Info
// -----------------------------------------------------------------------------

inline void LogInfo(const std::string& msg, const std::source_location& loc = std::source_location::current()) {
  GlobalLogger::Log(LogLevel::Info, msg, loc);
}

template <typename... Args>
inline void LogInfo(fmt::format_string<Args...> fmtStr,
                    Args&&... args,
                    const std::source_location& loc = std::source_location::current()) {
  GlobalLogger::Log(LogLevel::Info, fmt::format(fmtStr, std::forward<Args>(args)...), loc);
}

// -----------------------------------------------------------------------------
// Level: Warning
// -----------------------------------------------------------------------------

inline void LogWarn(const std::string& msg, const std::source_location& loc = std::source_location::current()) {
  GlobalLogger::Log(LogLevel::Warning, msg, loc);
}

template <typename... Args>
inline void LogWarn(fmt::format_string<Args...> fmtStr,
                    Args&&... args,
                    const std::source_location& loc = std::source_location::current()) {
  GlobalLogger::Log(LogLevel::Warning, fmt::format(fmtStr, std::forward<Args>(args)...), loc);
}

// -----------------------------------------------------------------------------
// Level: Error
// -----------------------------------------------------------------------------

inline void LogError(const std::string& msg, const std::source_location& loc = std::source_location::current()) {
  GlobalLogger::Log(LogLevel::Error, msg, loc);
}

template <typename... Args>
inline void LogError(fmt::format_string<Args...> fmtStr,
                     Args&&... args,
                     const std::source_location& loc = std::source_location::current()) {
  GlobalLogger::Log(LogLevel::Error, fmt::format(fmtStr, std::forward<Args>(args)...), loc);
}

// -----------------------------------------------------------------------------
// Level: Fatal
// -----------------------------------------------------------------------------

inline void LogFatal(const std::string& msg, const std::source_location& loc = std::source_location::current()) {
  GlobalLogger::Log(LogLevel::Fatal, msg, loc);
}

template <typename... Args>
inline void LogFatal(fmt::format_string<Args...> fmtStr,
                     Args&&... args,
                     const std::source_location& loc = std::source_location::current()) {
  GlobalLogger::Log(LogLevel::Fatal, fmt::format(fmtStr, std::forward<Args>(args)...), loc);
}

}  // namespace arise

#endif  // ARISE_LOG_HELPERS_H