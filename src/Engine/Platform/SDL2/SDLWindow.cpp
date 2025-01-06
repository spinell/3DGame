#include "SDLWindow.h"

#include "assert.h"

#include "Engine/Event.h"
#include "Engine/KeyCode.h"
#include "Engine/Log.h"
#include "Engine/Platform/SDL2/SDLHelper.h"

#include <SDL2/SDL.h>

namespace Engine {

const char* kWindowUserDataKey = "SDLWindow";

int eventFilter(void* userdata, SDL_Event* event) {
    switch (event->type) {
        case SDL_MOUSEMOTION: {
        } break;
        case SDL_MOUSEBUTTONDOWN:
            break;
        case SDL_MOUSEBUTTONUP:
            break;
        default:
            break;
    }

    // If the filter function returns 1 when called, then the event will be added to the internal
    // queue. If it returns 0, then the event will be dropped from the queue, but the internal state
    // will still be updated. This allows selective filtering of dynamically arriving events.
    return 1;
}

SDLWindow::SDLWindow(const WindowCreateInfo& createInfo) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        ENGINE_CORE_CRITICAL("Failed to initialize SDL2 : {}", SDL_GetError());
    }

    SDL_SetEventFilter(eventFilter, nullptr);

    // callback to be triggered when an event is added to the event queue.
    SDL_AddEventWatch(
        [](void* userdata, SDL_Event* event) {
            SDLWindow* win = (SDLWindow*)userdata;

            switch (event->type) {
                case SDL_WINDOWEVENT: {
                    const SDL_WindowEvent& winEvent = event->window;
                    win->onWindowEvent(winEvent);
                    break;
                }
                case SDL_MOUSEMOTION: {
                    const SDL_MouseMotionEvent& motion = event->motion;
                    Engine::MouseButtonFlag     mouseButtonFlag{}; // FIXME: Add MouseButtonFlag
                    win->onMouseMoved(Engine::MouseMovedEvent(motion.x, motion.y, mouseButtonFlag));
                    break;
                }
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP: {
                    const SDL_MouseButtonEvent& buttonEvent = event->button;

                    const Engine::MouseButton button =
                        SDLHelper::ConvertMouseButton(buttonEvent.button);
                    const double             x      = buttonEvent.x;
                    const double             y      = buttonEvent.y;
                    const Engine::KeyModFlag KeyMod = SDLHelper::CaptureKeyModifier();

                    if (buttonEvent.type == SDL_MOUSEBUTTONDOWN) {
                        win->onMousePress(Engine::MousePressEvent(button, x, y, KeyMod));
                    } else if (buttonEvent.type == SDL_MOUSEBUTTONUP) {
                        win->onMouseRelease(Engine::MouseReleaseEvent(button, x, y, KeyMod));
                    }
                    break;
                }
                default:
                    break;
            }
            return 0;
        },
        this);

    mWindow =
        SDL_CreateWindow(createInfo.title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         createInfo.width, createInfo.height, SDL_WINDOW_RESIZABLE);
    SDL_SetWindowData(mWindow, kWindowUserDataKey, this);
}

SDLWindow::~SDLWindow() {}

void SDLWindow::pollEvent() {
    SDL_Event event{};
    while (SDL_PollEvent(&event)) {
    }
}

void SDLWindow::onEvent(const SDL_MouseMotionEvent& e) {}

void SDLWindow::onWindowEvent(const SDL_WindowEvent& event) {
    switch (event.event) {
        case SDL_WINDOWEVENT_CLOSE:
            if (onClosed) {
                onClosed(Engine::WindowCloseEvent());            
            }
            break;
        case SDL_WINDOWEVENT_ENTER:
        case SDL_WINDOWEVENT_LEAVE: {
            if (onMouseEnterOrLeave) {
                const bool mouseEntered = event.event == SDL_WINDOWEVENT_ENTER;
                onMouseEnterOrLeave(Engine::MouseEnterOrLeaveEvent(mouseEntered));
            }
            break;
        }
        case SDL_WINDOWEVENT_DISPLAY_CHANGED:
            // TODO
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
        case SDL_WINDOWEVENT_FOCUS_LOST: {
            if (onFocusGain && event.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                onFocusGain();
            }
            if (onFocusLost && event.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                onFocusLost();
            }
            break;
        }
        case SDL_WINDOWEVENT_SHOWN:
        case SDL_WINDOWEVENT_HIDDEN: {
            if (onVisibilityChanged) {
                const bool isVisible = event.event == SDL_WINDOWEVENT_SHOWN;
                onVisibilityChanged(isVisible);
            }
            break;
        }
        case SDL_WINDOWEVENT_RESIZED: {
            if (onResized) {
                const int width  = event.data1;
                const int height = event.data2;
                onResized(Engine::WindowResizeEvent(width, height));
            }
            break;
        }                                    
        case SDL_WINDOWEVENT_MAXIMIZED: {
            if (onMaximazed) {
                onMaximazed();            
            }
            break;
        }
        case SDL_WINDOWEVENT_MINIMIZED: {
            if (onMinimazed) {
                onMinimazed();            
            }
            break;
        }
        case SDL_WINDOWEVENT_RESTORED: {
            if (onRestored) {
                onRestored();            
            }
        }
            break;
    }
}

} // namespace Engine
