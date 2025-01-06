#include "Application.h"

#include "Log.h"

#if USE_SDL == 1
#include "Engine/Platform/SDL2/SDLWindow.h"
#else
#include "Engine/Platform/GLFW/GLFWWindows.h"
#endif

#include <spdlog/spdlog.h>

namespace Engine {

Application::Application() {
    Engine::Log::Initialize();
    ENGINE_CORE_INFO("Engine::Application()");

    WindowCreateInfo windowCreateInfo{};
    windowCreateInfo.fullscreen = false;
#if USE_SDL == 1
    mWindows = new Engine::SDLWindow(windowCreateInfo);
#else
    mWindows = new Engine::GLFWWindows(windowCreateInfo);
#endif

    mWindows->onClosed  = [this](const Engine::WindowCloseEvent& event) { mRunning = false; };
    mWindows->onResized = [](const Engine::WindowResizeEvent& event) {
        ENGINE_CORE_DEBUG(event.toString());
    };
    mWindows->onMoved = [](const Engine::WindowMovedEvent& event) {
        ENGINE_CORE_DEBUG(event.toString());
    };
    mWindows->onVisibilityChanged = [](bool visible) {
        ENGINE_CORE_DEBUG("onVisibilityChanged {}", visible);
    };
    mWindows->onMinimazed  = []() { ENGINE_CORE_DEBUG("onMinimazed"); };
    mWindows->onMaximazed  = []() { ENGINE_CORE_DEBUG("onMaximazed"); };
    mWindows->onRestored   = []() { ENGINE_CORE_DEBUG("onRestored"); };
    mWindows->onFocusGain  = []() { ENGINE_CORE_DEBUG("onFocusGain"); };
    mWindows->onFocusLost  = []() { ENGINE_CORE_DEBUG("onFocusLost"); };

    mWindows->onMousePress = [](const Engine::MousePressEvent& event) {
        ENGINE_CORE_DEBUG(event.toString());
    };
    mWindows->onMouseRelease = [](const Engine::MouseReleaseEvent& event) {
        ENGINE_CORE_DEBUG(event.toString());
    };
    mWindows->onMouseMoved = [](const Engine::MouseMovedEvent& event) {
        // ENGINE_CORE_DEBUG(event.toString());
    };
    mWindows->onMouseScrolled = [](const Engine::MouseScrolledEvent& event) {
        // ENGINE_CORE_DEBUG(event.toString());
    };
    mWindows->onMouseEnterOrLeave = [](const Engine::MouseEnterOrLeaveEvent& event) {
        ENGINE_CORE_DEBUG(event.toString());
    };

    mWindows->onKeyPressed = [this](const Engine::KeyPressedEvent& event) {
        // if (event.getKey() == GLFW_KEY_ESCAPE) {
        //     mRunning = false;
        // }
        // ENGINE_CORE_DEBUG(event.toString());
    };
    mWindows->onKeyReleased = [](const Engine::KeyReleaseEvent& event) {
        // ENGINE_CORE_DEBUG(event.toString());
    };
}

Application::~Application() {
    ENGINE_CORE_INFO("Engine::~Application()");
    delete mWindows;
    Engine::Log::Shutdown();
}

int Application::run() {
    onInit();

    while (mRunning) {
        processEvent();
    }

    onShutdown();
    return 0;
}

void Application::close() { mRunning = false; }

void Application::processEvent() {
#if USE_SDL == 1
    mWindows->pollEvent();
#else
    glfwPollEvents();
#endif
}

} // namespace Engine
