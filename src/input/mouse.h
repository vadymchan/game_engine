#ifndef GAME_ENGINE_MOUSE_H
#define GAME_ENGINE_MOUSE_H

#include <SDL.h>

namespace game_engine {

/**
 * @typedef MouseButton
 * @brief Defines the type for mouse button identifiers.
 *
 * Possible constants include:
 * - SDL_BUTTON_LEFT: Left mouse button
 * - SDL_BUTTON_MIDDLE: Middle mouse button
 * - SDL_BUTTON_RIGHT: Right mouse button
 * - SDL_BUTTON_X1: First extra mouse button
 * - SDL_BUTTON_X2: Second extra mouse button
 */
using MouseButton = Uint8;

/**
 * @typedef MouseMotionState
 * @brief Defines the type for mouse motion state identifiers.
 *
 * Possible constants include:
 * - SDL_BUTTON_LMASK: Left mouse button mask
 * - SDL_BUTTON_MMASK: Middle mouse button mask
 * - SDL_BUTTON_RMASK: Right mouse button mask
 * - SDL_BUTTON_X1MASK: First extra mouse button mask
 * - SDL_BUTTON_X2MASK: Second extra mouse button mask
 */
using MouseMotionState = Uint32;

}  // namespace game_engine

#endif  // GAME_ENGINE_MOUSE_H