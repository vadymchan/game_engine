#define SDL_MAIN_HANDLED

#include "engine.h"

using namespace game_engine;

#if (defined(_WIN32) || defined(_WIN64)) \
    && defined(GAME_ENGINE_WINDOWS_SUBSYSTEM)
#include <windows.h>
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

  auto game = std::make_shared<game_engine::Game>();

  engine.setGame(game);

  game->setup();

  engine.run();

  SDL_Quit();

  return EXIT_SUCCESS;
}
