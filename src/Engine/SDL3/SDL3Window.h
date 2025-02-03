#pragma once
#include <string>
#include <cstdint>

struct SDL_Window;
struct SDL_Renderer;

namespace Engine {

struct WindowCreateInfo {
    std::string title      = "Engine";
    uint32_t    width      = 1600;
    uint32_t    height     = 900;
    bool        decorated  = true;
    bool        fullscreen = false;
};

class SDL3Window {
public:
    SDL3Window(const WindowCreateInfo& createInfo = WindowCreateInfo());
    ~SDL3Window();

    SDL_Window*   getSDLWindow() const { return mWindow; };
    SDL_Renderer* getRenderer() const { return mRenderer; };

    void setFullScreen(bool fullscreen);
    void setMouseGrab(bool grab);
    void toogleMouseGrab();
    void toogleMouseRelativeMode();

private:
    SDL_Window*   mWindow{};
    SDL_Renderer* mRenderer{};
};

} // namespace Engine
