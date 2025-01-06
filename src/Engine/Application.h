#pragma once

#define USE_SDL 1

#if USE_SDL == 1
#include "Engine/Platform/SDL2/SDLWindow.h"
#else
#include "Engine/Platform/GLFW/GLFWWindows.h"
#endif

namespace Engine {

class Application {
public:
    Application();
    virtual ~Application();

    Application(const Application&) = delete;
    Application(Application&&)      = delete;

    Application& operator=(const Application&) = delete;
    Application& operator=(Application&&)      = delete;

    int  run();
    void close();

protected:
    virtual void onInit() {}
    virtual void onShutdown() {}
    virtual void onUpdate() {}
    virtual void onEvent() {}

private:
    void processEvent();

private:
    bool mRunning = true;
#if USE_SDL == 1
    SDLWindow* mWindows{};
#else
    GLFWWindows* mWindows{};
#endif
};

} // namespace Engine
