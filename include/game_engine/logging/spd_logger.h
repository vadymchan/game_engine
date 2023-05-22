#pragma once
#include "logger.h"

namespace GameEngine
{
	class SPDLogger : public Logger<SPDLogger> 
	{
	public:
		void Debug(const std::string& message) {
			// Implement SPDLog-specific methods here
		}

		void Info(const std::string& message) {
			// Implement SPDLog-specific methods here
		}

		void Warn(const std::string& message) {
			// Implement SPDLog-specific methods here
		}

		void Error(const std::string& message) {
			// Implement SPDLog-specific methods here
		}
	};
}

