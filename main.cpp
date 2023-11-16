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

#define SDL_MAIN_HANDLED

#include <SDL.h>


#include <iostream>

//#include <Windows.h>
//int WINAPI wWinMain(_In_ HINSTANCE     hInstance,
//                    _In_opt_ HINSTANCE hPrevInstance,
//                    _In_ PWSTR         pCmdLine,
//                    _In_ int           nCmdShow) { 
  
int main(int argc, char* argv[]) {
    // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError()
              << std::endl;
    return 1;
  }

  // Create a window
  SDL_Window* window = SDL_CreateWindow("SDL Tutorial",
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        800,
                                        600,
                                        SDL_WINDOW_SHOWN);
  if (window == nullptr) {
    std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError()
              << std::endl;
    SDL_Quit();
    return 1;
  }

  // Main loop flag
  bool quit = false;

  // Event handler
  SDL_Event e;

  // While application is running
  while (!quit) {
    // Handle events on queue
    while (SDL_PollEvent(&e) != 0) {
      // User requests quit
      if (e.type == SDL_QUIT) {
        quit = true;
      }
    }

    // Your rendering and updating code goes here
  }

  // Destroy window
  SDL_DestroyWindow(window);

  // Quit SDL subsystems
  SDL_Quit();

  return 0;
}


//
//#include "utils/logger/global_logger.h"
//#include "utils/logger/console_logger.h"
//
//#include <iostream>
//
//auto main() -> int {
//  // Initialize a console logger
//  auto consoleLogger = std::make_shared<game_engine::ConsoleLogger>(
//      "TestLogger",
//      game_engine::LogLevel::Debug,
//      "%Y-%m-%d %H:%M:%S.%e %^[%l]%$ %v",
//      game_engine::ConsoleStreamType::StdOut,
//      true,
//      true);
//
//  game_engine::ConsoleLogger give_me_a_name;
//
//  // Add the console logger to the global logger
//  game_engine::GlobalLogger::AddLogger(consoleLogger);
//
//  // Log messages at different levels
//  game_engine::GlobalLogger::Log(game_engine::LogLevel::Info,
//                                 "This is an info message.");
//  game_engine::GlobalLogger::Log(game_engine::LogLevel::Warning,
//                                 "This is a warning message.");
//  game_engine::GlobalLogger::Log(game_engine::LogLevel::Error,
//                                 "This is an error message.");
//
//  consoleLogger->log(game_engine::LogLevel::Info, "This is local info message");
//
//  std::cout << "Check the console output for log messages." << '\n';
//
//  return 0;
//}