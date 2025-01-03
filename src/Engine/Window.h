#pragma once
#include "Engine/Event/EventSystem.h"
#include <functional>

namespace Engine
{

    ///
    ///
    /// TODO add a way to close the window ?
    //
    /// TODO GetGrabbedWindow
    /// TODO GetWindowOpacity
    /// TODO SetWindowBordered
    /// TODO SetWindowBrightness
    /// TODO SetWindowGammaRamp
    /// TODO SetWindowGrab
    /// TODO SetWindowHitTest
    /// TODO SetWindowInputFocus
    /// TODO SetWindowModalFor
    ///
    class Window
    {
    public:
        //
        enum class WindowMode
        {
            /// \brief "real" fullscreen with a videomode change
            FullScreen,
            /// \brief windowed mode
            Windowed,
            /// \brief "fake" fullscreen that takes the size of the desktop
            WindowedFullScreen
        };

        struct CreateInfo
        {
            const char *title = "Engine Window";
            int posX = 0;
            int posY = 40;
            int width = 800;
            int height = 600;
            bool resizable = true;
            bool showOnCreation = true;
            bool borderless = false;
            bool alwaysOnTop = false;
            bool noTaskBarIcon = false;
        };

        using EventCallback = std::function<void(const Event &)>;

    public:
        virtual ~Window() {}

        /*
         * \brief
         * \param createInfo
         */
        virtual bool create(const CreateInfo &createInfo = CreateInfo()) = 0;

        /// @biref
        ///
        /// When input is grabbed the mouse is confined to the window.
        /// If the caller enables a grab while another window is currently grabbed
        /// the other window loses its grab in favor of the caller's window.
        virtual void setGrabInput(bool pGrabInput) = 0;

        /// @return true if this window is grabbing the input
        virtual bool isGrabInput() const = 0;

        /*
         * \brief Toogle the grab input mode of the window.
         *
         *  When input is grabbed the mouse is confined to the window.
         *  If the caller enables a grab while another window is currently grabbed
         *  the other window loses its grab in favor of the caller's window.
         */
        virtual void toogleGrabInput() = 0;

        /// @biref Show the window.
        virtual void show() = 0;

        /// @biref Hide the window.
        virtual void hide() = 0;

        /// @biref Make a window as large as possible.
        virtual void maximize() = 0;

        /// @biref Minimize a window to an iconic representation.
        virtual void minimize() = 0;

        /// @biref Restore the size and position of a minimized or maximized window.
        virtual void restore() = 0;

        /// @biref Return true is the window is visible, otherwise false.
        virtual bool isVisible() const = 0;

        /// @biref Return true is the window has the input focus.
        virtual bool hasFocus() const = 0;

        /// @biref raise this window above other windows and set the input focus.
        virtual void raise() const = 0;

        /// @biref Set the icon for a window.
        /// @param width  The width of the icon
        /// @param height The height of the icon
        /// @param pixel The pixel of the icon in RGBA unsigned int format
        virtual void setIcon(unsigned width, unsigned height, const unsigned *pixel) = 0;

        /// @biref  set the opacity for this window.
        /// @param opacity the opacity value (0.0f - transparent, 1.0f - opaque)
        virtual void setOpacity(float opacity) = 0;

        /// @biref set the maximum size of a window's client area.
        /// @param maxWidth the width of the window in pixels must be > 0
        /// @param mHeight the height of the window in pixels must be > 0
        virtual void setMaximumSize(int maxWidth, int maxHeight) = 0;

        /*
         * \brief Use this function to get the maximum size of a window's client area.
         * \param pOutMaximalWidth
         * \param pOutMaximalHeight
         */
        virtual void getMaximumSize(int &pOutMaximalWidth, int &pOutMaximalHeight) const = 0;

        /// @biref set the maximum size of a window's client area.
        /// @param maxWidth the width of the window in pixels must be > 0
        /// @param mHeight the height of the window in pixels must be > 0
        virtual void setMinimumSize(int minWidth, int minHeight) = 0;

        /*
         * \brief Use this function to get the minimum size of a window's client area.
         * \param pOutMinimalWidth
         * \param pOutMinimalHeight
         */
        virtual void getMinimumSize(int &pOutMinimalWidth, int &pOutMinimalHeight) const = 0;

        /// @biref
        /// @param x
        /// @param y
        virtual void setPosition(int x, int y) = 0;
        virtual void getPosition(int &x, int &y) = 0;

        /// @biref
        /// @param width the width of the window in pixels must be > 0
        /// @param height the height of the window in pixels must be > 0
        virtual void setSize(int width, int height) = 0;

        /// @biref
        /// @param resizable
        virtual void setResizable(bool resizable) = 0;

        /// @biref
        /// @param title
        virtual void setTitle(const char *title) = 0;

        /// @biref Get the size of a window's client area.
        /// @param width
        /// @param height
        virtual void getSize(int &width, int &height) = 0;

        /*
         * \brief The height of the client area of the window.
         */
        virtual int getHeight() const = 0;

        /*
         * \brief The width of the client area of the window.
         */
        virtual int getWidth() const = 0;

        /// @biref Returns the title of the window in UTF-8 format or "" if there is no title.
        /// @return
        virtual const char *getTitle() const = 0;

        virtual bool isMinimized() const = 0;
        virtual bool isMaximized() const = 0;
        virtual bool isResizable() const = 0;

        /*
         * \brief Set the display mode to use when a window is visible at fullscreen.
         * \param
         */
        virtual void setFullScreenVideoMode(int pWidth, int pHeight) = 0;

        /*
         * \brief Set the window's fullscreen state.
         *
         * \param pWindowMode
         */
        virtual void setWindowMode(WindowMode pWindowMode) = 0;

        /*
         * \return the platform native window handle
         */
        virtual void *getWindow() const = 0;
        virtual void *getWindowHandle() const = 0;

        /*
         * \return the platform native window handle
         */
        void setEventCallback(EventCallback callback) { mEventCallback = callback; }

    protected:
        EventCallback mEventCallback{};
    };

} // namespace spine
