#pragma once
#include <Engine/Event.h>
#include <Engine/KeyCode.h>
#include <SDL3/SDL.h>

#include <optional>

namespace Engine {

/// @brief Helper class for doing SDL3 <-> Engine data conversion.
struct SDL3Helper {
    /// @brief Convert SDL2 mouse button
    /// @param  button A GLFW mouse button (SDL_BUTTON_LEFT, SDL_BUTTON_X1, ...).
    /// @return        The converted mouse button.
    static MouseButton ConvertMouseButton(int button);

    /// @brief Convert a SDL scancode.
    /// @param scancode The scancode to convert.
    /// @return The converted scancode or \p KeyCode::Unknown if the key is not suppported.
    static KeyCode ConvertScanCode(SDL_Scancode scancode);

    static MouseButtonFlag ConvertMouseButtonMask(SDL_MouseButtonFlags buttonMask);
    static KeyModFlag      ConvertKeyModifierMask(SDL_Keymod keymod);

    /// @param
    ///
    ///
    /// @return
    // static Engine::KeyModFlag CaptureKeyModifier();

    static MouseScrolledEvent   ConvertEvent(const SDL_MouseWheelEvent& event);
    static MouseMovedEvent      ConvertEvent(const SDL_MouseMotionEvent& event);
    static MouseButtonEvent     ConvertEvent(const SDL_MouseButtonEvent& event);
    static KeyEvent             ConvertEvent(const SDL_KeyboardEvent& event);
    static std::optional<Event> ConvertEvent(const SDL_WindowEvent& event);
};

} // namespace Engine
