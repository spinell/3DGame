#pragma once
#include <string>
#include <type_traits>

namespace Engine {

/// @brief Mouse button.
enum class MouseButton : unsigned short {
    Left   = 1,
    Right  = 2,
    Middle = 3,
    X4     = 4,
    X5     = 5,
    X6     = 6,
    X7     = 7,
    X8     = 8,
    Count,
    Unknow = 0

};

/// @brief Mouse Button mask
enum class MouseButtonFlag : unsigned short {
    Unknow = 0,
    Left   = 1 << static_cast<unsigned short>(MouseButton::Left),
    Right  = 1 << static_cast<unsigned short>(MouseButton::Right),
    Middle = 1 << static_cast<unsigned short>(MouseButton::Middle),
    X4     = 1 << static_cast<unsigned short>(MouseButton::X4),
    X5     = 1 << static_cast<unsigned short>(MouseButton::X5),
    X6     = 1 << static_cast<unsigned short>(MouseButton::X6),
    X7     = 1 << static_cast<unsigned short>(MouseButton::X7),
    X8     = 1 << static_cast<unsigned short>(MouseButton::X8),
};

constexpr inline MouseButtonFlag operator|(MouseButtonFlag a, MouseButtonFlag b) {
    using type = std::underlying_type_t<MouseButtonFlag>;
    return static_cast<MouseButtonFlag>(static_cast<type>(a) | static_cast<type>(b));
}
constexpr inline MouseButtonFlag& operator|=(MouseButtonFlag& a, MouseButtonFlag b) {
    return a = a | b;
}
constexpr inline bool operator&(MouseButtonFlag a, MouseButtonFlag b) {
    using type = std::underlying_type_t<MouseButtonFlag>;
    return static_cast<bool>(static_cast<type>(a) & static_cast<type>(b));
}

enum class KeyCode {
    Unknown = -1,
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,

    Key0,
    Key1,
    Key2,
    Key3,
    Key4,
    Key5,
    Key6,
    Key7,
    Key8,
    Key9,

    Return,
    Escape,
    Backspace,
    Tab,
    Space,
    Minus,
    Equals,
    LeftBracket,
    RightBracket,
    Backslash,
    Semicolon,
    Apostrophe,
    Grave,
    Comma,
    Period,
    Slash,
    Capslock,

    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    F21,
    F22,
    F23,
    F24,

    Printscreen,
    Scrolllock,
    Pause,
    Insert,
    Home,
    PageUp,
    Delete,
    End,
    PageDown,

    ArrowRight,
    ArrowLeft,
    ArrowDown,
    ArrowUp,

    // Keypad
    KeyPadNumlock,
    KeyPadDivide,
    KeyPadMultiply,
    KeyPadMinus,
    KeyPadPlus,
    KeyPadEnter,
    KeyPad0,
    KeyPad1,
    KeyPad2,
    KeyPad3,
    KeyPad4,
    KeyPad5,
    KeyPad6,
    KeyPad7,
    KeyPad8,
    KeyPad9,
    KeyPadPeriod,

    Application,
    VolumeUp,   // Key that allow to raise up the sound
    VolumeDown, // Key that allow to raise down the sound
    CtrlLeft,
    CtrlRight,
    ShiftLeft,
    ShiftRight,
    AltLeft,
    AltRight, // SDL generate Ctrl-left than Alt-right
    GuiLeft,
    GuiRight,
    AudioNext,
    AudioPrev,
    AudioStop,
    AudioPlay,
    Mute,
    MediaSelect, // Key that allow to open audio application
    Count
};

/// @brief Key modifier
enum class KeyModFlag : unsigned short {
    None       = 0,
    LeftShift  = 1 << 0, // left Shift key is down
    RightShift = 1 << 1, // right Shift key is down
    LeftCtrl   = 1 << 2, // the left Ctrl (Control) key is down.
    RightCtrl  = 1 << 3, // the right Ctrl (Control) key is down
    LeftAlt    = 1 << 4, // the left Alt key is down
    RightAlt   = 1 << 5, // the right Alt key is down
    LeftGUI    = 1 << 6, // the left GUI key (often the Windows key) is down
    RightGUI   = 1 << 7, // the right GUI key (often the Windows key) is down
    NumLock    = 1 << 8, // the Num Lock key (may be located on an extended keypad) is down.
    CapsLock   = 1 << 9, // the Caps Lock key is down
    Scroll     = 1 << 10 // the Scroll Lock key is down.
};

constexpr inline KeyModFlag operator|(KeyModFlag a, KeyModFlag b) {
    using type = std::underlying_type_t<KeyModFlag>;
    return static_cast<KeyModFlag>(static_cast<type>(a) | static_cast<type>(b));
}
constexpr inline KeyModFlag& operator|=(KeyModFlag& a, KeyModFlag b) { return a = a | b; }
constexpr inline bool        operator&(KeyModFlag a, KeyModFlag b) {
    using type = std::underlying_type_t<KeyModFlag>;
    return static_cast<bool>(static_cast<type>(a) & static_cast<type>(b));
}

/// @brief Return the name of a MouseButton
const char* toString(MouseButton button);
std::string toString(MouseButtonFlag mask);
std::string toString(KeyModFlag mask);

} // namespace Engine
