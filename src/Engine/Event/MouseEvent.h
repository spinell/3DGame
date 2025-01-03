#pragma once
#include "EventSystem.h"

#include "Engine/KeyCode.h"

#include <format>

namespace Engine {

/// @brief Event triggered when the mouse has moved.
class MouseMovedEvent : public Event {
public:
    EVENT_CLASS_TYPE(MouseMoved)

    MouseMovedEvent(int x, int y, int relX, int relY, MouseButtonMask buttonMask)
        : mPosX{x}, mPosY{y}, mRelX{relX}, mRelY{relY}, mButtonState{buttonMask} {}

    /// @brief Return the horizontal mouse position relative to the window.
    int getMouseX() const { return mPosX; };

    /// @brief Return the vertical mouse position relative to the window.
    int getMouseY() const { return mPosY; };

    /// @brief Return the horizontal motion.
    int getMouseRelX() const { return mRelX; };

    /// @brief Return the vertical motion.
    int getMouseRelY() const { return mRelY; };

    /// @brief Return the current mouse button state.
    MouseButtonMask getMouseButtonState() const { return mButtonState; };

    std::string toString() const override {
        return std::format("{}: {},{} rel = {},{} button {}", getName(), mPosX, mPosY, mRelX, mRelY,
                           static_cast<unsigned>(mButtonState));
    }

private:
    int             mPosX{};
    int             mPosY{};
    int             mRelX{};
    int             mRelY{};
    MouseButtonMask mButtonState{};
};

/// @brief Event triggered when the mouse wheel has moved.
class MouseScrolledEvent : public Event {
public:
    EVENT_CLASS_TYPE(MouseScrolled)

    MouseScrolledEvent(int x, int y, int mouseX, int mouseY)
        : mDeltaX{x}, mDeltaY{y}, mMouseX{mouseX}, mMouseY{mouseY} {}

    /// @brief Return the amount scrolled horizontally, positive to the right and negative to the
    /// left.
    int getX() const { return mDeltaX; };

    /// @brief Return the amount scrolled vertically, positive away from the user and negative
    /// toward the user
    int getY() const { return mDeltaY; };

    /// @brief Return the mouse X position.
    int getMouseX() const { return mMouseX; };

    /// @brief Return the mouse Y position.
    int getMouseY() const { return mMouseY; };

    std::string toString() const override {
        return std::format("{}: delta: {},{} mouse: {},{}", getName(), mDeltaX, mDeltaY, mMouseX,
                           mMouseY);
    }

private:
    int mDeltaX{};
    int mDeltaY{};
    int mMouseX{};
    int mMouseY{};
};

/// @brief Base class for mouse button event.
class MouseButtonEvent : public Event {
public:
    /// @brief Return the button related to this event.
    MouseButton getMouseButton() const { return mButton; }

    /// @brief Return the mouse horizontal position relative to window.
    int getMouseX() const { return mPositionX; }

    /// @brief Return the mouse vertical position relative to window.
    int GetMouseY() const { return mPositionY; }

protected:
    MouseButtonEvent(MouseButton button, int x, int y)
        : mButton(button), mPositionX(x), mPositionY(y) {}

    int         mPositionX{};
    int         mPositionY{};
    MouseButton mButton{};
};

/// @brief Event triggered when the mouse has moved.
class MouseButtonPressedEvent : public MouseButtonEvent {
public:
    EVENT_CLASS_TYPE(MouseButtonPressed)

    MouseButtonPressedEvent(int x, int y, MouseButton button, bool doubleClick)
        : MouseButtonEvent(button, x, y), mIsDoubleClick{doubleClick} {}

    /// @param Return true if this is a double click.
    bool isDoubleClick() const { return mIsDoubleClick; }

    std::string toString() const override {
        return std::format("{}: {} [{},{}]", getName(), (int)mButton, mPositionX, mPositionY);
    }

private:
    bool mIsDoubleClick{};
};

/// @brief Event triggered when the mouse has moved.
class MouseButtonReleasedEvent : public MouseButtonEvent {
public:
    EVENT_CLASS_TYPE(MouseButtonReleased)

    MouseButtonReleasedEvent(int x, int y, MouseButton button) : MouseButtonEvent(button, x, y) {}

    std::string toString() const override {
        return std::format("{}: {} [{},{}]", getName(), (int)mButton, mPositionX, mPositionY);
    }
};

} // namespace Engine
