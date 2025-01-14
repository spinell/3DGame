#pragma once
#include <Engine/KeyCode.h>

#include <format>
#include <functional>
#include <variant>

namespace Engine {

//==============================================================================
//                              Mouse Events
//==============================================================================

/// @brief Event triggered when the mouse has moved.
class MouseMovedEvent {
public:
    MouseMovedEvent(float x, float y, float deltaX, float deltaY, MouseButtonFlag buttonMask)
        : mPosX{x}, mPosY{y}, mDeltaX{deltaX}, mDeltaY{deltaY}, mButtonState{buttonMask} {}

    std::string toString() const {
        if (mButtonState == MouseButtonFlag::Unknow) {
            return std::format("MouseMoved: {},{}", mPosX, mPosY);
        } else {
            return std::format("MouseMoved: {},{} [{}]", mPosX, mPosY,
                               Engine::toString(mButtonState));
        }
    }

    std::pair<float, float> getMousePosition() const { return {mPosX, mPosY}; }
    std::pair<float, float> getMouseDelta() const { return {mDeltaX, mDeltaY}; }

private:
    float           mPosX{};
    float           mPosY{};
    float           mDeltaX{};
    float           mDeltaY{};
    MouseButtonFlag mButtonState{};
};

/// @brief Event triggered when the mouse wheel has moved.
class MouseScrolledEvent {
public:
    MouseScrolledEvent(float deltaX, float deltaY, float mouseX, float mouseY)
        : mDeltaX{deltaX}, mDeltaY{deltaY}, mMouseX{mouseX}, mMouseY{mouseY} {}

    /// @brief Return the amount scrolled horizontally and vertically.
    ///        Horizontal positive to the right and negative to the left.
    ///        Vertical positive away from the user and negative toward the user
    std::pair<float, float> getDelta() const { return {mDeltaX, mDeltaY}; };

    /// @brief Return the amount scrolled horizontally.
    /// positive to the right and negative to the left.
    float getDeltaX() const { return mDeltaX; };

    /// @brief Return the amount scrolled vertically
    /// positive away from the user and negative toward the user
    float getDeltaY() const { return mDeltaY; };

    std::string toString() const {
        return std::format("MouseScrolled: {},{}  pos {},{}", mDeltaX, mDeltaY, mMouseX, mMouseY);
    }

private:
    float mDeltaX{};
    float mDeltaY{};
    float mMouseX{};
    float mMouseY{};
};

/// @brief Event triggered when a mouse button has been pressed or released.
class MouseButtonEvent {
public:
    MouseButtonEvent(
        MouseButton button, bool isPressed, unsigned click, double mouseX, double mouseY)
        : mButton(button), mIsPressed(isPressed), mClick(click), mMouseX(mouseX), mMouseY(mouseY) {}

    bool                      isDown() const { return mIsPressed; }
    MouseButton               getButton() const { return mButton; }
    std::pair<double, double> getMousePos() const { return {mMouseX, mMouseY}; }
    double                    getMousePosX() const { return mMouseX; };
    double                    getMousePosY() const { return mMouseY; };

    std::string toString() const {
        if (mIsPressed) {
            return std::format("ButtonPressed: {} ({},{}) click={}", Engine::toString(mButton),
                               mMouseX, mMouseY, mClick);
        } else {
            return std::format("ButtonReleased: {} ({},{})", Engine::toString(mButton), mMouseX,
                               mMouseY);
        }
    }

private:
    MouseButton mButton{};
    bool        mIsPressed{};
    unsigned    mClick{};
    double      mMouseX{};
    double      mMouseY{};
};

//==============================================================================
//                              Keyboard Events
//==============================================================================

/// @brief Event triggered when a key was pressed / released.
class KeyEvent {
public:
    KeyEvent(KeyCode key, int scancode, bool pressed, bool isRepeated, KeyModFlag modifier)
        : mKey(key),
          mScannCode(scancode),
          mIsPressed(pressed),
          mIsRepeated{isRepeated},
          mModifier{modifier} {}

    KeyCode getKey() const { return mKey; }

    bool isRepeated() const { return mIsRepeated; }
    bool isPressed() const { return mIsPressed; }

    bool isShift() const { return false; }
    bool isLeftShift() const { return false; }
    bool isRightShift() const { return false; }

    std::string toString() const {
        const std::string modFormat =
            mModifier == KeyModFlag::None ? "" : Engine::toString(mModifier);
        if (mIsPressed) {
            return std::format("KeyPressed: key={} scannCode={} repeated={} [{}]", (int)mKey,
                               mScannCode, mIsRepeated, modFormat);
        } else {
            return std::format("KeyRelesed: key={} scannCode={} {}", (int)mKey, mScannCode,
                               modFormat);
        }
    }

private:
    KeyCode    mKey{};
    int        mScannCode{};
    bool       mIsPressed{};
    bool       mIsRepeated{};
    KeyModFlag mModifier{};
};

//==============================================================================
//                              Windows Events
//==============================================================================

struct WindowCloseEvent {
    std::string toString() const { return "WindowClose"; }
};

/// @brief Event triggered when the window has been move to another monitor.
class WindowDisplaydChangedEvent {
public:
    WindowDisplaydChangedEvent(int index) : mMonitorIndex(index) {}

    int getMonitorIndex() const { return mMonitorIndex; };

    std::string toString() const { return std::format("WindowDisplaydChanged: {}", mMonitorIndex); }

private:
    int mMonitorIndex{};
};

/// @brief Event triggered when the window enter the fullscreen mode.
struct WindowFullScreenEnterEvent {
    std::string toString() const { return "WindowFullScreenEnter"; }
};

/// @brief Event triggered when the window leave the fullscreen mode.
struct WindowFullScreenLeaveEvent {
    std::string toString() const { return "WindowFullScreenEnter"; }
};

/// @brief Event triggered when the window gain the focus.
struct WindowFocusGainEvent {
    std::string toString() const { return "WindowFocusGainEvent"; }
};

/// @brief Event triggered when the window lost the focus.
struct WindowFocusLostEvent {
    std::string toString() const { return "WindowFocusLostEvent"; }
};

/// @brief Event triggered when the window is maximized.
class WindowMaximizedEvent {
public:
    std::string toString() const { return "WindowMaximizedEvent"; }
};

/// @brief Event triggered when the window is minimized.
class WindowMinimizedEvent {
public:
    std::string toString() const { return "WindowMinimized"; }
};

/// @brief Event triggered when the mouse enter into the window.
struct WindowMouseEnterEvent {
    std::string toString() const { return "WindowMouseEnterEvent"; }
};

/// @brief Event triggered when the mouse leave the window.
struct WindowMouseLeaveEvent {
    std::string toString() const { return "WindowMouseLeaveEvent"; }
};

/// @brief Event triggered when the window has been moved.
class WindowMovedEvent {
public:
    WindowMovedEvent(int x, int y) : mPosX(x), mPosY(y) {}

    std::string toString() const { return std::format("WindowMoved: {},{}", mPosX, mPosY); }

private:
    int mPosX{};
    int mPosY{};
};

/// @brief Event triggered when the go back to it normal state (after minimized / maximized).
class WindowRestoredEvent {
public:
    std::string toString() const { return "WindowRestored"; }
};

/// @brief Event triggered when the window has been resized.
class WindowResizedEvent {
public:
    WindowResizedEvent(int width, int height) : mWidth(width), mHeight(height) {}

    inline unsigned int GetWidth() const { return mWidth; }
    inline unsigned int GetHeight() const { return mHeight; }

    std::string toString() const { return std::format("WindowResized: {}x{}", mWidth, mHeight); }

private:
    int mWidth{};
    int mHeight{};
};

//==============================================================================
//                              Drop Events
//==============================================================================

template <typename... Ts>
struct OverloadSet : Ts... {
    using Ts::operator()...;
};

/// @brief The requirement for a event type hold in \p Event.
template <typename T>
concept EventRequirement = requires(T t) {
    { t.toString() } -> std::convertible_to<std::string>;
};

/// @brief Thin wraper around std::variant to handle event.
class Event {
public:
    /// @brief Contruct a event from the real event type.
    ///
    /// @tparam T The type of the event.
    /// @param  e The event itself.
    template <EventRequirement T>
    Event(T e) : mData(e) {
        static_assert(isValidEventType<T>, "T is not defined as a event in std::variant<> list.");
    };

    /// @brief Check if the underlying event is a specific type.
    ///
    /// @tparam T The event type to check.
    /// @return Return true if the underlying event type is \p T.
    template <EventRequirement T>
    bool isA() const {
        static_assert(isValidEventType<T>, "T is not defined as a event in std::variant<> list.");
        return std::holds_alternative<T>(mData);
    }

    /// @brief Check if the underlying event is any one defined by \p T.
    ///
    /// @tparam T The list of event type to check.
    /// @return Return true if the underlying event type any one defined by \p T.
    template <EventRequirement... T>
    bool anyOf() const {
        static_assert(sizeof...(T) > 0, "T must be greater then 0.");
        static_assert((isValidEventType<T> && ...),
                      "T is not defined as a event in std::variant<> list.");
        return (std::holds_alternative<T>(mData) || ...);
    }

    /// @brief Return the underlying event.
    ///
    /// @tparam T The event type to return.
    /// @return Return a pointer on the underlying event or null ifthe event type is not \p T.
    template <EventRequirement T>
    T* getIf() {
        static_assert(isValidEventType<T>, "T is not defined as a event in std::variant<> list.");
        return std::get_if<T>(&mData);
    }

    /// @brief Return the underlying event.
    ///
    /// @tparam T The event type to return.
    /// @return Return a pointer on the underlying event or null ifthe event type is not \p T.
    template <EventRequirement T>
    const T* getIf() const {
        static_assert(isValidEventType<T>, "T is not defined as a event in std::variant<> list.");
        return std::get_if<T>(&mData);
    }

    template <EventRequirement T>
    void dispatch(std::function<void(const T&)> callback) const {
        if (isA<T>() && callback) {
            callback(std::get<T>(mData));
        }
    }

    template <EventRequirement... Ts>
    void dispatch2(Ts&&... handlers) {
        OverloadSet overloadSet{std::forward<Ts>(handlers)...,
                                []<typename T>(const T& myNum) { /* ignore */ }};
        visit(overloadSet);
    }

    /// @brief Return a string that represente the event (Usefull for debugging).
    std::string toString() const {
        auto visitor = []<typename T>(const T& event) { return event.toString(); };
        return std::visit(visitor, mData);
    }

private:
    std::variant<
        // Windows event
        WindowCloseEvent,
        WindowDisplaydChangedEvent,
        WindowMouseEnterEvent,
        WindowMouseLeaveEvent,
        WindowFocusGainEvent,
        WindowFocusLostEvent,
        WindowFullScreenEnterEvent,
        WindowFullScreenLeaveEvent,
        WindowMovedEvent,
        WindowMaximizedEvent,
        WindowMinimizedEvent,
        WindowResizedEvent,
        WindowRestoredEvent,
        // Mouse event
        MouseButtonEvent,
        MouseMovedEvent,
        MouseScrolledEvent,
        // Key event
        KeyEvent>
        mData;

    template <typename T, typename... Ts>
    [[nodiscard]] static constexpr bool isInParameterPack(const std::variant<Ts...>*) {
        return (std::is_same_v<T, Ts> || ...);
    }

    template <typename T>
    static constexpr bool isValidEventType = isInParameterPack<T>(decltype (&mData)(nullptr));

    template <typename T>
    decltype(auto) visit(T&& visitor) const {
        return std::visit(std::forward<T>(visitor), mData);
    }
};

} // namespace Engine
