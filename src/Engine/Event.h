#pragma once
#include <Engine/KeyCode.h>

#include <format>
#include <string>

namespace Engine {

//
// Windows events
//

/// @brief Event triggered when the window is closed.
struct WindowCloseEvent {};

/// @brief Event triggered when the window has been resized.
class WindowResizeEvent {
public:
    WindowResizeEvent(int width, int height) : mWidth(width), mHeight(height) {}

    inline unsigned int GetWidth() const { return mWidth; }
    inline unsigned int GetHeight() const { return mHeight; }

    std::string toString() const { return std::format("WindowResize: {}x{}", mWidth, mHeight); }

private:
    int mWidth{};
    int mHeight{};
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

//
// Keyboard events
//

/// @brief Event triggered when a key was pressed.
class KeyPressedEvent {
public:
    KeyPressedEvent(int key, int scancode, bool isRepeated, KeyModFlag modifier)
        : mKey(key), mScannCode(scancode), mIsRepeated{isRepeated}, mModifier{modifier} {}

    int getKey() const { return mKey; }

    bool isRepeated() const { return mIsRepeated; }

    bool isShift() const { return false; }
    bool isLeftShift() const { return false; }
    bool isRightShift() const { return false; }

    std::string toString() const {
        return std::format("KeyPress: key={} scannCode={} repeated={} mod={}", mKey, mScannCode,
                           mIsRepeated, Engine::toString(mModifier));
    }

private:
    int        mKey{};
    int        mScannCode{};
    bool       mIsRepeated{};
    KeyModFlag mModifier{};
};

/// @brief Event triggered when a key was released.
class KeyReleaseEvent {
public:
    KeyReleaseEvent(int key, int scancode, KeyModFlag modifier)
        : mKey(key), mScannCode(scancode), mModifier{modifier} {}

    bool isShift() const { return false; }
    bool isLeftShift() const { return false; }
    bool isRightShift() const { return false; }

    std::string toString() const {
        return std::format("KeyRelease: key={} scannCode={} mod={}", mKey, mScannCode,
                           Engine::toString(mModifier));
    }

private:
    int        mKey{};
    int        mScannCode{};
    KeyModFlag mModifier{};
};

//==============================================================================
//                              Mouse Events
//==============================================================================

/// @brief Event triggered when a mouse button has been pressed.
class MousePressEvent {
public:
    MousePressEvent(MouseButton button, double mouseX, double mouseY, KeyModFlag mod)
        : mButton(button), mMouseX(mouseX), mMouseY(mouseY), mModifier(mod) {}

    MouseButton               getButton() const { return mButton; }
    std::pair<double, double> getMousePosition() const { return {mMouseX, mMouseY}; }
    double                    getMousePositionX() const { return mMouseX; };
    double                    getMousePositionY() const { return mMouseY; };
    KeyModFlag                getKeyModifier() const { return mModifier; };

    std::string toString() const {
        return std::format("MousePress: Button={} ({},{}) mod={}", Engine::toString(mButton),
                           mMouseX, mMouseY, Engine::toString(mModifier));
    }

private:
    double      mMouseX{};
    double      mMouseY{};
    MouseButton mButton{};
    KeyModFlag  mModifier;
};

/// @brief Event triggered when a mouse button has been released.
class MouseReleaseEvent {
public:
    MouseReleaseEvent(MouseButton button, double mouseX, double mouseY, KeyModFlag mod)
        : mButton(button), mMouseX(mouseX), mMouseY(mouseY), mModifier(mod) {}

    MouseButton               getButton() const { return mButton; }
    std::pair<double, double> getMousePosition() const { return {mMouseX, mMouseY}; }
    double                    getMousePositionX() const { return mMouseX; };
    double                    getMousePositionY() const { return mMouseY; };
    KeyModFlag                getKeyModifier() const { return mModifier; };

    std::string toString() const {
        return std::format("MouseRelease: Button={} ({},{}) mod={}", Engine::toString(mButton),
                           mMouseX, mMouseY, Engine::toString(mModifier));
    }

private:
    double      mMouseX{};
    double      mMouseY{};
    MouseButton mButton{};
    KeyModFlag  mModifier{};
};

/// @brief Event triggered when the mouse has moved.
class MouseMovedEvent {
public:
    MouseMovedEvent(int x, int y, MouseButtonFlag buttonMask)
        : mPosX{x}, mPosY{y}, mButtonState{buttonMask} {}

    std::string toString() const {
        return std::format("MouseMoved: {},{} button {}", mPosX, mPosY,
                           Engine::toString(mButtonState));
    }

private:
    int             mPosX{};
    int             mPosY{};
    MouseButtonFlag mButtonState{};
};

/// @brief Event triggered when the mouse wheel has moved.
class MouseScrolledEvent {
public:
    MouseScrolledEvent(double deltaX, double deltaY, double mouseX, double mouseY)
        : mDeltaX{deltaX}, mDeltaY{deltaY}, mMouseX{mouseX}, mMouseY{mouseY} {}

    /// @brief Return the amount scrolled horizontally, positive to the right and negative to the
    /// left.
    double getDeltaX() const { return mDeltaX; };

    /// @brief Return the amount scrolled vertically, positive away from the user and negative
    /// toward the user
    double getDeltaY() const { return mDeltaY; };

    std::string toString() const {
        return std::format("MouseScrolled: delta: {},{} pos  {},{}", mDeltaX, mDeltaY, mMouseX,
                           mMouseY);
    }

private:
    double mDeltaX{};
    double mDeltaY{};
    double mMouseX{};
    double mMouseY{};
};

/// @brief Event triggered when the mouse enter or leave the windows client area.
struct MouseEnterOrLeaveEvent {
    MouseEnterOrLeaveEvent(bool entered) : mEntered(entered) {}

    bool isEntered() const { return mEntered; }

    std::string toString() const {
        if (mEntered) {
            return std::string("MouseEnterOrLeaveEvent: entered");
        } else {
            return std::string("MouseEnterOrLeaveEvent: leaved");
        }
    }

private:
    bool mEntered{};
};

} // namespace Engine
