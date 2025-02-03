#include "Application.h"

#include "Log.h"

#include <Engine/Event.h>
#include <Engine/ImGuiLayer.h>
#include <Engine/Input.h>
#include <Engine/KeyCode.h>
#include <Engine/SDL3/SDL3Helper.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <imgui.h>

#define ENABLE_IMGUI

namespace {
void printSDL2Info() {
    const int compiled = SDL_VERSION;      // hardcoded number from SDL headers
    const int linked   = SDL_GetVersion(); // reported by linked SDL library

    ENGINE_CORE_INFO("SDL compiled version: {}.{}.{}.", SDL_VERSIONNUM_MAJOR(compiled),
                     SDL_VERSIONNUM_MINOR(compiled), SDL_VERSIONNUM_MICRO(compiled));
    ENGINE_CORE_INFO("SDL runtime version : {}.{}.{}.", SDL_VERSIONNUM_MAJOR(linked),
                     SDL_VERSIONNUM_MINOR(linked), SDL_VERSIONNUM_MICRO(linked));

    ENGINE_CORE_INFO("SDL has found {} audio driver.", SDL_GetNumAudioDrivers());
    for (int i = 0; i < SDL_GetNumAudioDrivers(); i++) {
        ENGINE_CORE_INFO(" - {}", SDL_GetAudioDriver(i));
    }

    ENGINE_CORE_INFO("SDL has found {} video driver.", SDL_GetNumVideoDrivers());
    for (int i = 0; i < SDL_GetNumVideoDrivers(); i++) {
        ENGINE_CORE_INFO(" - {}", SDL_GetVideoDriver(i));
    }

    ENGINE_CORE_INFO("SDL has found {} renderer driver.", SDL_GetNumRenderDrivers());
    for (int i = 0; i < SDL_GetNumRenderDrivers(); i++) {
        ENGINE_CORE_INFO(" - {}", SDL_GetRenderDriver(i));
    }

    ENGINE_CORE_INFO("SDL has found {} GPPU driver.", SDL_GetNumGPUDrivers());
    for (int i = 0; i < SDL_GetNumGPUDrivers(); i++) {
        ENGINE_CORE_INFO(" - {}", SDL_GetGPUDriver(i));
    }
}
} // namespace

Engine::Application* Engine::Application::sInstance = nullptr;

Engine::Application::Application() {
    assert(!sInstance && "Only one instance allowd !");
    sInstance = this;

    Engine::Log::Initialize();
    ENGINE_CORE_INFO("Engine::Application()");

    printSDL2Info();

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC |
                  SDL_INIT_GAMEPAD)) {
        ENGINE_CORE_CRITICAL("Failed to initialize SDL2 : {}", SDL_GetError());
        return;
    }

    mWindow     = new SDL3Window();
#ifdef DISABLE_IMGUI
    mImGuiLayer = new ImGuiLayer();
    pushOverlay(mImGuiLayer);
#endif
}

Engine::Application::~Application() {
    delete mWindow;
    SDL_Quit();

    ENGINE_CORE_INFO("Engine::~Application()");
    Engine::Log::Shutdown();
}

void Engine::Application::pushLayer(Layer* layer) {
    mLayerStack.pushLayer(layer);
    layer->onAttach();
}

void Engine::Application::pushOverlay(Layer* overlay) {
    mLayerStack.pushOverlay(overlay);
    overlay->onAttach();
}

void Engine::Application::popLayer(Layer* layer) {
    mLayerStack.popLayer(layer);
    layer->onDetach();
}

void Engine::Application::popOverlay(Layer* overlay) {
    mLayerStack.popOverlay(overlay);
    overlay->onDetach();
}

int Engine::Application::run() {
    onInit();

    auto frameStart = std::chrono::high_resolution_clock::now();
    while (mRunning) {
        // compute delta time since last frame
        auto   frameEnd      = std::chrono::high_resolution_clock::now();
        double frameDuration = std::chrono::duration<double>(frameEnd - frameStart).count();
        frameStart           = frameEnd;

        // update input state and process event
        Engine::Input::Update();
        processEvent();

        // Update each layer
        for (Layer* layer : mLayerStack) {
            layer->onUpdate(frameDuration);
        }

        // Update Imgui
#ifdef DISABLE_IMGUI
        mImGuiLayer->begin();
        for (Layer* layer : mLayerStack) {
            layer->onImGuiRender();
        }
        mImGuiLayer->end();
#endif
        // TODO: Remove this when vulkan renderer is ready
        SDL_GL_SwapWindow(mWindow->getSDLWindow());
    }

    onShutdown();

    return 0;
}

void Engine::Application::close() { mRunning = false; }

void Engine::Application::onEvent(const Event& event) {
    Engine::Input::OnEvent(event);

    // if Imgui use the keyboard or the mouse, don't propagate event to layers
#ifdef DISABLE_IMGUI
    if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
        return;
    }
#endif
    for (auto it = mLayerStack.rbegin(); it != mLayerStack.rend(); ++it) {
        if ((*it)->onEvent(event)) {
            // The layer has handle the event,
            // stop dispaching the event to other layers.
            break;
        }
    }
}

void Engine::Application::processEvent() {
    SDL_Event event;

    // Events management
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            mRunning = false;
        }
        //
        // Window
        //
        if (event.type >= SDL_EVENT_WINDOW_FIRST && event.type <= SDL_EVENT_WINDOW_LAST) {
            std::optional<Event> e = SDL3Helper::ConvertEvent(event.window);
            if (e.has_value()) {
                onEvent(e.value());
            }
        }
        //
        // Keyboard
        //
        else if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
            onEvent(SDL3Helper::ConvertEvent(event.key));
        }
        //
        // Mouse event
        //
        else if (event.type == SDL_EVENT_MOUSE_MOTION) {
            onEvent(SDL3Helper::ConvertEvent(event.motion));
        } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN ||
                   event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            onEvent(SDL3Helper::ConvertEvent(event.button));
        } else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
            onEvent(SDL3Helper::ConvertEvent(event.wheel));
        }
    }
}
