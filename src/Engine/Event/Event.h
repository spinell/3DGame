#pragma once
#include "Engine/KeyCode.h"

#include <functional>
#include <string>

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

#define EVENT_CLASS_TYPE(type)                                                    \
    static EventType GetStaticType() { return EventType::type; }                  \
                                                                                  \
    virtual EventType   getEventType() const override { return GetStaticType(); } \
    virtual const char* getName() const override { return #type; }

struct Event {
    bool Handled = false;

    virtual EventType   getEventType() const { return EventType::None; };
    virtual const char* getName() const { return ""; };
    virtual std::string toString() const { return getName(); }
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
