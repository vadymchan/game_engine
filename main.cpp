//#include "engine.h"
//
//namespace game_engine {
//
//#if defined(_WIN32) || defined(_WIN64)
//  #include <Windows.h>
//int WINAPI wWinMain(_In_ HINSTANCE     hInstance,
//                    _In_opt_ HINSTANCE hPrevInstance,
//                    _In_ PWSTR         pCmdLine,
//                    _In_ int           nCmdShow) {
//#else
//int main(int argc, char* argv[]) {
//
//#endif
//
//  return 0;
//}
//
//}  // namespace game_engine

#include "utils/logger/global_logger.h"
#include "utils/logger/console_logger.h"

#include <iostream>

auto main() -> int {
  // Initialize a console logger
  auto consoleLogger = std::make_shared<game_engine::ConsoleLogger>(
      "TestLogger",
      game_engine::LogLevel::Debug,
      "%Y-%m-%d %H:%M:%S.%e %^[%l]%$ %v",
      game_engine::ConsoleStreamType::StdOut,
      true,
      true);

  game_engine::ConsoleLogger give_me_a_name;

  // Add the console logger to the global logger
  game_engine::GlobalLogger::AddLogger(consoleLogger);

  // Log messages at different levels
  game_engine::GlobalLogger::Log(game_engine::LogLevel::Info,
                                 "This is an info message.");
  game_engine::GlobalLogger::Log(game_engine::LogLevel::Warning,
                                 "This is a warning message.");
  game_engine::GlobalLogger::Log(game_engine::LogLevel::Error,
                                 "This is an error message.");

  game_engine::GlobalLogger::Log(game_engine::LogLevel::Info,
                                 "This is an info message {}.",
                                 "with variadic templated args");

  consoleLogger->log(game_engine::LogLevel::Info, "This is local info message");
  consoleLogger->log(
      game_engine::LogLevel::Info, "This is local info message {}", "with variadic templated metod");

  std::cout << "Check the console output for log messages." << '\n';

  return 0;
}