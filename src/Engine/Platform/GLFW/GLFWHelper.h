#pragma once
#include <Engine/KeyCode.h>
#include <GLFW/glfw3.h>

namespace Engine {

struct GLFWHelper {
    /// @brief Convert a GLFW key modifier
    ///
    /// This function convert GLFW key modifier into the Engine key modifier.
    /// GLFW does not report left/right modifier. It just report the modifier.
    ///
    /// For example, the Shift modifier is reported no matter if it the left or
    /// right modifier.
    ///
    /// To work around this, we need to query the state if the left/right
    /// key for a specific windows with glfwGetKey();
    ///
    /// @param window The GLFW window to check for the modifier are pressed.
    /// @param mods   The GLFW key modifier (GLFW_MOD_SHIFT, ...).
    /// @return       The Engine key modifier.
    static Engine::KeyModFlag ConvertKeyModKer(GLFWwindow* window, int mods);

    /// @brief Convert GLFW mouse button
    /// @param  button A GLFW mouse button (GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_1, ...).
    /// @return        The converted mouse button.
    static Engine::MouseButton ConvertMouseButton(int button);

    /// @brief Return a mask of all mouse button which as pressed.
    ///
    /// @param window The GLFW window used to query themouse button state.
    /// return The mouse button mask
    static Engine::MouseButtonFlag GetMouseButtonMask(GLFWwindow* window);
};

} // namespace Engine
