#ifndef GAME_ENGINE_EVENT_H
#define GAME_ENGINE_EVENT_H

#include <SDL.h>

namespace game_engine {

using EventType = SDL_EventType;
using Event     = SDL_Event;

using WindowEvent     = SDL_WindowEvent;
using WindowEventType = Uint8;

using KeyboardEvent = SDL_KeyboardEvent;

using MouseButtonEvent = SDL_MouseButtonEvent;
using MouseMotionEvent = SDL_MouseMotionEvent;
using MouseWheelEvent  = SDL_MouseWheelEvent;

using ApplicationEvent = SDL_QuitEvent; // for now only quit event
using ApplicationEventType = Uint32;

}  // namespace game_engine

#endif  // GAME_ENGINE_EVENT_H