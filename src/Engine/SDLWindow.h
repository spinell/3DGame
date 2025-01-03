#pragma once
#include <Engine/Event/EventSystem.h>
#include <Engine/Window.h>
#include <SDL2/SDL.h>

/*
 * Implementation of Window for SDL
 *
 *
 */
class SDLWindow : public Engine::Window
{
public:
    /// \brief Retrive a SDLWindow object from a sdl window ID.
    /// \param windowId The sdl window ID.
    /// \return A pointer the a SDLWindow of null.
    static SDLWindow *GetWindowFromID(Uint32 windowId);

public:
    SDLWindow();
    ~SDLWindow();

    /// @copydoc Window::create
    bool create(const CreateInfo &createInfo = CreateInfo()) override;

    /// @copydoc Window::setGrabInput
    void setGrabInput(bool pGrabInput) override;

    /// @copydoc Window::isGrabInput
    bool isGrabInput() const override;

    /// @copydoc Window::toogleGrabInput
    void toogleGrabInput() override;

    /// @copydoc Window::show
    void show() override;

    /// @copydoc Window::hide
    void hide() override;

    /// @copydoc Window::isVisible
    bool isVisible() const override;

    /// @copydoc Window::hasFocus
    bool hasFocus() const override;

    /// @copydoc Window::raise
    void raise() const override;

    /// @copydoc Window::setIcon
    void setIcon(unsigned width, unsigned height, const unsigned *data) override;

    /// @copydoc Window::setOpacity
    void setOpacity(float opacity) override;

    /// @copydoc Window::setMaximumSize
    void setMaximumSize(int maxWidth, int maxHeight) override;

    /// @copydoc Window::getMaximalSize
    void getMaximumSize(int &pOutMaximalWidth, int &pOutMaximalHeight) const override;

    /// @copydoc Window::setMinimumSize
    void setMinimumSize(int minWidth, int minHeight) override;

    /// @copydoc Window::getMinimalSize
    void getMinimumSize(int &pOutMinimalWidth, int &pOutMinimalHeight) const override;

    /// @copydoc Window::setPosition
    void setPosition(int x, int y) override;

    /// @copydoc Window::getPosition
    void getPosition(int &x, int &y) override;

    /// @copydoc Window::setSize
    void setSize(int width, int height) override;

    /// @copydoc Window::setSize
    void setResizable(bool resizable) override;

    /// @copydoc Window::getSize
    void getSize(int &width, int &height) override;

    /// @copydoc Window::getHeight
    int getHeight() const override;

    /// @copydoc Window::getWidth
    int getWidth() const override;

    /// @copydoc Window::setTitle
    void setTitle(const char *title) override;

    /// @copydoc Window::getTitle
    const char *getTitle() const override;

    /// @copydoc Window::maximize
    void maximize() override;

    /// @copydoc Window::minimize.
    void minimize() override;

    /// @copydoc Window::restore
    void restore() override;

    /// @copydoc Window::restore
    bool isMinimized() const override;

    /// @copydoc Window::restore
    bool isMaximized() const override;

    /// @copydoc Window::isResizable
    bool isResizable() const override;

    /// @copydoc Window::setFullScreenVideoMode
    void setFullScreenVideoMode(int pWidth, int pHeight) override;

    /// @copydoc Window::setWindowMode
    void setWindowMode(WindowMode pWindowMode) override;

    /// @copydoc Window::getWindowHandle
    void *getWindow() const override;
    void *getWindowHandle() const override;

    void pollEvent();
    void processEvent(const SDL_Event &);

private:
    /// \brief The SDL window handle
    SDL_Window *mWindow{};

    /// \brief The SDL window ID;
    Uint32 mWindowID{};

    /// \brief
    SDL_Surface *mIconSurface{};
};
