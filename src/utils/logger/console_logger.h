#ifndef GAME_ENGINE_CONSOLE_LOGGER_H
#define GAME_ENGINE_CONSOLE_LOGGER_H

#include "utils/logger/i_logger.h"

#include <spdlog/spdlog.h>

#include <memory>

namespace game_engine {

enum class ConsoleStreamType {
  StdOut,
  StdErr,
  // StdLog, - not needed for now
};

class ConsoleLogger : public ILogger {
  public:
  ConsoleLogger(const std::string& loggerName    = std::string(s_kDefaultLoggerName),
                LogLevel           logLevel      = s_kDefaultLogLevel,
                const std::string& pattern       = std::string(s_kDefaultPattern),
                ConsoleStreamType  consoleType   = s_kDefaultConsoleType,
                bool               multiThreaded = true,
                bool               colored       = true);

  ~ConsoleLogger() override;

  void log(LogLevel                    level,
           const std::string&          message,
           const std::source_location& location = std::source_location::current()) override;

  [[nodiscard]] const std::string& getPattern() const;
  [[nodiscard]] LogLevel           getLogLevel() const;

  void setLoggerName(const std::string& name);
  void setPattern(const std::string& pattern);
  void setLogLevel(LogLevel level);

  static constexpr std::string_view  s_kDefaultLoggerName  = "default_logger";
  static constexpr LogLevel          s_kDefaultLogLevel    = LogLevel::Trace;
  static constexpr std::string_view  s_kDefaultPattern     = "[%^%l%$] %v";
  static constexpr ConsoleStreamType s_kDefaultConsoleType = ConsoleStreamType::StdOut;

  private:
  std::string                     m_pattern_;
  std::shared_ptr<spdlog::logger> m_logger_;
  LogLevel                        m_logLevel_;
  ConsoleStreamType               m_consoleType_;
  bool                            m_isMultithreaded_;
  bool                            m_isColored_;
};
}  // namespace game_engine
#endif  // GAME_ENGINE_CONSOLE_LOGGER_H