#include "SDLWindow.h"

#include "Engine/Event/DropEvent.h"
#include "Engine/Event/KeyBoardEvent.h"
#include "Engine/Event/MouseEvent.h"
#include "Engine/Event/WindowEvent.h"
#include "Engine/Log.h"
#include "Engine/SDL/sdl_mapping.h"

#include <SDL_syswm.h>

namespace {
const char* kWindowUserDataKey = "SDLWindow";
}

SDLWindow* SDLWindow::GetWindowFromID(Uint32 windowId) {
    SDL_Window* window = SDL_GetWindowFromID(windowId);
    // X_ASSERT(window);

    if (window) {
        void* userData = SDL_GetWindowData(window, kWindowUserDataKey);
        // X_ASSERT(userData);

        if (userData) return static_cast<SDLWindow*>(userData);
    }

    return nullptr;
}

SDL_HitTestResult hitTestCallBack(SDL_Window* win, const SDL_Point* area, void* data) {
    return SDL_HITTEST_NORMAL;
}

SDLWindow::SDLWindow() {}

SDLWindow::~SDLWindow() {
    if (mIconSurface) {
        SDL_FreeSurface(mIconSurface);
    }

    if (mWindow) SDL_DestroyWindow(mWindow);
}

bool SDLWindow::create(const CreateInfo& createInfo) {
    Uint32 windowFlags = 0;
    if (!createInfo.showOnCreation) windowFlags |= SDL_WINDOW_HIDDEN;
    if (createInfo.borderless) windowFlags |= SDL_WINDOW_BORDERLESS;
    if (createInfo.alwaysOnTop) windowFlags |= SDL_WINDOW_ALWAYS_ON_TOP;
    if (createInfo.noTaskBarIcon) windowFlags |= SDL_WINDOW_SKIP_TASKBAR;
    if (createInfo.resizable) windowFlags |= SDL_WINDOW_RESIZABLE;

    mWindow = SDL_CreateWindow(createInfo.title, createInfo.posX, createInfo.posY, createInfo.width,
                               createInfo.height, windowFlags);
    if (!mWindow) {
        const char* msg = SDL_GetError();
        if (msg) {
        }
    }

    SDL_SetWindowHitTest(mWindow, hitTestCallBack, nullptr);

    SDL_SetWindowData(mWindow, kWindowUserDataKey, this);

    mWindowID = SDL_GetWindowID(mWindow);

    return true;
}

void SDLWindow::setGrabInput(bool pGrabInput) {
    if (mWindow) SDL_SetWindowGrab(mWindow, pGrabInput ? SDL_TRUE : SDL_FALSE);
}

bool SDLWindow::isGrabInput() const { return SDL_GetWindowGrab(mWindow) == SDL_TRUE; }

void SDLWindow::toogleGrabInput() { setGrabInput(!isGrabInput()); }

void SDLWindow::show() { SDL_ShowWindow(mWindow); }

void SDLWindow::hide() { SDL_HideWindow(mWindow); }

bool SDLWindow::isVisible() const { return SDL_GetWindowFlags(mWindow) & SDL_WINDOW_SHOWN; }

bool SDLWindow::hasFocus() const { return SDL_GetWindowFlags(mWindow) & SDL_WINDOW_INPUT_FOCUS; }

void SDLWindow::raise() const { SDL_RaiseWindow(mWindow); }

void SDLWindow::setTitle(const char* title) { SDL_SetWindowTitle(mWindow, title); }

void SDLWindow::setIcon(unsigned width, unsigned height, const unsigned* data) {
    if (mIconSurface) {
        SDL_FreeSurface(mIconSurface);
    }

    mIconSurface = SDL_CreateRGBSurfaceFrom(
        (void*)data, width, height, 32 /*depth*/, width * sizeof(unsigned) /*pitch*/,
        0xff000000 /*Rmask*/, 0x00ff0000 /*Gmask*/, 0x0000ff00 /*Bmask*/, 0x000000ff /*Amask*/
    );
    if (!mIconSurface) {
        auto error = SDL_GetError();
        ENGINE_CORE_WARNING("Failed to set window icon. {0}", error);
    } else {
        SDL_SetWindowIcon(mWindow, mIconSurface);
    }
}

void SDLWindow::setOpacity(float opacity) {
    if (SDL_SetWindowOpacity(mWindow, opacity) < 0) {
        auto error = SDL_GetError();
        ENGINE_CORE_WARNING("Failed to set window opacity. {0}", error);
    }
}

void SDLWindow::setMaximumSize(int maxWidth, int maxHeight) {
    SDL_SetWindowMaximumSize(mWindow, maxWidth, maxHeight);
}

void SDLWindow::getMaximumSize(int& pOutMaximalWidth, int& pOutMaximalHeight) const {
    SDL_GetWindowMaximumSize(mWindow, &pOutMaximalWidth, &pOutMaximalHeight);
}

void SDLWindow::setMinimumSize(int minWidth, int minHeight) {
    SDL_SetWindowMinimumSize(mWindow, minWidth, minHeight);
}

void SDLWindow::getMinimumSize(int& pOutMaximalWidth, int& pOutMaximalHeight) const {
    SDL_GetWindowMinimumSize(mWindow, &pOutMaximalWidth, &pOutMaximalHeight);
}

void SDLWindow::setPosition(int x, int y) { SDL_SetWindowPosition(mWindow, x, y); }

void SDLWindow::getPosition(int& x, int& y) { SDL_GetWindowPosition(mWindow, &x, &y); }

void SDLWindow::setSize(int width, int height) { SDL_SetWindowSize(mWindow, width, height); }

void SDLWindow::setResizable(bool resizable) {
    SDL_SetWindowResizable(mWindow, resizable ? SDL_TRUE : SDL_FALSE);
}

void SDLWindow::getSize(int& width, int& height) { SDL_GetWindowSize(mWindow, &width, &height); }

int SDLWindow::getHeight() const {
    int x = 0;
    SDL_GetWindowSize(mWindow, nullptr, &x);
    return x;
}

int SDLWindow::getWidth() const {
    int x = 0;
    SDL_GetWindowSize(mWindow, &x, nullptr);
    return x;
}

const char* SDLWindow::getTitle() const { return SDL_GetWindowTitle(mWindow); }

void SDLWindow::maximize() { SDL_MaximizeWindow(mWindow); }

void SDLWindow::minimize() { SDL_MinimizeWindow(mWindow); }

void SDLWindow::restore() { SDL_RestoreWindow(mWindow); }

bool SDLWindow::isMinimized() const { return SDL_GetWindowFlags(mWindow) & SDL_WINDOW_MINIMIZED; }

bool SDLWindow::isMaximized() const { return SDL_GetWindowFlags(mWindow) & SDL_WINDOW_MAXIMIZED; }

bool SDLWindow::isResizable() const { return SDL_GetWindowFlags(mWindow) & SDL_WINDOW_RESIZABLE; }

void* SDLWindow::getWindow() const { return mWindow; }

void* SDLWindow::getWindowHandle() const {
    void* windowHandle = nullptr;

    if (mWindow) {
        SDL_SysWMinfo wmInfo;
        SDL_VERSION(&wmInfo.version);

        SDL_GetWindowWMInfo(mWindow, &wmInfo);

        if (wmInfo.subsystem == SDL_SYSWM_WINDOWS) {
            windowHandle = wmInfo.info.win.window;
        }
    }
    return windowHandle;
}

void SDLWindow::setFullScreenVideoMode(int pWidth, int pHeight) {
    SDL_DisplayMode mode;
    SDL_zero(mode);

    mode.w = pWidth;
    mode.h = pHeight;

    if (mWindow) {
        SDL_SetWindowDisplayMode(mWindow, &mode);
        // X_ASSERT(SDL_SetWindowDisplayMode(mWindow, &mode) != -1);
    }
}

void SDLWindow::setWindowMode(WindowMode pWindowMode) {
    Uint32 flags = 0;
    if (pWindowMode == WindowMode::FullScreen) {
        flags = SDL_WINDOW_FULLSCREEN;
    } else if (pWindowMode == WindowMode::WindowedFullScreen) {
        flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    int err = SDL_SetWindowFullscreen(mWindow, flags);
    if (err != 0) {
        const char* msg = SDL_GetError();
        if (msg) {
        }
    }
}

void SDLWindow::pollEvent() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        processEvent(event);
    }
}

void SDLWindow::processEvent(const SDL_Event& event) {
    if (!mEventCallback) return;

    switch (event.type) {
        //
        // Window Event
        //
        case SDL_WINDOWEVENT: {
            const SDL_WindowEvent& windowEvent = event.window;
            switch (windowEvent.event) {
                case SDL_WINDOWEVENT_SHOWN:
                    mEventCallback(Engine::WindowShowEvent());
                    break;
                case SDL_WINDOWEVENT_HIDDEN:
                    mEventCallback(Engine::WindowHideEvent());
                    break;
                case SDL_WINDOWEVENT_EXPOSED:
                    // Window has been exposed and should be redrawn
                    ENGINE_CORE_DEBUG("SDL_WINDOWEVENT_EXPOSED");
                    break;
                case SDL_WINDOWEVENT_MOVED:
                    mEventCallback(Engine::WindowMovedEvent(windowEvent.data1, windowEvent.data2));
                    break;

                case SDL_WINDOWEVENT_RESIZED:
                    // window has been resized
                    // this event is always preceded by SDL_WINDOWEVENT_SIZE_CHANGED

                    // mEventCallback(Engine::WindowResizeEvent(windowEvent.data1,
                    // windowEvent.data2));
                    break;

                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    // The window size has changed, either as a result of an API call or through the
                    // system or user changing the window size. this event is followed by
                    // SDL_WINDOWEVENT_RESIZED if the size was changed by an external event, i.e.
                    // the user or the window manager
                    mEventCallback(Engine::WindowResizeEvent(windowEvent.data1, windowEvent.data2));
                    break;
                case SDL_WINDOWEVENT_MINIMIZED:
                    mEventCallback(Engine::WindowMinimizedEvent());
                    break;
                case SDL_WINDOWEVENT_MAXIMIZED:
                    mEventCallback(Engine::WindowMaximizedEvent());
                    break;
                case SDL_WINDOWEVENT_RESTORED:
                    mEventCallback(Engine::WindowRestoredEvent());
                    break;
                case SDL_WINDOWEVENT_ENTER:
                    mEventCallback(Engine::WindowMouseEnterEvent());
                    break;
                case SDL_WINDOWEVENT_LEAVE:
                    mEventCallback(Engine::WindowMouseLeaveEvent());
                    break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    mEventCallback(Engine::WindowFocusGainEvent());
                    break;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    mEventCallback(Engine::WindowFocusLostEvent());
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    mEventCallback(Engine::WindowCloseEvent());
                    break;
                case SDL_WINDOWEVENT_TAKE_FOCUS:
                    // Window is being offered a focus
                    // (should SetWindowInputFocus() on itself or a subwindow, or ignore)
                    // linux only ....
                    ENGINE_CORE_DEBUG("SDL_WINDOWEVENT_TAKE_FOCUS");
                    break;
                case SDL_WINDOWEVENT_HIT_TEST:
                    // This is only triggered when using SDL_SetWindowHitTest()
                    // This call back need to return anything except SDL_HITTEST_NORMAL
                    ENGINE_CORE_DEBUG("SDL_WINDOWEVENT_HIT_TEST");
                    break;
                case SDL_WINDOWEVENT_DISPLAY_CHANGED:
                    // The window has been moved to another monitor
                    mEventCallback(Engine::WindowDisplaydChangeEvent(windowEvent.data1));
                    break;
                case SDL_WINDOWEVENT_ICCPROF_CHANGED:
                    // ICC profile  has change from the system ...
                    ENGINE_CORE_DEBUG("SDL_WINDOWEVENT_ICCPROF_CHANGED");
                    break;
            }
            break;
        }

        //
        // Event subtype for display events
        //
        case SDL_DISPLAYEVENT: {
            const SDL_DisplayEvent& e = event.display;
            if (e.event == SDL_DISPLAYEVENT_ORIENTATION) {
                // SDL_ORIENTATION_UNKNOWN,            /**< The display orientation can't be
                // determined */ SDL_ORIENTATION_LANDSCAPE,          /**< The display is in
                // landscape mode, with the right side up, relative to portrait mode */
                // SDL_ORIENTATION_LANDSCAPE_FLIPPED,  /**< The display is in landscape mode, with
                // the left side up, relative to portrait mode */ SDL_ORIENTATION_PORTRAIT, /**< The
                // display is in portrait mode */ SDL_ORIENTATION_PORTRAIT_FLIPPED    /**< The
                // display is in portrait mode, upside down */
                ENGINE_CORE_DEBUG("SDL_DISPLAYEVENT {} {}", "ORIENTATION", e.display, e.data1);
            }
            if (e.display == SDL_DISPLAYEVENT_CONNECTED) {
                ENGINE_CORE_DEBUG("SDL_DISPLAYEVENT {} {}", "CONNECTED", e.display);
            }
            if (e.display == SDL_DISPLAYEVENT_DISCONNECTED) {
                ENGINE_CORE_DEBUG("SDL_DISPLAYEVENT {} {}", "DISCONNECTED", e.display);
            }
            if (e.display == SDL_DISPLAYEVENT_MOVED) {
                ENGINE_CORE_DEBUG("SDL_DISPLAYEVENT {} {}", "MOVE", e.display);
            };
        }
        case SDL_LOCALECHANGED: {
            ENGINE_CORE_DEBUG("SDL_LOCALECHANGED");
            break;
        }
        //
        // Keyboard event
        //
        case SDL_KEYUP:
        case SDL_KEYDOWN: {
            const SDL_KeyboardEvent& keyEvent = event.key;

            const auto keyCode = fromSDL(keyEvent.keysym.scancode);
            const auto keyMod  = convert_sdl_key_mod((SDL_Keymod)keyEvent.keysym.mod);

            if (keyEvent.state == SDL_PRESSED) {
                mEventCallback(Engine::KeyPressedEvent(keyCode, keyEvent.repeat, keyMod));
            } else if (keyEvent.state == SDL_RELEASED) {
                mEventCallback(Engine::KeyReleasedEvent(keyCode, keyMod));
            }
        } break;
        case SDL_TEXTINPUT: {
            const SDL_TextInputEvent& textEvent = event.text;
            mEventCallback(Engine::InputTextEvent((char*)textEvent.text));
        } break;
        case SDL_TEXTEDITING:
            //
            //	TODO ???
            //
        {
            const SDL_TextEditingEvent& textEvent = event.edit;
            
            ENGINE_CORE_DEBUG("SDL_TEXTEDITING {} {} {}", textEvent.start, textEvent.length, textEvent.text);
        }
            break;
        case SDL_TEXTEDITING_EXT:
            //
            //	TODO ???
            //
            {
                const SDL_TextEditingExtEvent& textEvent = event.editExt;
                ENGINE_CORE_DEBUG("SDL_TEXTEDITING_EXT");     
            }

            break;
        case SDL_KEYMAPCHANGED:
            // Triggered when the keyboard layout changed (WM_INPUTLANGCHANGE)
            ENGINE_CORE_DEBUG("SDL_KEYMAPCHANGED");
            break;
        //
        // Mouse event
        //
        case SDL_MOUSEMOTION: {
            const SDL_MouseMotionEvent& motionEvent = event.motion;
            Engine::MouseButtonMask     buttonMask  = fromSDLButtonMask(motionEvent.state);
            Engine::MouseMovedEvent mouseMoveEvent(motionEvent.x, motionEvent.y, motionEvent.xrel,
                                                   motionEvent.yrel, buttonMask);
            mEventCallback(mouseMoveEvent);
        } break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            const SDL_MouseButtonEvent& e      = event.button;
            const Engine::MouseButton   button = fromSDL(e.button);

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                const bool doubleClick = e.clicks > 1;
                mEventCallback(Engine::MouseButtonPressedEvent(e.x, e.y, button, doubleClick));
            } else if (e.type == SDL_MOUSEBUTTONUP) {
                mEventCallback(Engine::MouseButtonReleasedEvent(e.x, e.y, button));
            }
        } break;
        case SDL_MOUSEWHEEL: {
            const SDL_MouseWheelEvent& e         = event.wheel;
            const bool                 isFlipped = event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED;
            const int                  x         = isFlipped ? -e.x : e.x;
            const int                  y         = isFlipped ? -e.y : e.y;
            mEventCallback(Engine::MouseScrolledEvent(x, y, e.mouseX, e.mouseY));
        } break;
        //
        // File drop event
        //
        case SDL_DROPBEGIN:
            mEventCallback(Engine::DropBeginEvent());
            break;
        case SDL_DROPCOMPLETE:
            mEventCallback(Engine::DropCompleteEvent());
            break;
        case SDL_DROPFILE: {
            const SDL_DropEvent& dropEvent = event.drop;
            mEventCallback(Engine::DropFileEvent(dropEvent.file));
            SDL_free(dropEvent.file);
        }
    }
}
