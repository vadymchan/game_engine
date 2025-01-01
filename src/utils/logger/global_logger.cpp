#include "utils/logger/global_logger.h"

namespace game_engine {

void GlobalLogger::AddLogger(const std::shared_ptr<ILogger>& logger) {
  s_loggers.push_back(logger);
}

void GlobalLogger::Log(LogLevel logLevel, const std::string& message) {
  for (auto& logger : s_loggers) {
    // TODO: consider using fmt::format or std::format
    logger->log(logLevel, message);
  }
}

std::shared_ptr<ILogger> GlobalLogger::GetLogger(const std::string& name) {
  for (const auto& logger : s_loggers) {
    // returns the first logger with the given name
    if (logger->getLoggerName() == name) {
      return logger;
    }
  }
  return nullptr;
}
}  // namespace game_engine
