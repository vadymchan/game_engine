#ifndef GAME_ENGINE_FILE_LOGGER_H
#define GAME_ENGINE_FILE_LOGGER_H

#include "utils/logger/i_logger.h"

#include <spdlog/spdlog.h>

#include <memory>
#include <string>

namespace game_engine {

class FileLogger : public ILogger {
  public:
  FileLogger(const std::string& loggerName = std::string(s_kDefaultLoggerName),
             LogLevel           logLevel   = s_kDefaultLogLevel,
             const std::string& pattern    = std::string(s_kDefaultPattern),
             const std::string& filePath   = std::string(s_kDefaultLogFilePath),
             bool               multiThreaded  = true,
             bool               truncateOnOpen = false);

  void log(LogLevel logLevel, const std::string& message) override;

  [[nodiscard]] auto getLoggerName() const -> const std::string&;
  [[nodiscard]] auto getPattern() const -> const std::string&;
  [[nodiscard]] auto getLogLevel() const -> LogLevel;
  [[nodiscard]] auto getFilePath() const -> const std::string&;

  void setLoggerName(const std::string& name);
  void setPattern(const std::string& pattern);
  void setLogLevel(LogLevel level);
  void setFilePath(const std::string& filePath);

  static constexpr std::string_view s_kDefaultLoggerName = "file_logger";
  static constexpr LogLevel         s_kDefaultLogLevel   = LogLevel::Trace;
  static constexpr std::string_view s_kDefaultPattern
      = "%Y-%m-%d %H:%M:%S.%e %^[%l]%$ %v";
  static constexpr std::string_view s_kDefaultLogFilePath = "output.log";

  private:
  std::shared_ptr<spdlog::logger> m_logger_;
  std::string                     m_pattern_;
  std::string                     m_filePath_;
  LogLevel                        m_logLevel_;
  bool                            m_isMultithreaded_;
  bool                            m_truncateOnOpen_;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_FILE_LOGGER_H
