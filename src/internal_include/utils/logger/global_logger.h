#ifndef GAME_ENGINE_GLOBAL_LOGGER_H
#define GAME_ENGINE_GLOBAL_LOGGER_H

#include "i_logger.h"

#include <memory>
#include <vector>

namespace game_engine {

class GlobalLogger {
  public:
  // ======= BEGIN: public static fields ======================================

  static void AddLogger(const std::shared_ptr<ILogger>& logger);

  static void Log(LogLevel logLevel, const std::string& message);

  // ======= END: public static fields   ======================================

  private:
  // ======= BEGIN: private static fields =====================================

  static inline std::vector<std::shared_ptr<ILogger>> s_loggers;

  // ======= END: private static fields   =====================================
};

}  // namespace game_engine

#endif  // GAME_ENGINE_GLOBAL_LOGGER_H
