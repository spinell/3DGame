#pragma once
#include <Engine/Event.h>

#include <functional>

struct SDL_Window;
struct SDL_MouseMotionEvent;
struct SDL_WindowEvent;

namespace Engine {

struct WindowCreateInfo {
    std::string title      = "Engine";
    uint32_t    width      = 1600;
    uint32_t    height     = 900;
    bool        fullscreen = false;
};

class SDLWindow {
public:
    SDLWindow(const WindowCreateInfo& createInfo = WindowCreateInfo());
    ~SDLWindow();

    // Windows Event
    std::function<void(const Engine::WindowCloseEvent& event)>  onClosed;
    std::function<void(const Engine::WindowResizeEvent& event)> onResized;
    std::function<void()>                                       onMinimazed;
    std::function<void()>                                       onMaximazed;
    std::function<void()>                                       onRestored;
    std::function<void()>                                       onFocusGain;
    std::function<void()>                                       onFocusLost;
    std::function<void(const Engine::WindowMovedEvent& event)>  onMoved;
    std::function<void(bool)>                                   onVisibilityChanged;
    // Mouse Event
    std::function<void(const Engine::MousePressEvent& event)>        onMousePress;
    std::function<void(const Engine::MouseReleaseEvent& event)>      onMouseRelease;
    std::function<void(const Engine::MouseMovedEvent& event)>        onMouseMoved;
    std::function<void(const Engine::MouseScrolledEvent& event)>     onMouseScrolled;
    std::function<void(const Engine::MouseEnterOrLeaveEvent& event)> onMouseEnterOrLeave;

    // Keyboard Event
    std::function<void(const Engine::KeyPressedEvent& event)> onKeyPressed;
    std::function<void(const Engine::KeyReleaseEvent& event)> onKeyReleased;
    std::function<void(char c)>                               onTextChar;

    void pollEvent();

private:
    void onEvent(const SDL_MouseMotionEvent& e);
    void onWindowEvent(const SDL_WindowEvent& e);

    SDL_Window* mWindow{};
};

} // namespace Engine
