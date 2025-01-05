#include "GLFWWindows.h"

#include "GLFWHelper.h"
#include "assert.h"

#include "Engine/Event.h"
#include "Engine/KeyCode.h"
#include "Engine/Log.h"

namespace {
void glgw_error_callback(int code, const char* description) {
    ENGINE_CORE_ERROR("GLFW has trigger a error. Error code {} : {}", code, description);
}

Engine::GLFWWindows* getUserData(GLFWwindow* win) {
    return static_cast<Engine::GLFWWindows*>(glfwGetWindowUserPointer(win));
}

} // namespace

namespace Engine {

GLFWWindows::GLFWWindows(const WindowCreateInfo& createInfo) {
    // Additional calls to this function after successful initialization
    // but before termination will return GLFW_TRUE immediately.
    if (glfwInit() == GLFW_FALSE) {
        ENGINE_CORE_CRITICAL("");
    }

    glfwSetErrorCallback(glgw_error_callback);

    // glfwWindowHint(GLFW_DECORATED, true);

    ENGINE_INFO("Creating window {0} ({1}, {2})", createInfo.title, createInfo.width,
                createInfo.height);
    
    // disable opengl context creation
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    if (createInfo.fullscreen) {
        GLFWmonitor*       primaryMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode           = glfwGetVideoMode(primaryMonitor);

        // glfwWindowHint(GLFW_DECORATED, false);
        // glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        // glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        // glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        // glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        mWindow = glfwCreateWindow(createInfo.width, createInfo.height, createInfo.title.c_str(),
                                   primaryMonitor, nullptr);
    } else {
        mWindow = glfwCreateWindow(createInfo.width, createInfo.height, createInfo.title.c_str(),
                                   nullptr, nullptr);
    }

    // When this input mode is enabled, any callback that receives modifier bits will have the
    // GLFW_MOD_CAPS_LOCK bit set if Caps Lock was on when the event occurred
    // and the GLFW_MOD_NUM_LOCK bit set if Num Lock was on.
    glfwSetInputMode(mWindow, GLFW_LOCK_KEY_MODS, GLFW_TRUE);

    // set this object as user data of this window
    // so you can retrive the window inside event callback.
    glfwSetWindowUserPointer(mWindow, this);

    registerWindowEventCallback();
    registerKeyboardEventCallback();
    registerMouseEventCallback();
}

GLFWWindows::~GLFWWindows() {
    // When a full screen window is destroyed, the original video mode of its monitor is restored,
    // but the gamma ramp is left untouched.
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

void GLFWWindows::registerWindowEventCallback() {
    // close event
    glfwSetWindowCloseCallback(mWindow, [](GLFWwindow* window) {
        GLFWWindows* win = getUserData(window);
        if (win->onClosed) win->onClosed(WindowCloseEvent());
    });

    // window resize
    glfwSetWindowSizeCallback(mWindow, [](GLFWwindow* window, int width, int height) {
        GLFWWindows* win = getUserData(window);
        if (win->onResized) win->onResized(WindowResizeEvent(width, height));
    });

    // window content scale
    glfwSetWindowContentScaleCallback(mWindow, [](GLFWwindow* window, float xScale, float yScale) {
        // ENGINE_CORE_DEBUG("Content Scale Callback");
    });

    // window position
    glfwSetWindowPosCallback(mWindow, [](GLFWwindow* window, int xPos, int yPos) {
        GLFWWindows* win = getUserData(window);
        if (win->onMoved) win->onMoved(WindowMovedEvent(xPos, yPos));
    });

    // iconnification (minimized / restore)
    glfwSetWindowIconifyCallback(mWindow, [](GLFWwindow* window, int iconified) {
        GLFWWindows* win = getUserData(window);
        if (iconified) {
            if (win->onMinimazed) win->onMinimazed();
        } else {
            if (win->onRestored) win->onRestored();
        }
    });

    // maximization (maximized / restore)
    glfwSetWindowMaximizeCallback(mWindow, [](GLFWwindow* window, int maximized) {
        GLFWWindows* win = getUserData(window);
        if (maximized) {
            if (win->onMinimazed) win->onMinimazed();
        } else {
            if (win->onRestored) win->onRestored();
        }
    });

    // windows focus
    glfwSetWindowFocusCallback(mWindow, [](GLFWwindow* window, int focused) {
        GLFWWindows* win = getUserData(window);
        if (focused) {
            if (win->onFocusGain) win->onFocusGain();
        } else {
            if (win->onFocusLost) win->onFocusLost();
        }
    });

    // windows need to be redraw
    glfwSetWindowRefreshCallback(mWindow, [](GLFWwindow* window) {
        // ENGINE_CORE_DEBUG("paint");
    });
}

void GLFWWindows::registerKeyboardEventCallback() {
    //
    // Keyboard key callback (key press, key release, key repeat)
    //
    glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int key, int scancode, int action,
                                   int mods) {
        const Engine::KeyModFlag keyModFlag = Engine::GLFWHelper::ConvertKeyModKer(window, mods);

        switch (action) {
            case GLFW_PRESS: {
                GLFWWindows* win = getUserData(window);
                if (win->onKeyPressed)
                    win->onKeyPressed(
                        Engine::KeyPressedEvent(key, scancode, false /*isRepeat*/, keyModFlag));
                const auto name = glfwGetKeyName(key, scancode);
                const auto name2 = glfwGetKeyName(GLFW_KEY_UNKNOWN, scancode);
                ENGINE_CORE_INFO("{} {} {} {}", key, name ? name : "No name", scancode, name2 ? name2 : "No name");
                break;
            }
            case GLFW_REPEAT: {
                GLFWWindows* win = getUserData(window);
                if (win->onKeyPressed)
                    win->onKeyPressed(
                        Engine::KeyPressedEvent(key, scancode, true /*isRepeat*/, keyModFlag));
                break;
            }
            case GLFW_RELEASE: {
                GLFWWindows* win = getUserData(window);
                if (win->onKeyReleased)
                    win->onKeyReleased(Engine::KeyReleaseEvent(key, scancode, keyModFlag));
                break;
            }
            default:
                ENGINE_CORE_DEBUG("GLFW has trigger a unknow key callback action.");
        }
    });

    //
    // Keyboard text event
    //
    glfwSetCharCallback(mWindow, [](GLFWwindow* window, unsigned int codepoint) {
        GLFWWindows* win = getUserData(window);
        // FIXME: don't cast the codepoint to char
        if (win->onTextChar) win->onTextChar((char)codepoint);
    });
}

void GLFWWindows::registerMouseEventCallback() {
    ///
    /// Mouse moved
    ///
    glfwSetCursorPosCallback(mWindow, [](GLFWwindow* window, double xpos, double ypos) {
        GLFWWindows*                  win             = getUserData(window);
        const Engine::MouseButtonFlag mouseButtonFlag = GLFWHelper::GetMouseButtonMask(window);
        win->onMouseMoved(MouseMovedEvent(xpos, ypos, mouseButtonFlag));
    });

    //
    // Mouse button press / release
    //
    // Note: GLWF does not support double click.
    //
    glfwSetMouseButtonCallback(mWindow, [](GLFWwindow* window, int button, int action, int mods) {
        GLFWWindows* win = getUserData(window);

        double mouseX{};
        double mouseY{};
        glfwGetCursorPos(window, &mouseX, &mouseY);

        switch (action) {
            case GLFW_PRESS: {
                const Engine::MouseButton convertedButton = GLFWHelper::ConvertMouseButton(button);
                const Engine::KeyModFlag  convertedModifierFlags =
                    GLFWHelper::ConvertKeyModKer(window, mods);

                win->onMousePress(
                    MousePressEvent(convertedButton, mouseX, mouseY, convertedModifierFlags));
                break;
            }
            case GLFW_RELEASE: {
                const Engine::MouseButton convertedButton = GLFWHelper::ConvertMouseButton(button);
                const Engine::KeyModFlag  convertedModifierFlags =
                    GLFWHelper::ConvertKeyModKer(window, mods);
                win->onMouseRelease(
                    MouseReleaseEvent(convertedButton, mouseX, mouseY, convertedModifierFlags));
                break;
            }
            default:
                ENGINE_CORE_DEBUG("GLFW has trigger a unknow mouse button callback action.");
        }
    });

    ///
    /// Mouse scroll / wheel
    ///
    glfwSetScrollCallback(mWindow, [](GLFWwindow* window, double xoffset, double yoffset) {
        double mouseX{};
        double mouseY{};
        glfwGetCursorPos(window, &mouseX, &mouseY);

        GLFWWindows* win = getUserData(window);
        win->onMouseScrolled(MouseScrolledEvent(xoffset, yoffset, mouseX, mouseY));
    });

    // windows need to be redraw
    glfwSetCursorEnterCallback(mWindow, [](GLFWwindow* window, int entered) {
        GLFWWindows* win = getUserData(window);
        win->onMouseEnterOrLeave(MouseEnterOrLeaveEvent(entered));
    });
}

} // namespace Engine
