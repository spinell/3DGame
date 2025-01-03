#pragma once
#include "Event.h"

#include <format>

namespace Engine {

struct WindowShowEvent : public Event {
    EVENT_CLASS_TYPE(WindowResize)
    EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

struct WindowHideEvent : public Event {
    EVENT_CLASS_TYPE(WindowHide)
    EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

/// @brief Event triggered when the window has been moved.
class WindowMovedEvent : public Event {
public:
    EVENT_CLASS_TYPE(WindowMoved)
    EVENT_CLASS_CATEGORY(EventCategoryWindow)

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
    EVENT_CLASS_CATEGORY(EventCategoryWindow)

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
    EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

/// @brief Event triggered when the window has been maximized.
struct WindowMaximizedEvent : public Event {
    EVENT_CLASS_TYPE(WindowMaximized)
    EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

/// @brief Event triggered when the window has been restored to normal size and position.
struct WindowRestoredEvent : public Event {
    EVENT_CLASS_TYPE(WindowRestored)
    EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

/// @brief Event triggered when the mouse enter the windows client zone.
struct WindowMouseEnterEvent : public Event {
    EVENT_CLASS_TYPE(WindowMouseEnter)
    EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

/// @brief Event triggered when the mouse leave the windows client zone.
struct WindowMouseLeaveEvent : public Event {
    EVENT_CLASS_TYPE(WindowMouseLeave)
    EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

/// @brief Event triggered when the window get the keyboard focus.
struct WindowFocusGainEvent : public Event {
    EVENT_CLASS_TYPE(WindowFocusGain)
    EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

/// @brief Event triggered when the window lost the keyboard focus.
struct WindowFocusLostEvent : public Event {
    EVENT_CLASS_TYPE(WindowFocusLost)
    EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

/// @brief Event triggered when the window is closed.
struct WindowCloseEvent : public Event {
    EVENT_CLASS_TYPE(WindowClose)
    EVENT_CLASS_CATEGORY(EventCategoryWindow)
};

/// @brief Event triggered when the window has been move to another monitor.
class WindowDisplaydChangeEvent : public Event {
public:
    EVENT_CLASS_TYPE(WindowDisplayChanged)
    EVENT_CLASS_CATEGORY(EventCategoryWindow)

    WindowDisplaydChangeEvent(int index) : mMonitorIndex(index) {}

    int getMonitorIndex() const { return mMonitorIndex; };

    std::string toString() const override {
        return std::format("{}: {}", getName(), mMonitorIndex);
    }

private:
    int mMonitorIndex{};
};

} // namespace Engine
