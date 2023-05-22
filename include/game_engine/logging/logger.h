#pragma once


#include <memory>
#include <string>

namespace GameEngine 
{
    //using CRTP pattern
    template<typename DerivedLogger>
    class Logger {
    public:
        void Debug(const std::string& message) { impl().LogDebug(message); }
        void Info(const std::string& message) { impl().LogInfo(message); }
        void Warn(const std::string& message) { impl().LogWarn(message); }
        void Error(const std::string& message) { impl().LogError(message); }

    private:
        // cast to derived type
        DerivedLogger& impl() { return static_cast<DerivedLogger&>(*this); }
    };


}  // namespace GameEngine
