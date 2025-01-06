#pragma once
#include <Engine/KeyCode.h>
#include <SDL2/SDL.h>

namespace Engine {

struct SDLHelper {
    /// @brief Convert SDL2 mouse button
    /// @param  button A GLFW mouse button (SDL_BUTTON_LEFT, SDL_BUTTON_X1, ...).
    /// @return        The converted mouse button.
    static Engine::MouseButton ConvertMouseButton(int button);

    /// @param
    ///
    ///
    /// @return
    static Engine::KeyModFlag CaptureKeyModifier();
};

} // namespace Engine
