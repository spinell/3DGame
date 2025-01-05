#include <Engine/Platform/GLFW/GLFWHelper.h>

namespace Engine {

Engine::KeyModFlag GLFWHelper::ConvertKeyModKer(GLFWwindow* window, int mods) {
    Engine::KeyModFlag keyModFlag{};

    // shift modifier
    if (mods & GLFW_MOD_SHIFT) {
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            keyModFlag |= Engine::KeyModFlag::LeftShift;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
            keyModFlag |= Engine::KeyModFlag::RightShift;
        }
    }
    // ctrl modifier
    if (mods & GLFW_MOD_CONTROL) {
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            keyModFlag |= Engine::KeyModFlag::LeftCtrl;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
            keyModFlag |= Engine::KeyModFlag::RightCtrl;
        }
    }
    // alt modifier
    if (mods & GLFW_MOD_ALT) {
        if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) {
            keyModFlag |= Engine::KeyModFlag::LeftAlt;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS) {
            keyModFlag |= Engine::KeyModFlag::RightAlt;
        }
    }
    // caps lock modifier
    if (mods & GLFW_MOD_CAPS_LOCK) {
        keyModFlag |= Engine::KeyModFlag::CapsLock;
    }
    // caps lock modifier
    if (mods & GLFW_MOD_NUM_LOCK) {
        keyModFlag |= Engine::KeyModFlag::NumLock;
    }
    // super/windows key lock modifier
    if (mods & GLFW_MOD_SUPER) {
        if (glfwGetKey(window, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS) {
            keyModFlag |= Engine::KeyModFlag::LeftGUI;
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS) {
            keyModFlag |= Engine::KeyModFlag::RightGUI;
        }
    }

    return keyModFlag;
}

Engine::MouseButton GLFWHelper::ConvertMouseButton(int button) {
    switch (button) {
            // clang-format off
        case GLFW_MOUSE_BUTTON_LEFT:   return Engine::MouseButton::Left;
        case GLFW_MOUSE_BUTTON_RIGHT:  return Engine::MouseButton::Right;
        case GLFW_MOUSE_BUTTON_MIDDLE: return Engine::MouseButton::Middle;
        case GLFW_MOUSE_BUTTON_4:      return Engine::MouseButton::X4;
        case GLFW_MOUSE_BUTTON_5:      return Engine::MouseButton::X5;
        case GLFW_MOUSE_BUTTON_6:      return Engine::MouseButton::X6;
        case GLFW_MOUSE_BUTTON_7:      return Engine::MouseButton::X7;
        case GLFW_MOUSE_BUTTON_8:      return Engine::MouseButton::X8;
        default:                       return Engine::MouseButton::Unknow;
            // clang-format on
    }
}

Engine::MouseButtonFlag GLFWHelper::GetMouseButtonMask(GLFWwindow* window) {
    Engine::MouseButtonFlag mask{};

    for (unsigned i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++) {
        if (glfwGetMouseButton(window, i) == GLFW_PRESS) {
            Engine::MouseButton button = ConvertMouseButton(i);
            const unsigned      bit    = 1 << static_cast<unsigned short>(button);
            mask |= static_cast<Engine::MouseButtonFlag>(bit);
        }
    }

    return mask;
}

} // namespace Engine
