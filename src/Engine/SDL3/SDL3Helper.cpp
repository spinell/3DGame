#include "SDL3Helper.h"

#include <Engine/Log.h>

namespace Engine {

MouseButton SDL3Helper::ConvertMouseButton(int button) {
    switch (button) {
            // clang-format off
            case SDL_BUTTON_LEFT:   return MouseButton::Left;
            case SDL_BUTTON_RIGHT:  return MouseButton::Right;
            case SDL_BUTTON_MIDDLE: return MouseButton::Middle;
            case SDL_BUTTON_X1:     return MouseButton::X4;
            case SDL_BUTTON_X2:     return MouseButton::X5;
            default:                return MouseButton::Unknow;
            // clang-format on
    }
}

KeyCode SDL3Helper::ConvertScanCode(SDL_Scancode scancode) {
    switch (scancode) {
            // clang-format off
        case SDL_SCANCODE_UNKNOWN:              return KeyCode::Unknown;
        case SDL_SCANCODE_A:                    return KeyCode::A;
        case SDL_SCANCODE_B:                    return KeyCode::B;
        case SDL_SCANCODE_C:                    return KeyCode::C;
        case SDL_SCANCODE_D:                    return KeyCode::D;
        case SDL_SCANCODE_E:                    return KeyCode::E;
        case SDL_SCANCODE_F:                    return KeyCode::F;
        case SDL_SCANCODE_G:                    return KeyCode::G;
        case SDL_SCANCODE_H:                    return KeyCode::H;
        case SDL_SCANCODE_I:                    return KeyCode::I;
        case SDL_SCANCODE_J:                    return KeyCode::J;
        case SDL_SCANCODE_K:                    return KeyCode::K;
        case SDL_SCANCODE_L:                    return KeyCode::L;
        case SDL_SCANCODE_M:                    return KeyCode::M;
        case SDL_SCANCODE_N:                    return KeyCode::N;
        case SDL_SCANCODE_O:                    return KeyCode::O;
        case SDL_SCANCODE_P:                    return KeyCode::P;
        case SDL_SCANCODE_Q:                    return KeyCode::Q;
        case SDL_SCANCODE_R:                    return KeyCode::R;
        case SDL_SCANCODE_S:                    return KeyCode::S;
        case SDL_SCANCODE_T:                    return KeyCode::T;
        case SDL_SCANCODE_U:                    return KeyCode::U;
        case SDL_SCANCODE_V:                    return KeyCode::V;
        case SDL_SCANCODE_W:                    return KeyCode::W;
        case SDL_SCANCODE_X:                    return KeyCode::X;
        case SDL_SCANCODE_Y:                    return KeyCode::Y;
        case SDL_SCANCODE_Z:                    return KeyCode::Z;
        case SDL_SCANCODE_1:                    return KeyCode::Key1;
        case SDL_SCANCODE_2:                    return KeyCode::Key2;
        case SDL_SCANCODE_3:                    return KeyCode::Key3;
        case SDL_SCANCODE_4:                    return KeyCode::Key4;
        case SDL_SCANCODE_5:                    return KeyCode::Key5;
        case SDL_SCANCODE_6:                    return KeyCode::Key6;
        case SDL_SCANCODE_7:                    return KeyCode::Key7;
        case SDL_SCANCODE_8:                    return KeyCode::Key8;
        case SDL_SCANCODE_9:                    return KeyCode::Key9;
        case SDL_SCANCODE_0:                    return KeyCode::Key0;
        case SDL_SCANCODE_RETURN:               return KeyCode::Return;
        case SDL_SCANCODE_ESCAPE:               return KeyCode::Escape;
        case SDL_SCANCODE_BACKSPACE:            return KeyCode::Backspace;
        case SDL_SCANCODE_TAB:                  return KeyCode::Tab;
        case SDL_SCANCODE_SPACE:                return KeyCode::Space;
        case SDL_SCANCODE_MINUS:                return KeyCode::Minus;
        case SDL_SCANCODE_EQUALS:               return KeyCode::Equals;
        case SDL_SCANCODE_LEFTBRACKET:          return KeyCode::LeftBracket;
        case SDL_SCANCODE_RIGHTBRACKET:         return KeyCode::RightBracket;
        case SDL_SCANCODE_BACKSLASH:            return KeyCode::Backslash;
        // case SDL_SCANCODE_NONUSHASH:			return KeyCode::NONUSHASH;
        case SDL_SCANCODE_SEMICOLON:            return KeyCode::Semicolon;
        case SDL_SCANCODE_APOSTROPHE:           return KeyCode::Apostrophe;
        case SDL_SCANCODE_GRAVE:                return KeyCode::Grave;
        case SDL_SCANCODE_COMMA:                return KeyCode::Comma;
        case SDL_SCANCODE_PERIOD:               return KeyCode::Period;
        case SDL_SCANCODE_SLASH:                return KeyCode::Slash;
        case SDL_SCANCODE_CAPSLOCK:             return KeyCode::Capslock;
        case SDL_SCANCODE_F1:                   return KeyCode::F1;
        case SDL_SCANCODE_F2:                   return KeyCode::F2;
        case SDL_SCANCODE_F3:                   return KeyCode::F3;
        case SDL_SCANCODE_F4:                   return KeyCode::F4;
        case SDL_SCANCODE_F5:                   return KeyCode::F5;
        case SDL_SCANCODE_F6:                   return KeyCode::F6;
        case SDL_SCANCODE_F7:                   return KeyCode::F7;
        case SDL_SCANCODE_F8:                   return KeyCode::F8;
        case SDL_SCANCODE_F9:                   return KeyCode::F9;
        case SDL_SCANCODE_F10:                  return KeyCode::F10;
        case SDL_SCANCODE_F11:                  return KeyCode::F11;
        case SDL_SCANCODE_F12:                  return KeyCode::F12;
        case SDL_SCANCODE_PRINTSCREEN:          return KeyCode::Printscreen;
        case SDL_SCANCODE_SCROLLLOCK:           return KeyCode::Scrolllock;
        case SDL_SCANCODE_PAUSE:                return KeyCode::Pause;
        case SDL_SCANCODE_INSERT:               return KeyCode::Insert;
        case SDL_SCANCODE_HOME:                 return KeyCode::Home;
        case SDL_SCANCODE_PAGEUP:               return KeyCode::PageUp;
        case SDL_SCANCODE_DELETE:               return KeyCode::Delete;
        case SDL_SCANCODE_END:                  return KeyCode::End;
        case SDL_SCANCODE_PAGEDOWN:             return KeyCode::PageDown;
        case SDL_SCANCODE_RIGHT:                return KeyCode::ArrowRight;
        case SDL_SCANCODE_LEFT:                 return KeyCode::ArrowLeft;
        case SDL_SCANCODE_DOWN:                 return KeyCode::ArrowDown;
        case SDL_SCANCODE_UP:                   return KeyCode::ArrowUp;
        case SDL_SCANCODE_NUMLOCKCLEAR:         return KeyCode::KeyPadNumlock;
        case SDL_SCANCODE_KP_DIVIDE:            return KeyCode::KeyPadDivide;
        case SDL_SCANCODE_KP_MULTIPLY:          return KeyCode::KeyPadMultiply;
        case SDL_SCANCODE_KP_MINUS:             return KeyCode::KeyPadMinus;
        case SDL_SCANCODE_KP_PLUS:              return KeyCode::KeyPadPlus;
        case SDL_SCANCODE_KP_ENTER:             return KeyCode::KeyPadEnter;
        case SDL_SCANCODE_KP_1:                 return KeyCode::KeyPad1;
        case SDL_SCANCODE_KP_2:                 return KeyCode::KeyPad2;
        case SDL_SCANCODE_KP_3:                 return KeyCode::KeyPad3;
        case SDL_SCANCODE_KP_4:                 return KeyCode::KeyPad4;
        case SDL_SCANCODE_KP_5:                 return KeyCode::KeyPad5;
        case SDL_SCANCODE_KP_6:                 return KeyCode::KeyPad6;
        case SDL_SCANCODE_KP_7:                 return KeyCode::KeyPad7;
        case SDL_SCANCODE_KP_8:                 return KeyCode::KeyPad8;
        case SDL_SCANCODE_KP_9:                 return KeyCode::KeyPad9;
        case SDL_SCANCODE_KP_0:                 return KeyCode::KeyPad0;
        case SDL_SCANCODE_KP_PERIOD:            return KeyCode::KeyPadPeriod;
        case SDL_SCANCODE_NONUSBACKSLASH:       return KeyCode::Unknown;
        case SDL_SCANCODE_APPLICATION:          return KeyCode::Application;
        case SDL_SCANCODE_POWER:                return KeyCode::Unknown;
        case SDL_SCANCODE_KP_EQUALS:            return KeyCode::Unknown;
        case SDL_SCANCODE_F13:                  return KeyCode::F13;
        case SDL_SCANCODE_F14:                  return KeyCode::F14;
        case SDL_SCANCODE_F15:                  return KeyCode::F15;
        case SDL_SCANCODE_F16:                  return KeyCode::F16;
        case SDL_SCANCODE_F17:                  return KeyCode::F17;
        case SDL_SCANCODE_F18:                  return KeyCode::F18;
        case SDL_SCANCODE_F19:                  return KeyCode::F19;
        case SDL_SCANCODE_F20:                  return KeyCode::F20;
        case SDL_SCANCODE_F21:                  return KeyCode::F21;
        case SDL_SCANCODE_F22:                  return KeyCode::F22;
        case SDL_SCANCODE_F23:                  return KeyCode::F23;
        case SDL_SCANCODE_F24:                  return KeyCode::F24;
        case SDL_SCANCODE_EXECUTE:              return KeyCode::Unknown;
        case SDL_SCANCODE_HELP:                 return KeyCode::Unknown;
        case SDL_SCANCODE_MENU:                 return KeyCode::Unknown;
        case SDL_SCANCODE_SELECT:               return KeyCode::Unknown;
        case SDL_SCANCODE_STOP:                 return KeyCode::Unknown;
        case SDL_SCANCODE_AGAIN:                return KeyCode::Unknown;
        case SDL_SCANCODE_UNDO:                 return KeyCode::Unknown;
        case SDL_SCANCODE_CUT:                  return KeyCode::Unknown;
        case SDL_SCANCODE_COPY:                 return KeyCode::Unknown;
        case SDL_SCANCODE_PASTE:                return KeyCode::Unknown;
        case SDL_SCANCODE_FIND:                 return KeyCode::Unknown;
        case SDL_SCANCODE_MUTE:                 return KeyCode::Mute;
        case SDL_SCANCODE_VOLUMEUP:             return KeyCode::VolumeUp;
        case SDL_SCANCODE_VOLUMEDOWN:           return KeyCode::VolumeDown;
        case SDL_SCANCODE_KP_COMMA:             return KeyCode::Unknown;
        case SDL_SCANCODE_KP_EQUALSAS400:       return KeyCode::Unknown;
        case SDL_SCANCODE_INTERNATIONAL1:       return KeyCode::Unknown;
        case SDL_SCANCODE_INTERNATIONAL2:       return KeyCode::Unknown;
        case SDL_SCANCODE_INTERNATIONAL3:       return KeyCode::Unknown;
        case SDL_SCANCODE_INTERNATIONAL4:       return KeyCode::Unknown;
        case SDL_SCANCODE_INTERNATIONAL5:       return KeyCode::Unknown;
        case SDL_SCANCODE_INTERNATIONAL6:       return KeyCode::Unknown;
        case SDL_SCANCODE_INTERNATIONAL7:       return KeyCode::Unknown;
        case SDL_SCANCODE_INTERNATIONAL8:       return KeyCode::Unknown;
        case SDL_SCANCODE_INTERNATIONAL9:       return KeyCode::Unknown;
        case SDL_SCANCODE_LANG1:                return KeyCode::Unknown;
        case SDL_SCANCODE_LANG2:                return KeyCode::Unknown;
        case SDL_SCANCODE_LANG3:                return KeyCode::Unknown;
        case SDL_SCANCODE_LANG4:                return KeyCode::Unknown;
        case SDL_SCANCODE_LANG5:                return KeyCode::Unknown;
        case SDL_SCANCODE_LANG6:                return KeyCode::Unknown;
        case SDL_SCANCODE_LANG7:                return KeyCode::Unknown;
        case SDL_SCANCODE_LANG8:                return KeyCode::Unknown;
        case SDL_SCANCODE_LANG9:                return KeyCode::Unknown;
        case SDL_SCANCODE_ALTERASE:             return KeyCode::Unknown;
        case SDL_SCANCODE_SYSREQ:               return KeyCode::Unknown;
        case SDL_SCANCODE_CANCEL:               return KeyCode::Unknown;
        case SDL_SCANCODE_CLEAR:                return KeyCode::Unknown;
        case SDL_SCANCODE_PRIOR:                return KeyCode::Unknown;
        case SDL_SCANCODE_RETURN2:              return KeyCode::Unknown;
        case SDL_SCANCODE_SEPARATOR:            return KeyCode::Unknown;
        case SDL_SCANCODE_OUT:                  return KeyCode::Unknown;
        case SDL_SCANCODE_OPER:                 return KeyCode::Unknown;
        case SDL_SCANCODE_CLEARAGAIN:           return KeyCode::Unknown;
        case SDL_SCANCODE_CRSEL:                return KeyCode::Unknown;
        case SDL_SCANCODE_EXSEL:                return KeyCode::Unknown;
        case SDL_SCANCODE_KP_00:                return KeyCode::Unknown;
        case SDL_SCANCODE_KP_000:               return KeyCode::Unknown;
        case SDL_SCANCODE_THOUSANDSSEPARATOR:   return KeyCode::Unknown;
        case SDL_SCANCODE_DECIMALSEPARATOR:     return KeyCode::Unknown;
        case SDL_SCANCODE_CURRENCYUNIT:         return KeyCode::Unknown;
        case SDL_SCANCODE_CURRENCYSUBUNIT:      return KeyCode::Unknown;
        case SDL_SCANCODE_KP_LEFTPAREN:         return KeyCode::Unknown;
        case SDL_SCANCODE_KP_RIGHTPAREN:        return KeyCode::Unknown;
        case SDL_SCANCODE_KP_LEFTBRACE:         return KeyCode::Unknown;
        case SDL_SCANCODE_KP_RIGHTBRACE:        return KeyCode::Unknown;
        case SDL_SCANCODE_KP_TAB:               return KeyCode::Unknown;
        case SDL_SCANCODE_KP_BACKSPACE:         return KeyCode::Unknown;
        case SDL_SCANCODE_KP_A:                 return KeyCode::Unknown;
        case SDL_SCANCODE_KP_B:                 return KeyCode::Unknown;
        case SDL_SCANCODE_KP_C:                 return KeyCode::Unknown;
        case SDL_SCANCODE_KP_D:                 return KeyCode::Unknown;
        case SDL_SCANCODE_KP_E:                 return KeyCode::Unknown;
        case SDL_SCANCODE_KP_F:                 return KeyCode::Unknown;
        case SDL_SCANCODE_KP_XOR:               return KeyCode::Unknown;
        case SDL_SCANCODE_KP_POWER:             return KeyCode::Unknown;
        case SDL_SCANCODE_KP_PERCENT:           return KeyCode::Unknown;
        case SDL_SCANCODE_KP_LESS:              return KeyCode::Unknown;
        case SDL_SCANCODE_KP_GREATER:           return KeyCode::Unknown;
        case SDL_SCANCODE_KP_AMPERSAND:         return KeyCode::Unknown;
        case SDL_SCANCODE_KP_DBLAMPERSAND:      return KeyCode::Unknown;
        case SDL_SCANCODE_KP_VERTICALBAR:       return KeyCode::Unknown;
        case SDL_SCANCODE_KP_DBLVERTICALBAR:    return KeyCode::Unknown;
        case SDL_SCANCODE_KP_COLON:             return KeyCode::Unknown;
        case SDL_SCANCODE_KP_HASH:              return KeyCode::Unknown;
        case SDL_SCANCODE_KP_SPACE:             return KeyCode::Unknown;
        case SDL_SCANCODE_KP_AT:                return KeyCode::Unknown;
        case SDL_SCANCODE_KP_EXCLAM:            return KeyCode::Unknown;
        case SDL_SCANCODE_KP_MEMSTORE:          return KeyCode::Unknown;
        case SDL_SCANCODE_KP_MEMRECALL:         return KeyCode::Unknown;
        case SDL_SCANCODE_KP_MEMCLEAR:          return KeyCode::Unknown;
        case SDL_SCANCODE_KP_MEMADD:            return KeyCode::Unknown;
        case SDL_SCANCODE_KP_MEMSUBTRACT:       return KeyCode::Unknown;
        case SDL_SCANCODE_KP_MEMMULTIPLY:       return KeyCode::Unknown;
        case SDL_SCANCODE_KP_MEMDIVIDE:         return KeyCode::Unknown;
        case SDL_SCANCODE_KP_PLUSMINUS:         return KeyCode::Unknown;
        case SDL_SCANCODE_KP_CLEAR:             return KeyCode::Unknown;
        case SDL_SCANCODE_KP_CLEARENTRY:        return KeyCode::Unknown;
        case SDL_SCANCODE_KP_BINARY:            return KeyCode::Unknown;
        case SDL_SCANCODE_KP_OCTAL:             return KeyCode::Unknown;
        case SDL_SCANCODE_KP_DECIMAL:           return KeyCode::Unknown;
        case SDL_SCANCODE_KP_HEXADECIMAL:       return KeyCode::Unknown;
        case SDL_SCANCODE_LCTRL:                return KeyCode::CtrlLeft;
        case SDL_SCANCODE_LSHIFT:               return KeyCode::ShiftLeft;
        case SDL_SCANCODE_LALT:                 return KeyCode::AltLeft;
        case SDL_SCANCODE_LGUI:                 return KeyCode::GuiLeft;
        case SDL_SCANCODE_RCTRL:                return KeyCode::CtrlRight;
        case SDL_SCANCODE_RSHIFT:               return KeyCode::ShiftRight;
        case SDL_SCANCODE_RALT:                 return KeyCode::AltRight;
        case SDL_SCANCODE_RGUI:                 return KeyCode::GuiRight;
        case SDL_SCANCODE_MODE:                 return KeyCode::Unknown;
        case SDL_SCANCODE_MEDIA_NEXT_TRACK:     return KeyCode::AudioNext;
        case SDL_SCANCODE_MEDIA_PREVIOUS_TRACK: return KeyCode::AudioPrev;
        case SDL_SCANCODE_MEDIA_STOP:           return KeyCode::AudioStop;
        case SDL_SCANCODE_MEDIA_PLAY:           return KeyCode::AudioPlay; // ???
        case SDL_SCANCODE_MEDIA_SELECT:         return KeyCode::MediaSelect;
        case SDL_SCANCODE_AC_SEARCH:            return KeyCode::Unknown;
        case SDL_SCANCODE_AC_HOME:              return KeyCode::Unknown;
        case SDL_SCANCODE_AC_BACK:              return KeyCode::Unknown;
        case SDL_SCANCODE_AC_FORWARD:           return KeyCode::Unknown;
        case SDL_SCANCODE_AC_STOP:              return KeyCode::Unknown;
        case SDL_SCANCODE_AC_REFRESH:           return KeyCode::Unknown;
        case SDL_SCANCODE_AC_BOOKMARKS:         return KeyCode::Unknown;
        case SDL_SCANCODE_MEDIA_EJECT:          return KeyCode::Unknown;
        case SDL_SCANCODE_MEDIA_PLAY_PAUSE:     return KeyCode::AudioPlay; // fixme
        case SDL_SCANCODE_SLEEP:                return KeyCode::Unknown;
        case SDL_SCANCODE_MEDIA_REWIND:         return KeyCode::Unknown;
        case SDL_SCANCODE_MEDIA_FAST_FORWARD:   return KeyCode::Unknown;
        default:                                return KeyCode::Unknown;
            // clang-format on
    }
}

MouseButtonFlag SDL3Helper::ConvertMouseButtonMask(SDL_MouseButtonFlags buttonMask) {
    MouseButtonFlag mask{};
    if ((buttonMask & SDL_BUTTON_LMASK) == SDL_BUTTON_LMASK) {
        mask |= MouseButtonFlag::Left;
    }
    if ((buttonMask & SDL_BUTTON_MMASK) == SDL_BUTTON_MMASK) {
        mask |= MouseButtonFlag::Middle;
    }
    if ((buttonMask & SDL_BUTTON_RMASK) == SDL_BUTTON_RMASK) {
        mask |= MouseButtonFlag::Right;
    }
    if ((buttonMask & SDL_BUTTON_X1MASK) == SDL_BUTTON_X1MASK) {
        mask |= MouseButtonFlag::X4;
    }
    if ((buttonMask & SDL_BUTTON_X2MASK) == SDL_BUTTON_X2MASK) {
        mask |= MouseButtonFlag::X5;
    }
    return mask;
}

KeyModFlag SDL3Helper::ConvertKeyModifierMask(SDL_Keymod keymod) {
    KeyModFlag mod{};
    if ((keymod & SDL_KMOD_LSHIFT) == SDL_KMOD_LSHIFT) {
        mod |= KeyModFlag::LeftShift;
    }
    if ((keymod & SDL_KMOD_RSHIFT) == SDL_KMOD_RSHIFT) {
        mod |= KeyModFlag::RightShift;
    }
    if ((keymod & SDL_KMOD_LCTRL) == SDL_KMOD_LCTRL) {
        mod |= KeyModFlag::LeftCtrl;
    }
    if ((keymod & SDL_KMOD_RCTRL) == SDL_KMOD_RCTRL) {
        mod |= KeyModFlag::RightCtrl;
    }
    if ((keymod & SDL_KMOD_LALT) == SDL_KMOD_LALT) {
        mod |= KeyModFlag::LeftAlt;
    }
    if ((keymod & SDL_KMOD_RALT) == SDL_KMOD_RALT) {
        mod |= KeyModFlag::RightAlt;
    }
    if ((keymod & SDL_KMOD_LGUI) == SDL_KMOD_LGUI) {
        mod |= KeyModFlag::LeftGUI;
    }
    if ((keymod & SDL_KMOD_RGUI) == SDL_KMOD_RGUI) {
        mod |= KeyModFlag::RightGUI;
    }
    if ((keymod & SDL_KMOD_NUM) == SDL_KMOD_NUM) {
        mod |= KeyModFlag::NumLock;
    }
    if ((keymod & SDL_KMOD_CAPS) == SDL_KMOD_CAPS) {
        mod |= KeyModFlag::CapsLock;
    }
    if ((keymod & SDL_KMOD_MODE) == SDL_KMOD_MODE) {
        // mod |= (KeyModFlag)SDL_KMOD_MODE;
    }
    if ((keymod & SDL_KMOD_SCROLL) == SDL_KMOD_SCROLL) {
        mod |= KeyModFlag::Scroll;
    }
    return mod;
}

MouseScrolledEvent SDL3Helper::ConvertEvent(const SDL_MouseWheelEvent& event) {
    const SDL_MouseWheelDirection direction = event.direction;
    const float                   deltaX = direction == SDL_MOUSEWHEEL_FLIPPED ? -event.x : event.x;
    const float                   deltaY = direction == SDL_MOUSEWHEEL_FLIPPED ? -event.y : event.y;
    return MouseScrolledEvent(deltaX, deltaY, event.mouse_x, event.mouse_y);
}

MouseMovedEvent SDL3Helper::ConvertEvent(const SDL_MouseMotionEvent& event) {
    const Engine::MouseButtonFlag buttonMask = ConvertMouseButtonMask(event.state);
    return Engine::MouseMovedEvent(event.x, event.y, event.xrel, event.yrel, buttonMask);
}

MouseButtonEvent SDL3Helper::ConvertEvent(const SDL_MouseButtonEvent& event) {
    const Engine::MouseButton button = SDL3Helper::ConvertMouseButton(event.button);
    return Engine::MouseButtonEvent(button, event.down, event.clicks, event.x, event.y);
}

KeyEvent SDL3Helper::ConvertEvent(const SDL_KeyboardEvent& event) {
    const KeyCode    keyCode = ConvertScanCode(event.scancode);
    const KeyModFlag mod     = ConvertKeyModifierMask(event.mod);
    return KeyEvent(keyCode, (int)event.scancode, event.down, event.repeat, mod);
}

std::optional<Event> SDL3Helper::ConvertEvent(const SDL_WindowEvent& event) {
    if (event.type == SDL_EVENT_WINDOW_SHOWN) {
        //
        // Do we need this ?
        //
        // ENGINE_CORE_DEBUG("[Window {}] SDL_EVENT_WINDOW_SHOWN", windowEvent.windowID);
    } else if (event.type == SDL_EVENT_WINDOW_HIDDEN) {
        //
        // Do we need this ?
        //
        // ENGINE_CORE_DEBUG("[Window {}] SDL_EVENT_WINDOW_HIDDEN", windowEvent.windowID);
    } else if (event.type == SDL_EVENT_WINDOW_EXPOSED) {
        //
        // Do we need this ?
        //
    } else if (event.type == SDL_EVENT_WINDOW_MOVED) {
        return WindowMovedEvent(event.data1, event.data2);
    } else if (event.type == SDL_EVENT_WINDOW_RESIZED) {
        return WindowResizedEvent(event.data1, event.data2);
    } else if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
        //
        // WTF is this event ?
        //
        // ENGINE_CORE_DEBUG("[Window {}] SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED {}x{}",
        // windowEvent.windowID, windowEvent.data1, windowEvent.data2);
    } else if (event.type == SDL_EVENT_WINDOW_METAL_VIEW_RESIZED) {
        //
        // WTF is this event ?
        //
        // ENGINE_CORE_DEBUG("[Window {}] SDL_EVENT_WINDOW_METAL_VIEW_RESIZED {}x{}",
        // windowEvent.windowID, windowEvent.data1, windowEvent.data2);
    } else if (event.type == SDL_EVENT_WINDOW_MINIMIZED) {
        return WindowMinimizedEvent();
    } else if (event.type == SDL_EVENT_WINDOW_MAXIMIZED) {
        return WindowMaximizedEvent();
    } else if (event.type == SDL_EVENT_WINDOW_RESTORED) {
        return WindowRestoredEvent();
    } else if (event.type == SDL_EVENT_WINDOW_MOUSE_ENTER) {
        return WindowMouseEnterEvent();
    } else if (event.type == SDL_EVENT_WINDOW_MOUSE_LEAVE) {
        return WindowMouseLeaveEvent();
    } else if (event.type == SDL_EVENT_WINDOW_FOCUS_GAINED) {
        return WindowFocusGainEvent();
    } else if (event.type == SDL_EVENT_WINDOW_FOCUS_LOST) {
        return WindowFocusLostEvent();
    } else if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
        return WindowCloseEvent();
    } else if (event.type == SDL_EVENT_WINDOW_HIT_TEST) {
        //
        // Do we need this ?
        //
        // ENGINE_CORE_DEBUG("[Window {}] SDL_EVENT_WINDOW_HIT_TEST", windowEvent.windowID);
    } else if (event.type == SDL_EVENT_WINDOW_ICCPROF_CHANGED) {
        //
        // Do we need this ?
        //
        // ENGINE_CORE_DEBUG("[Window {}] SDL_EVENT_WINDOW_ICCPROF_CHANGED", windowEvent.windowID);
    } else if (event.type == SDL_EVENT_WINDOW_DISPLAY_CHANGED) {
        return WindowDisplaydChangedEvent(event.data1);
    } else if (event.type == SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED) {
        //
        // Do we need this ?
        //
        // ENGINE_CORE_DEBUG("[Window {}] SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED {} {}",
        // windowEvent.windowID, windowEvent.data1, windowEvent.data2);
    } else if (event.type == SDL_EVENT_WINDOW_SAFE_AREA_CHANGED) {
        // What is this event ?????
        //
        // ENGINE_CORE_DEBUG("[Window {}] SDL_EVENT_WINDOW_SAFE_AREA_CHANGED {}x{}",
        // windowEvent.windowID, windowEvent.data1, windowEvent.data2);
    } else if (event.type == SDL_EVENT_WINDOW_OCCLUDED) {
        //
        // Look to do nothing ...
        //
        // ENGINE_CORE_DEBUG("[Window {}] SDL_EVENT_WINDOW_OCCLUDED", windowEvent.windowID);
    } else if (event.type == SDL_EVENT_WINDOW_ENTER_FULLSCREEN) {
        return WindowFullScreenEnterEvent();
    } else if (event.type == SDL_EVENT_WINDOW_LEAVE_FULLSCREEN) {
        return WindowFullScreenLeaveEvent();
    } else if (event.type == SDL_EVENT_WINDOW_DESTROYED) {
        // ENGINE_CORE_DEBUG("[Window {}] SDL_EVENT_WINDOW_DESTROYED", windowEvent.windowID);
    } else if (event.type == SDL_EVENT_WINDOW_HDR_STATE_CHANGED) {
        // ENGINE_CORE_DEBUG("[Window {}] SDL_EVENT_WINDOW_HDR_STATE_CHANGED",
        // windowEvent.windowID);
    }

    return {};
}

#if 0
Engine::KeyModFlag SDL3Helper::CaptureKeyModifier() {
    Engine::KeyModFlag KeyMod{};
    const Uint8*       keysState = SDL_GetKeyboardState(nullptr);
    if (keysState[SDL_SCANCODE_LSHIFT]) {
        KeyMod |= Engine::KeyModFlag::LeftShift;
    }
    if (keysState[SDL_SCANCODE_RSHIFT]) {
        KeyMod |= Engine::KeyModFlag::RightShift;
    }
    if (keysState[SDL_SCANCODE_LCTRL]) {
        KeyMod |= Engine::KeyModFlag::LeftCtrl;
    }
    if (keysState[SDL_SCANCODE_RCTRL]) {
        KeyMod |= Engine::KeyModFlag::RightCtrl;
    }
    if (keysState[SDL_SCANCODE_LALT]) {
        KeyMod |= Engine::KeyModFlag::LeftAlt;
    }
    if (keysState[SDL_SCANCODE_RALT]) {
        KeyMod |= Engine::KeyModFlag::RightAlt;
    }
    if (keysState[SDL_SCANCODE_LGUI]) {
        KeyMod |= Engine::KeyModFlag::LeftGUI;
    }
    if (keysState[SDL_SCANCODE_RGUI]) {
        KeyMod |= Engine::KeyModFlag::RightGUI;
    }
    if (keysState[SDL_SCANCODE_CAPSLOCK]) {
        KeyMod |= Engine::KeyModFlag::CapsLock;
    }
    if (keysState[SDL_SCANCODE_NUMLOCKCLEAR]) {
        KeyMod |= Engine::KeyModFlag::NumLock;
    }
    return KeyMod;
}
#endif

} // namespace Engine
