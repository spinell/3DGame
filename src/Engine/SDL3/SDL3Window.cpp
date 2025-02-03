#include <Engine/Log.h>
#include <Engine/SDL3/SDL3Window.h>
#include <SDL3/SDL.h>

namespace {
void CheckSDLError(const std::string_view& action) {
    if (const char* error = SDL_GetError(); *error != '\0') {
        ENGINE_CORE_ERROR("[{}] SDL Error: {}", action, error);
        SDL_ClearError();
    }
}

}; // namespace

namespace Engine {

SDL3Window::SDL3Window(const WindowCreateInfo& createInfo) {
    SDL_SetHint(SDL_HINT_IME_IMPLEMENTED_UI, "1");
    mWindow = SDL_CreateWindow(createInfo.title.c_str(), createInfo.width, createInfo.height,
                               SDL_WINDOW_RESIZABLE /*| SDL_WINDOW_OPENGL*/);
    CheckSDLError("Creating winodws");

    // TODO: Remove the SDL renderer when the Vulkan renderer is ready.
    //mRenderer = SDL_CreateRenderer(mWindow, nullptr);
    //CheckSDLError("Creating rendering");
    //ENGINE_CORE_INFO("SDL Renderer: {}", SDL_GetRendererName(mRenderer));

    // This function will enable text input
    // (SDL_EVENT_TEXT_INPUT and SDL_EVENT_TEXT_EDITING events) in the specified window.
    SDL_StartTextInput(mWindow);
}

SDL3Window::~SDL3Window() {
    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);
}

void SDL3Window::setFullScreen(bool fulscreen) {
    if (!SDL_SetWindowFullscreen(mWindow, fulscreen)) {
        CheckSDLError("Fullscreen");
    }
}

void SDL3Window::setMouseGrab(bool grab) {
    if (!SDL_SetWindowMouseGrab(mWindow, grab)) {
        CheckSDLError("MouseGrab");
    }
}

void SDL3Window::toogleMouseGrab() {
    const bool grab = SDL_GetWindowMouseGrab(mWindow);
    setMouseGrab(!grab);
}

void SDL3Window::toogleMouseRelativeMode() {
    const bool isRelative = SDL_GetWindowRelativeMouseMode(mWindow);
    if (!SDL_SetWindowRelativeMouseMode(mWindow, !isRelative)) {
        CheckSDLError("MouseRelativeMode");
    }
}

} // namespace Engine
