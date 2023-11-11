#ifndef GAME_ENGINE_CONSOLE_LOGGER_H
#define GAME_ENGINE_CONSOLE_LOGGER_H

#include "i_logger.h"

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
  static constexpr std::string_view s_kDefaultLoggerName = "default_logger";
  static constexpr LogLevel         s_kDefaultLogLevel   = LogLevel::Trace;
  static constexpr std::string_view s_kDefaultPattern
      = "%Y-%m-%d %H:%M:%S.%e %^[%l]%$ %v";
  static constexpr ConsoleStreamType s_kDefaultConsoleType
      = ConsoleStreamType::StdOut;

  private:
  std::string                     m_pattern_;
  std::shared_ptr<spdlog::logger> m_logger_;
  LogLevel                        m_logLevel_;
  ConsoleStreamType               m_consoleType_;
  bool                            m_isMultithreaded_;
  bool                            m_isColored_;

  public:
  ConsoleLogger(const std::string& logger_name
                = std::string(s_kDefaultLoggerName),
                LogLevel           log_level   = s_kDefaultLogLevel,
                const std::string& pattern     = std::string(s_kDefaultPattern),
                ConsoleStreamType  consoleType = s_kDefaultConsoleType,
                bool               multi_threaded = true,
                bool               colored        = true);

  void log(LogLevel logLevel, const std::string& message) override;

  // Getters
  [[nodiscard]] auto getLoggerName() const -> const std::string&;
  [[nodiscard]] auto getPattern() const -> const std::string&;
  [[nodiscard]] auto getLogLevel() const -> LogLevel;

  // Setters
  void setLoggerName(const std::string& name);
  void setPattern(const std::string& pattern);
  void setLogLevel(LogLevel level);
};
}  // namespace game_engine
#endif
