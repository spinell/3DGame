#include "SDLHelper.h"

namespace Engine {

Engine::MouseButton SDLHelper::ConvertMouseButton(int button) {
    switch (button) {
            // clang-format off
            case SDL_BUTTON_LEFT:   return Engine::MouseButton::Left;
            case SDL_BUTTON_RIGHT:  return Engine::MouseButton::Right;
            case SDL_BUTTON_MIDDLE: return Engine::MouseButton::Middle;
            case SDL_BUTTON_X1:     return Engine::MouseButton::X4;
            case SDL_BUTTON_X2:     return Engine::MouseButton::X5;
            default:                return Engine::MouseButton::Unknow;
            // clang-format on
    }
}

Engine::KeyModFlag SDLHelper::CaptureKeyModifier() {
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

} // namespace Engine
