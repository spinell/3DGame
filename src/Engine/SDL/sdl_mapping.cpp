#include "Engine/SDL/sdl_mapping.h"

Engine::KeyCode fromSDL(SDL_Scancode key) {
    switch (key) {
            // clang-format off
        case SDL_SCANCODE_UNKNOWN : return Engine::KeyCode::Unknown;
        case SDL_SCANCODE_A : return Engine::KeyCode::A;
        case SDL_SCANCODE_B : return Engine::KeyCode::B;
        case SDL_SCANCODE_C : return Engine::KeyCode::C;
        case SDL_SCANCODE_D : return Engine::KeyCode::D;
        case SDL_SCANCODE_E : return Engine::KeyCode::E;
        case SDL_SCANCODE_F : return Engine::KeyCode::F;
        case SDL_SCANCODE_G : return Engine::KeyCode::G;
        case SDL_SCANCODE_H : return Engine::KeyCode::H;
        case SDL_SCANCODE_I : return Engine::KeyCode::I;
        case SDL_SCANCODE_J : return Engine::KeyCode::J;
        case SDL_SCANCODE_K : return Engine::KeyCode::K;
        case SDL_SCANCODE_L : return Engine::KeyCode::L;
        case SDL_SCANCODE_M : return Engine::KeyCode::M;
        case SDL_SCANCODE_N : return Engine::KeyCode::N;
        case SDL_SCANCODE_O : return Engine::KeyCode::O;
        case SDL_SCANCODE_P : return Engine::KeyCode::P;
        case SDL_SCANCODE_Q : return Engine::KeyCode::Q;
        case SDL_SCANCODE_R : return Engine::KeyCode::R;
        case SDL_SCANCODE_S : return Engine::KeyCode::S;
        case SDL_SCANCODE_T : return Engine::KeyCode::T;
        case SDL_SCANCODE_U : return Engine::KeyCode::U;
        case SDL_SCANCODE_V : return Engine::KeyCode::V;
        case SDL_SCANCODE_W : return Engine::KeyCode::W;
        case SDL_SCANCODE_X : return Engine::KeyCode::X;
        case SDL_SCANCODE_Y : return Engine::KeyCode::Y;
        case SDL_SCANCODE_Z : return Engine::KeyCode::Z;
        case SDL_SCANCODE_1 : return Engine::KeyCode::Key1;
        case SDL_SCANCODE_2 : return Engine::KeyCode::Key2;
        case SDL_SCANCODE_3 : return Engine::KeyCode::Key3;
        case SDL_SCANCODE_4 : return Engine::KeyCode::Key4;
        case SDL_SCANCODE_5 : return Engine::KeyCode::Key5;
        case SDL_SCANCODE_6 : return Engine::KeyCode::Key6;
        case SDL_SCANCODE_7 : return Engine::KeyCode::Key7;
        case SDL_SCANCODE_8 : return Engine::KeyCode::Key8;
        case SDL_SCANCODE_9 : return Engine::KeyCode::Key9;
        case SDL_SCANCODE_0 : return Engine::KeyCode::Key0;
        case SDL_SCANCODE_F1 : return Engine::KeyCode::F1;
        case SDL_SCANCODE_F2 : return Engine::KeyCode::F2;
        case SDL_SCANCODE_F3 : return Engine::KeyCode::F3;
        case SDL_SCANCODE_F4 : return Engine::KeyCode::F4;
        case SDL_SCANCODE_F5 : return Engine::KeyCode::F5;
        case SDL_SCANCODE_F6 : return Engine::KeyCode::F6;
        case SDL_SCANCODE_F7 : return Engine::KeyCode::F7;
        case SDL_SCANCODE_F8 : return Engine::KeyCode::F8;
        case SDL_SCANCODE_F9 : return Engine::KeyCode::F9;
        case SDL_SCANCODE_F10 :return Engine::KeyCode::F10;
        case SDL_SCANCODE_F11 :return Engine::KeyCode::F11;
        case SDL_SCANCODE_F12 :return Engine::KeyCode::F12;
        case SDL_SCANCODE_NUMLOCKCLEAR:return Engine::KeyCode::KeyPadNumlock;
        case SDL_SCANCODE_KP_DIVIDE:   return Engine::KeyCode::KeyPadDivide;
        case SDL_SCANCODE_KP_MULTIPLY: return Engine::KeyCode::KeyPadMultiply;
        case SDL_SCANCODE_KP_MINUS:    return Engine::KeyCode::KeyPadMinus;
        case SDL_SCANCODE_KP_PLUS:     return Engine::KeyCode::KeyPadPlus;
        case SDL_SCANCODE_KP_ENTER:    return Engine::KeyCode::KeyPadEnter;
        case SDL_SCANCODE_KP_1: return Engine::KeyCode::KeyPad1;
        case SDL_SCANCODE_KP_2: return Engine::KeyCode::KeyPad2;
        case SDL_SCANCODE_KP_3: return Engine::KeyCode::KeyPad3;
        case SDL_SCANCODE_KP_4: return Engine::KeyCode::KeyPad4;
        case SDL_SCANCODE_KP_5: return Engine::KeyCode::KeyPad5;
        case SDL_SCANCODE_KP_6: return Engine::KeyCode::KeyPad6;
        case SDL_SCANCODE_KP_7: return Engine::KeyCode::KeyPad7;
        case SDL_SCANCODE_KP_8: return Engine::KeyCode::KeyPad8;
        case SDL_SCANCODE_KP_9: return Engine::KeyCode::KeyPad9;
        case SDL_SCANCODE_KP_0: return Engine::KeyCode::KeyPad0;
        case SDL_SCANCODE_KP_PERIOD: return Engine::KeyCode::KeyPadPeriod;
            // clang-format on
        default:
            return Engine::KeyCode::Unknown;
    }
}

unsigned convert_sdl_key_mod(SDL_Keymod sdlMod) {
    unsigned mod{};

    if ((sdlMod & KMOD_LSHIFT) == KMOD_LSHIFT) mod |= (int)Engine::KeyMod::LeftShift;

    if ((sdlMod & KMOD_RSHIFT) == KMOD_RSHIFT) mod |= (int)Engine::KeyMod::RightShift;

    if ((sdlMod & KMOD_LCTRL) == KMOD_LCTRL) mod |= (int)Engine::KeyMod::LeftCtrl;

    if ((sdlMod & KMOD_RCTRL) == KMOD_RCTRL) mod |= (int)Engine::KeyMod::RightCtrl;

    if ((sdlMod & KMOD_LALT) == KMOD_LALT) mod |= (int)Engine::KeyMod::LeftAlt;

    if ((sdlMod & KMOD_RALT) == KMOD_RALT) mod |= (int)Engine::KeyMod::RightAlt;

    if ((sdlMod & KMOD_LGUI) == KMOD_LGUI) mod |= (int)Engine::KeyMod::LeftGUI;

    if ((sdlMod & KMOD_RGUI) == KMOD_RGUI) mod |= (int)Engine::KeyMod::RightGUI;

    // else if((sdlMod & KMOD_NUM)    == KMOD_NUM)
    //	return spine::KeyMod::None;
    //
    // else if((sdlMod & KMOD_CAPS)   == KMOD_CAPS)
    //	return spine::KeyMod::None;
    //
    // else if((sdlMod & KMOD_MODE)   == KMOD_MODE)
    //	return spine::KeyMod::None;

    return mod;
}

Engine::MouseButton fromSDL(Uint32 button) {
    switch (button) {
            // clang-format off
        case SDL_BUTTON_LEFT:  return Engine::MouseButton::Left;
        case SDL_BUTTON_RIGHT: return Engine::MouseButton::Right;
        case SDL_BUTTON_MIDDLE:return Engine::MouseButton::Middle;
        case SDL_BUTTON_X1:    return Engine::MouseButton::X1;
        case SDL_BUTTON_X2:    return Engine::MouseButton::X2;
        default:               return Engine::MouseButton::Unknown;
            // clang-format on
    }
}

Engine::MouseButtonMask fromSDLButtonMask(Uint32 pButtonMask) {
    Engine::MouseButtonMask mask{};

    if ((pButtonMask & SDL_BUTTON_LMASK) == SDL_BUTTON_LMASK)
        mask = (Engine::MouseButtonMask)(mask | Engine::MouseButtonMask::Left);

    if ((pButtonMask & SDL_BUTTON_MMASK) == SDL_BUTTON_MMASK)
        mask = (Engine::MouseButtonMask)(mask | Engine::MouseButtonMask::Middle);

    if ((pButtonMask & SDL_BUTTON_RMASK) == SDL_BUTTON_RMASK)
        mask = (Engine::MouseButtonMask)(mask | Engine::MouseButtonMask::Right);

    if ((pButtonMask & SDL_BUTTON_X1MASK) == SDL_BUTTON_X1MASK)
        mask = (Engine::MouseButtonMask)(mask | Engine::MouseButtonMask::X1);

    if ((pButtonMask & SDL_BUTTON_X2MASK) == SDL_BUTTON_X2MASK)
        mask = (Engine::MouseButtonMask)(mask | Engine::MouseButtonMask::X2);

    return mask;
}

Engine::GameControllerButton fromSDL(SDL_GameControllerButton button) {
    return Engine::GameControllerButton::Unknown;
}

Engine::GameControllerAxis fromSDL(SDL_GameControllerAxis axis) {
    return Engine::GameControllerAxis::Unknown;
}
