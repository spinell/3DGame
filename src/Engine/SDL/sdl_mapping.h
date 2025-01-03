#pragma once
#include "Engine/Event/Event.h"
#include "Engine/KeyCode.h"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_gamecontroller.h>
#include <SDL2/SDL_keycode.h>

/// @brief Convert a SDL scancode into the engine KeyCode,
Engine::KeyCode fromSDL(SDL_Scancode key);

unsigned convert_sdl_key_mod(SDL_Keymod sdlMod);

/// @brief Convert a SDL mouse button into the engine mouse button,
Engine::MouseButton fromSDL(Uint32 button);

/// @brief Convert a SDL mouse button mask into the engine mouse button mask,
Engine::MouseButtonMask fromSDLButtonMask(Uint32 pButtonMask);

/// @brief Convert a SDL controller button into the engine controller button,
Engine::GameControllerButton fromSDL(SDL_GameControllerButton button);

/// @brief Convert a SDL controller axis into the engine controller axis,
Engine::GameControllerAxis fromSDL(SDL_GameControllerAxis axis);
