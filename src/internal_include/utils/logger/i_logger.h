#ifndef GAME_ENGINE_I_LOGGER_H
#define GAME_ENGINE_I_LOGGER_H

#include <string>

namespace game_engine {

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
  virtual void log(LogLevel logLevel, const std::string& message) = 0;

  ILogger()                                  = default;
  ILogger(const ILogger&)                    = delete;
  auto operator=(const ILogger&) -> ILogger& = delete;
  ILogger(ILogger&&)                         = delete;
  auto operator=(ILogger&&) -> ILogger&      = delete;

  virtual ~ILogger() = default;
};

}  // namespace game_engine

#endif // GAME_ENGINE_I_LOGGER_H
