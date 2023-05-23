#pragma once
#include "logger.h"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace GameEngine
{
    class SPDLogger : public Logger<SPDLogger>
    {
    public:
        SPDLogger() {
            m_logger_ = spdlog::stdout_color_mt("SPDLogger");
            spdlog::set_pattern("[%T] [%l] %v");
        }

        void LogDebug(const std::string& message) 
        {
            m_logger_->debug(message);
        }

        void LogInfo(const std::string& message) 
        {
            m_logger_->info(message);
        }

        void LogWarn(const std::string& message) 
        {
            m_logger_->warn(message);
        }

        void LogError(const std::string& message) 
        {
            m_logger_->error(message);
        }

    private:
        std::shared_ptr<spdlog::logger> m_logger_;
    };
}

