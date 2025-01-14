#pragma once
#include <Engine/KeyCode.h>

namespace Engine {
class Event;

/// @brief Interface to query input device state
///
/// @note Internal only. The \p Update() function need to be called once per frame.
/// @note Internal only. The \p OnEvent() function need to be called once per frame.
///
/// @todo Add support for joystick / gamepad
///
class Input {
public:
    /// @brief This function check is the given key \p key has been pressed this frame.
    /// @param key The key to check.
    /// @return True if key \p key has been pressed this frame.
    static bool IsKeyPressed(KeyCode key);

    /// @brief This function check is the given key \p key is currently pressed.
    /// @param key The key to check.
    /// @return True if key \p key is currently pressed.
    static bool IsKeyDown(KeyCode key);

    /// @brief This function check is the given key \p key has been released this frame.
    /// @param key The key to check.
    /// @return True if key \p key has been released this frame.
    static bool IsKeyReleased(KeyCode key);

    /// @brief This function check is the given mouse \p button has been pressed this frame.
    /// @param key The mouse button to check.
    /// @return True if key \p button has been pressed this frame.
    static bool IsButtonPressed(MouseButton button);

    /// @brief This function check is the given mouse \p button is currently pressed.
    /// @param key The mouse button to check.
    /// @return True if mouse \p button is currently pressed.
    static bool IsButtonDown(MouseButton button);

    /// @brief This function check is the mouse \p mouse has been released this frame.
    /// @param key The mouse button to check.
    /// @return True if mouse \p button has been released this frame.
    static bool IsButtonReleased(MouseButton button);

    /// @brief Return the mouse position in pixel coordinate.
    ///        The top-left of the screen or window is at (0, 0).
    ///        The bottom-right of the screen or window is at (width,height)
    static std::pair<float, float> GetMousePosition();

    /// @brief Return the current mouse position delta in pixel coordinates.
    ///        The top-left of the screen or window is at (0, 0).
    ///        The bottom-right of the screen or window is at (width,height).
    ///        If no mouse mouvement happen during this frame, 0 is returned.
    static std::pair<float, float> GetMousePositionDelta();

    /// @brief Return the amount of mouse scroll (wheel) during this frame.
    ///        Horizontal positive to the right and negative to the left.
    ///        Vertical positive away from the user and negative toward the user
    ///        If no scroll happen during this frame, 0 is returned.
    static std::pair<float, float> GetMouseScrollDelta();

    /// @brief Update the internal input state.
    ///        This must be called once per frame.
    /// @remark Internal only.
    static void Update();

    /// @brief Update the states of the inputs.
    ///        This should be called during processing events.
    /// @param event The event to process.
    /// @remark Internal only.
    static void OnEvent(const Engine::Event& event);
};

} // namespace Engine
