
#include "Event.h"

namespace Engine {

/// @brief Base class for keybord event.
class KeyEvent : public Event {
public:
    /// @brief Retuirn key that was pressed.
    KeyCode GetKeyCode() const { return mKeyCode; }

protected:
    KeyEvent(KeyCode keycode) : mKeyCode(keycode) {}

    KeyCode mKeyCode{};
};

/// @brief Event triggered when a key was pressed.
class KeyPressedEvent : public KeyEvent {
public:
    EVENT_CLASS_TYPE(KeyPressed)

    KeyPressedEvent(KeyCode key, bool isRepeated, unsigned modifier)
        : KeyEvent{key}, mIsRepeated{isRepeated}, mModifier{modifier} {}

    /// @param Return true if the key is repeated.
    bool isRepeated() const { return mIsRepeated; }

    std::string toString() const {
        return std::format("{}: {} (repeat {}) (mod {})", getName(), (int)mKeyCode, mIsRepeated,
                           mModifier);
    }

private:
    bool     mIsRepeated{};
    unsigned mModifier{};
};

/// @brief Event triggered when a key was released.
class KeyReleasedEvent : public KeyEvent {
public:
    EVENT_CLASS_TYPE(KeyReleased)

    KeyReleasedEvent(KeyCode key, unsigned modifier) : KeyEvent{key}, mModifier{modifier} {}

    /// @brief Retuirn key that was release.
    KeyCode GetKeyCode() const { return mKeyCode; }

    std::string toString() const {
        return std::format("{}: {} (mod {})", getName(), (int)mKeyCode, mModifier);
    }

private:
    unsigned mModifier{};
};

/// @brief Keyboard text input
class InputTextEvent : public Event {
public:
    EVENT_CLASS_TYPE(InputText)

    InputTextEvent(const char* text) : mText(text) {}

    std::string toString() const { return std::format("{}: {}", getName(), mText); }

private:
    const char* mText;
};

} // namespace Engine
