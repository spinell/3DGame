#pragma once

namespace Engine
{

    /** */
    enum class KeyCode
    {
        Unknown,
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
        AudioMute,
        MediaSelect, // Key that allow to open audio application
        KeyCodeCount
    };

    enum KeyMod
    {
        None = 0x0,
        LeftShift = 0x1,
        RightShift = 0x2,
        LeftCtrl = 0x4,
        RightCtrl = 0x8,
        LeftAlt = 0x10,
        RightAlt = 0x20,
        LeftGUI = 0x40,
        RightGUI = 0x80
    };

    /** */
    inline const char *getKeyCodeString(KeyCode pKeyCode)
    {
        switch (pKeyCode)
        {
        case KeyCode::A:
            return "A";
        case KeyCode::B:
            return "B";
        case KeyCode::C:
            return "C";
        case KeyCode::D:
            return "D";
        case KeyCode::E:
            return "E";
        case KeyCode::F:
            return "F";
        case KeyCode::G:
            return "G";
        case KeyCode::H:
            return "H";
        case KeyCode::I:
            return "I";
        case KeyCode::J:
            return "J";
        case KeyCode::K:
            return "K";
        case KeyCode::L:
            return "L";
        case KeyCode::N:
            return "M";
        case KeyCode::M:
            return "N";
        case KeyCode::O:
            return "O";
        case KeyCode::P:
            return "P";
        case KeyCode::Q:
            return "Q";
        case KeyCode::R:
            return "R";
        case KeyCode::S:
            return "S";
        case KeyCode::T:
            return "T";
        case KeyCode::U:
            return "U";
        case KeyCode::V:
            return "V";
        case KeyCode::W:
            return "W";
        case KeyCode::X:
            return "X";
        case KeyCode::Y:
            return "Y";
        case KeyCode::Z:
            return "Z";
        case KeyCode::Key0:
            return "Key0";
        case KeyCode::Key1:
            return "Key1";
        case KeyCode::Key2:
            return "Key2";
        case KeyCode::Key3:
            return "Key3";
        case KeyCode::Key4:
            return "Key4";
        case KeyCode::Key5:
            return "Key5";
        case KeyCode::Key6:
            return "Key6";
        case KeyCode::Key7:
            return "Key7";
        case KeyCode::Key8:
            return "Key8";
        case KeyCode::Key9:
            return "Key9";
        case KeyCode::F1:
            return "F1";
        case KeyCode::F2:
            return "F2";
        case KeyCode::F3:
            return "F3";
        case KeyCode::F4:
            return "F4";
        case KeyCode::F5:
            return "F5";
        case KeyCode::F6:
            return "F6";
        case KeyCode::F7:
            return "F7";
        case KeyCode::F8:
            return "F8";
        case KeyCode::F9:
            return "F9";
        case KeyCode::F10:
            return "F10";
        case KeyCode::F11:
            return "F11";
        case KeyCode::F12:
            return "F12";
        case KeyCode::F13:
            return "F13";
        case KeyCode::F14:
            return "F14";
        case KeyCode::F15:
            return "F15";
        case KeyCode::F16:
            return "F16";
        case KeyCode::F17:
            return "F17";
        case KeyCode::F18:
            return "F18";
        case KeyCode::F19:
            return "F19";
        case KeyCode::F20:
            return "F20";
        case KeyCode::F21:
            return "F21";
        case KeyCode::F22:
            return "F22";
        case KeyCode::F23:
            return "F23";
        case KeyCode::F24:
            return "F24";
        case KeyCode::KeyPad0:
            return "KeyPad0";
        case KeyCode::KeyPad1:
            return "KeyPad1";
        case KeyCode::KeyPad2:
            return "KeyPad2";
        case KeyCode::KeyPad3:
            return "KeyPad3";
        case KeyCode::KeyPad4:
            return "KeyPad4";
        case KeyCode::KeyPad5:
            return "KeyPad5";
        case KeyCode::KeyPad6:
            return "KeyPad6";
        case KeyCode::KeyPad7:
            return "KeyPad7";
        case KeyCode::KeyPad8:
            return "KeyPad8";
        case KeyCode::KeyPad9:
            return "KeyPad9";
        case KeyCode::KeyPadDivide:
            return "KeyPadDivide";
        case KeyCode::KeyPadEnter:
            return "KeyPadEnter";
        case KeyCode::KeyPadMinus:
            return "KeyPadMinus";
        case KeyCode::KeyPadMultiply:
            return "KeyPadMultiply";
        case KeyCode::KeyPadNumlock:
            return "KeyPadNumlock";
        case KeyCode::KeyPadPeriod:
            return "KeyPadPeriod";
        case KeyCode::KeyPadPlus:
            return "KeyPadPlus";

        case KeyCode::ArrowDown:
            return "ArrowDown";
        case KeyCode::ArrowLeft:
            return "ArrowLeft";
        case KeyCode::ArrowRight:
            return "ArrowRight";
        case KeyCode::ArrowUp:
            return "ArrowUp";

        case KeyCode::Return:
            return "Return";
        case KeyCode::Escape:
            return "Escape";
        case KeyCode::Backspace:
            return "Backspace";
        case KeyCode::Tab:
            return "Tab";
        case KeyCode::Space:
            return "Space";
        case KeyCode::Minus:
            return "Minus";
        case KeyCode::Equals:
            return "Equals";
        case KeyCode::LeftBracket:
            return "LeftBracket";
        case KeyCode::RightBracket:
            return "RightBracket";
        case KeyCode::Backslash:
            return "Backslash";
        case KeyCode::Semicolon:
            return "Semicolon";
        case KeyCode::Apostrophe:
            return "Apostrophe";
        case KeyCode::Grave:
            return "Grave";
        case KeyCode::Comma:
            return "Comma";
        case KeyCode::Period:
            return "Period";
        case KeyCode::Slash:
            return "Slash";
        case KeyCode::Capslock:
            return "Capslock";

        case KeyCode::Printscreen:
            return "Printscreen";
        case KeyCode::Scrolllock:
            return "Scrolllock";
        case KeyCode::Pause:
            return "Pause";
        case KeyCode::Insert:
            return "Insert";
        case KeyCode::Home:
            return "Home";
        case KeyCode::PageUp:
            return "PageUp";
        case KeyCode::Delete:
            return "Delete";
        case KeyCode::End:
            return "End";
        case KeyCode::PageDown:
            return "PageDown";

        case KeyCode::CtrlLeft:
            return "CtrlLeft";
        case KeyCode::ShiftLeft:
            return "ShiftLeft";
        case KeyCode::AltLeft:
            return "AltLeft";
        case KeyCode::GuiLeft:
            return "GuiLeft";
        case KeyCode::CtrlRight:
            return "CtrlRight";
        case KeyCode::ShiftRight:
            return "ShiftRight";
        case KeyCode::AltRight:
            return "AltRight";
        case KeyCode::GuiRight:
            return "GuiRight";

        case KeyCode::VolumeUp:
            return "VolumeUp";
        case KeyCode::VolumeDown:
            return "VolumeDown";
        case KeyCode::AudioMute:
            return "AudioMute";
        case KeyCode::AudioNext:
            return "AudioNext";
        case KeyCode::AudioPlay:
            return "AudioPlay";
        case KeyCode::AudioPrev:
            return "AudioPrev";
        case KeyCode::AudioStop:
            return "AudioStop";
        case KeyCode::MediaSelect:
            return "MediaSelect";

        case KeyCode::Application:
            return "Application";

        case KeyCode::Unknown:
        default:
            return "Unknown";
        }
    }

    enum class MouseButton
    {
        Unknown,
        Left,
        Right,
        Middle,
        X1,
        X2,
        Count,
    };

    enum MouseButtonMask
    {
        Left = 1 << 0,
        Right = 1 << 1,
        Middle = 1 << 2,
        X1 = 1 << 3,
        X2 = 1 << 4,
    };

    enum class GameControllerButton
    {
        Unknown,
        BUTTON_A,
        BUTTON_B,
        BUTTON_X,
        BUTTON_Y,
        BUTTON_BACK,
        BUTTON_GUIDE,
        BUTTON_START,
        BUTTON_LEFTSTICK,
        BUTTON_RIGHTSTICK,
        BUTTON_LEFTSHOULDER,
        BUTTON_RIGHTSHOULDER,
        BUTTON_DPAD_UP,
        BUTTON_DPAD_DOWN,
        BUTTON_DPAD_LEFT,
        BUTTON_DPAD_RIGHT,
        Count
    };

    enum class GameControllerAxis
    {
        Unknown,
        LeftX,
        LeftY,
        RightX,
        RightY,
        TriggerLeft,
        TriggerRight,
        Count
    };

}; // namespace Engine
