#pragma once
#include "Engine/KeyCode.h"
#include <format>

namespace Engine
{

    enum class EventType
    {
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
        MouseScrolled,
        // Application Event
        AppTick,
        AppUpdate,
        AppRender,
        // Scene Event
        ScenePreStart,
        ScenePostStart,
        ScenePreStop,
        ScenePostStop,
        // Selection Event
        SelectionChanged,
        // Editor Event
        EditorExitPlayMode,
        // Others
        AssetReloaded,
        AnimationGraphCompiled
    };

#define EVENT_CLASS_TYPE(type)                                                  \
    static EventType GetStaticType() { return EventType::type; }                \
                                                                                \
    virtual EventType getEventType() const override { return GetStaticType(); } \
    virtual const char *getName() const override { return #type; }

    struct Event
    {

        virtual EventType getEventType() const { return EventType::None; };
        virtual const char *getName() const { return ""; };
        virtual std::string toString() const { return getName(); }
    };

    //
    // Drop Event
    //
    class DropBeginEvent : public Event
    {
    };
    class DropCompleteEvent : public Event
    {
    };
    class DropTextEvent : public Event
    {
    };
    class DropFileEvent : public Event
    {
    };

    class EventSystem
    {
    public:
        EventSystem();
        ~EventSystem();

        EventSystem(const EventSystem &) = delete;
        EventSystem(EventSystem &&) = delete;

        EventSystem &operator=(const EventSystem &) = delete;
        EventSystem &operator=(EventSystem &&) = delete;
    };

};
