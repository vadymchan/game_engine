#pragma once


#include <memory>
#include <string>

// Forward declaration of spdlog::logger
// This lets us use spdlog::logger as a type in this header file
// without needing to include the full definition of spdlog::logger.
namespace spdlog {
    class logger;
}

namespace GameEngine {

    class Logger {
    public:
        Logger(const std::string& name);
        ~Logger();

        void Debug(const std::string& message);
        void Info(const std::string& message);
        void Warn(const std::string& message);
        void Error(const std::string& message);

    private:
        std::shared_ptr<spdlog::logger> logger;
    };

}  // namespace GameEngine
