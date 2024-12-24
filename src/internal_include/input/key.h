#ifndef GAME_ENGINE_KEY_H
#define GAME_ENGINE_KEY_H

#include <SDL.h>

namespace game_engine {

using PhysicalKey = SDL_Scancode;
using VirtualKey  = SDL_Keycode;

// TODO: why not use SDL_EventType?
using KeyType = Uint32;  // SDL_KEYDOWN or SDL_KEYUP

}  // namespace game_engine

#endif  // GAME_ENGINE_KEY_H
