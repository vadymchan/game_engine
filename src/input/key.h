#ifndef ARISE_KEY_H
#define ARISE_KEY_H

#include <SDL.h>

namespace arise {

using PhysicalKey = SDL_Scancode;
using VirtualKey  = SDL_Keycode;

// TODO: why not use SDL_EventType?
using KeyType = Uint32;  // SDL_KEYDOWN or SDL_KEYUP

}  // namespace arise

#endif  // ARISE_KEY_H
