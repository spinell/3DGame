#pragma once

class SDLWindow;

namespace Engine {

class Event;

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
    virtual void onEvent(const Engine::Event& event);

private:
    void processEvent();

private:
    bool       mRunning = true;
    SDLWindow* mWindow{};
};

} // namespace Engine
