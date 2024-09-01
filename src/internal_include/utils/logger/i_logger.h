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
  // ======= BEGIN: public constructors =======================================

  ILogger()               = default;
  ILogger(const ILogger&) = delete;
  ILogger(ILogger&&)      = delete;

  // ======= END: public constructors   =======================================

  // ======= BEGIN: public destructor =========================================

  virtual ~ILogger() = default;

  // ======= END: public destructor   =========================================

  // ======= BEGIN: public overloaded operators ===============================

  auto operator=(const ILogger&) -> ILogger& = delete;
  auto operator=(ILogger&&) -> ILogger&      = delete;

  // ======= END: public overloaded operators   ===============================

  // ======= BEGIN: public overridden methods =================================

  virtual void log(LogLevel logLevel, const std::string& message) = 0;

  // ======= END: public overridden methods   =================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_I_LOGGER_H
