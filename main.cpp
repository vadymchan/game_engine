#define SDL_MAIN_HANDLED

#include "engine.h"

// ----------------------------------------------
// Event system
#include "event/event.h"
#include "event/event_handler.h"
#include "event/keyboard_event_handler.h"
#include "event/mouse_event_handler.h"
#include "event/window_event_handler.h"
#include "event/window_event_manager.h"

// Input system
#include "input/input_manager.h"
#include "input/key.h"
#include "input/mouse.h"

// Platform specific
#include "platform/common/window.h"

// Utilities
#include "gfx/rhi/vulkan/spirv_util.h"
#include "utils/logger/console_logger.h"
#include "utils/logger/global_logger.h"
#include "utils/logger/i_logger.h"
#include "utils/time/stopwatch.h"

// ----------------------------------------------

#if (defined(_WIN32) || defined(_WIN64)) \
    && defined(GAME_ENGINE_WINDOWS_SUBSYSTEM)
  #include <Windows.h>
int WINAPI wWinMain(_In_ HINSTANCE     hInstance,
                    _In_opt_ HINSTANCE hPrevInstance,
                    _In_ PWSTR         pCmdLine,
                    _In_ int           nCmdShow) {
#else
auto main(int argc, char* argv[]) -> int {

#endif
  // Inform SDL that the program will handle its own initialization
  SDL_SetMainReady();

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
    SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    return EXIT_FAILURE;
  }

  game_engine::Engine engine;

  engine.init();

  engine.run();

  engine.release();

  game_engine::g_rhi_vk->release();

  SDL_Quit();

  return EXIT_SUCCESS;
}
