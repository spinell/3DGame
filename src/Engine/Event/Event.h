#pragma once
#include "Engine/KeyCode.h"

#include <functional>
#include <string>

// TODO: Move somewhere else
#define BIT(x) (1u << x)

namespace Engine {

enum class EventType {
    None = 0,
    // Window Event
    WindowClose,
    WindowFocusGain,
    WindowFocusLost,
    WindowHide,
    WindowMaximized,
    WindowMinimized,
    WindowMouseEnter,
    WindowMouseLeave,
    WindowMoved,
    WindowRestored,
    WindowResize,
    WindowShow,
    WindowDisplayChanged,
    // WindowTitleBarHitTest,
    //  Drop event
    DropBegin,
    DropComplete,
    DropFile,
    DropText,
    // Keyboard event
    KeyPressed,
    KeyReleased,
    InputText,
    // Mouse Event
    MouseButtonPressed,
    MouseButtonReleased,
    MouseButtonDown,
    MouseMoved,
    MouseScrolled
};

enum EventCategory {
    EventCategoryWindow      = BIT(0),
    EventCategoryInput       = BIT(1),
    EventCategoryKeyboard    = BIT(2),
    EventCategoryMouse       = BIT(3),
    EventCategoryMouseButton = BIT(4),
    EventCategoryDrop        = BIT(5)
};

#define EVENT_CLASS_TYPE(type)                                                    \
    static EventType GetStaticType() { return EventType::type; }                  \
                                                                                  \
    virtual EventType   getEventType() const override { return GetStaticType(); } \
    virtual const char* getName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) \
    virtual int GetCategoryFlags() const override { return category; }

struct Event {
    bool Handled = false;

    virtual EventType   getEventType() const     = 0;
    virtual const char* getName() const          = 0;
    virtual int         GetCategoryFlags() const = 0;
    virtual std::string toString() const { return getName(); }

    bool isInCategory(EventCategory category) const { return GetCategoryFlags() & category; }
};

class EventDispatcher {
    template <typename T>
    using EventFn = std::function<bool(T&)>;

public:
    EventDispatcher(Event& event) : m_Event(event) {}

    template <typename T>
    bool Dispatch(EventFn<T> func) {
        if (m_Event.getEventType() == T::GetStaticType() && !m_Event.Handled) {
            m_Event.Handled = func(*(T*)&m_Event);
            return true;
        }
        return false;
    }

private:
    Event& m_Event;
};

}; // namespace Engine
