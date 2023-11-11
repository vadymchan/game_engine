#include "utils/logger/global_logger.h"

namespace game_engine {

void GlobalLogger::AddLogger(const std::shared_ptr<ILogger>& logger) {
  s_loggers.push_back(logger);
}

void GlobalLogger::Log(LogLevel logLevel, const std::string& message) {
  for (auto& logger : s_loggers) {
    logger->log(logLevel, message);
  }
}
}  // namespace game_engine