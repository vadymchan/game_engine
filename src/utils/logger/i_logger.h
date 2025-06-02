#ifndef ARISE_I_LOGGER_H
#define ARISE_I_LOGGER_H

#include <source_location>
#include <string>

namespace arise {

enum class LogLevel {
  Trace,
  Debug,
  Info,
  Warning,
  Error,
  Fatal,
  Off,
};

class ILogger {
  public:
  ILogger(std::string loggerName)
      : loggerName(std::move(loggerName)) {}

  ILogger(const ILogger&) = delete;
  ILogger(ILogger&&)      = delete;

  virtual ~ILogger() = default;

  auto operator=(const ILogger&) -> ILogger& = delete;
  auto operator=(ILogger&&) -> ILogger&      = delete;

  virtual void log(LogLevel                    level,
                   const std::string&          message,
                   const std::source_location& loc = std::source_location::current())
      = 0;

  auto getLoggerName() const -> const std::string& { return loggerName; }

  private:
  std::string loggerName;
};

}  // namespace arise

#endif  // ARISE_I_LOGGER_H
