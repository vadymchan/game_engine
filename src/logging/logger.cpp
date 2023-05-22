#include "../include/game_engine/logging/logger.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>



namespace GameEngine {

    Logger::Logger(const std::string& name)
        : logger(spdlog::stdout_color_mt(name)) {
        logger->set_level(spdlog::level::debug);
    }

    Logger::~Logger() {
        spdlog::drop(logger->name());
    }

    void Logger::Debug(const std::string& message) {
        logger->debug(message);
    }

    void Logger::Info(const std::string& message) {
        logger->info(message);
    }

    void Logger::Warn(const std::string& message) {
        logger->warn(message);
    }

    void Logger::Error(const std::string& message) {
        logger->error(message);
    }

}  // namespace GameEngine
