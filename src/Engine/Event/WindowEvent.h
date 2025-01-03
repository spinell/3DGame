#pragma once
#include "Event.h"

#include <format>

namespace Engine {

struct WindowShowEvent : public Event {
    EVENT_CLASS_TYPE(WindowResize)
};

struct WindowHideEvent : public Event {
    EVENT_CLASS_TYPE(WindowHide)
};

/// @brief Event triggered when the window has been moved.
class WindowMovedEvent : public Event {
public:
    EVENT_CLASS_TYPE(WindowMoved)

    WindowMovedEvent(int x, int y) : mPosX(x), mPosY(y) {}

    std::string toString() const override {
        return std::format("{}: {}x{}", getName(), mPosX, mPosY);
    }

private:
    int mPosX{};
    int mPosY{};
};

/// @brief Event triggered when the window has been resized.
class WindowResizeEvent : public Event {
public:
    EVENT_CLASS_TYPE(WindowResize)

    WindowResizeEvent(int width, int height) : mWidth(width), mHeight(height) {}

    inline unsigned int GetWidth() const { return mWidth; }
    inline unsigned int GetHeight() const { return mHeight; }

    std::string toString() const override {
        return std::format("{}: {}x{}", getName(), mWidth, mHeight);
    }

private:
    int mWidth{};
    int mHeight{};
};

/// @brief Event triggered when the window has been minimized.
struct WindowMinimizedEvent : public Event {
    EVENT_CLASS_TYPE(WindowMinimized)
};

/// @brief Event triggered when the window has been maximized.
struct WindowMaximizedEvent : public Event {
    EVENT_CLASS_TYPE(WindowMaximized)
};

/// @brief Event triggered when the window has been restored to normal size and position.
struct WindowRestoredEvent : public Event {
    EVENT_CLASS_TYPE(WindowRestored)
};

/// @brief Event triggered when the mouse enter the windows client zone.
struct WindowMouseEnterEvent : public Event {
    EVENT_CLASS_TYPE(WindowMouseEnter)
};

/// @brief Event triggered when the mouse leave the windows client zone.
struct WindowMouseLeaveEvent : public Event {
    EVENT_CLASS_TYPE(WindowMouseLeave)
};

/// @brief Event triggered when the window get the keyboard focus.
struct WindowFocusGainEvent : public Event {
    EVENT_CLASS_TYPE(WindowFocusGain)
};

/// @brief Event triggered when the window lost the keyboard focus.
struct WindowFocusLostEvent : public Event {
    EVENT_CLASS_TYPE(WindowFocusLost)
};

/// @brief Event triggered when the window is closed.
struct WindowCloseEvent : public Event {
    EVENT_CLASS_TYPE(WindowClose)
};
} // namespace Engine
