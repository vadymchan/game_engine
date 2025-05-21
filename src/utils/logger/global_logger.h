#ifndef GAME_ENGINE_GLOBAL_LOGGER_H
#define GAME_ENGINE_GLOBAL_LOGGER_H

#include "utils/logger/i_logger.h"

#include <memory>
#include <vector>

namespace game_engine {

class GlobalLogger {
  public:
  static void AddLogger(std::unique_ptr<ILogger> logger);

  static void Log(LogLevel                    level,
                  const std::string&          message,
                  const std::source_location& loc = std::source_location::current());

  static ILogger* GetLogger(const std::string& name);

  static void Shutdown();

  private:
  static inline std::vector<std::unique_ptr<ILogger>> s_loggers;
};

}  // namespace game_engine

#endif  // GAME_ENGINE_GLOBAL_LOGGER_H