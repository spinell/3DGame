#pragma once

#include <Engine/Event.h>
#include <Engine/LayerStack.h>
#include <Engine/SDL3/SDL3Window.h>

namespace Engine {

class ImGuiLayer;

class Application {
public:
    Application();
    virtual ~Application();

    Application(const Application&) = delete;
    Application(Application&&)      = delete;

    Application& operator=(const Application&) = delete;
    Application& operator=(Application&&)      = delete;

    static Application& Get() { return *sInstance; }
    SDL3Window&         GetWindow() { return *mWindow; }

    int  run();
    void close();

    /// @brief
    /// @param layer
    void pushLayer(Layer* layer);

    /// @brief
    /// @param overlay
    void pushOverlay(Layer* overlay);

    /// @brief
    /// @param layer
    void popLayer(Layer* layer);

    /// @brief
    /// @param overlay
    void popOverlay(Layer* layer);

protected:
    virtual void onInit() {}
    virtual void onShutdown() {}
    virtual void onUpdate() {}
    virtual void onEvent(const Event& event);

private:
    void processEvent();

private:
    static Application* sInstance;
    bool                mRunning = true;
    SDL3Window*         mWindow{};
    ImGuiLayer*         mImGuiLayer{};
    LayerStack          mLayerStack{};
};

} // namespace Engine
