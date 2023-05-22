#pragma once
#include "logger.h"

namespace GameEngine
{
	class SPDLogLogger : public Logger<SPDLogLogger> 
	{
	public:
		void LogDebug(const std::string& message) {
			// Implement SPDLog-specific methods here
		}

		void LogInfo(const std::string& message) {
			// Implement SPDLog-specific methods here
		}

		void LogWarn(const std::string& message) {
			// Implement SPDLog-specific methods here
		}

		void LogError(const std::string& message) {
			// Implement SPDLog-specific methods here
		}
	};
}

