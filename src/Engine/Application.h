#pragma once
#include <Engine/Platform/GLFW/GLFWWindows.h>

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
    bool         mRunning = true;
    GLFWWindows* mWindows{};
};

} // namespace Engine
