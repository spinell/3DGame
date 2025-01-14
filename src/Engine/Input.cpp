#include <Engine/Event.h>
#include <Engine/Input.h>
#include <SDL3/SDL.h>

#include <map>

namespace {
enum class KeyState { None = -1, Pressed, Held, Released };

std::map<Engine::KeyCode, KeyState>     sKeyStates;
std::map<Engine::MouseButton, KeyState> sMouseButtonStates;
std::pair<float, float>                 sMousePosition;
std::pair<float, float>                 sMouseDelta;
std::pair<float, float>                 sMouseWheel;

} // namespace

namespace Engine {

/// ==================================================================================
///                                 KeyBoard
/// ==================================================================================

bool Input::IsKeyPressed(KeyCode key) {
    return std::ranges::find_if(sKeyStates, [key](const auto& keyState) {
               return keyState.first == key && keyState.second == KeyState::Pressed;
           }) != sKeyStates.end();
}

bool Input::IsKeyDown(KeyCode key) {
    return std::ranges::find_if(sKeyStates, [key](const auto& keyState) {
               // check if it pressed this frame or held down because
               // the held state happen on the next frame.
               return keyState.first == key &&
                      (keyState.second == KeyState::Pressed || keyState.second == KeyState::Held);
           }) != sKeyStates.end();
}

bool Input::IsKeyReleased(KeyCode key) {
    return std::ranges::find_if(sKeyStates, [key](const auto& keyState) {
               return keyState.first == key && keyState.second == KeyState::Released;
           }) != sKeyStates.end();
}

/// ==================================================================================
///                                 Mouse
/// ==================================================================================

bool Input::IsButtonPressed(MouseButton button) {
    return std::ranges::find_if(sMouseButtonStates, [button](const auto& state) {
               return state.first == button && state.second == KeyState::Pressed;
           }) != sMouseButtonStates.end();
}

bool Input::IsButtonDown(MouseButton button) {
    return std::ranges::find_if(sMouseButtonStates, [button](const auto& state) {
               // check if it pressed this frame or held down because
               // the held state happen on the next frame.
               return state.first == button &&
                      (state.second == KeyState::Pressed || state.second == KeyState::Held);
           }) != sMouseButtonStates.end();
}

bool Input::IsButtonReleased(MouseButton button) {
    return std::ranges::find_if(sMouseButtonStates, [button](const auto& state) {
               return state.first == button && (state.second == KeyState::Released);
           }) != sMouseButtonStates.end();
}

std::pair<float, float> Input::GetMousePosition() { return sMousePosition; }

std::pair<float, float> Input::GetMousePositionDelta() { return sMouseDelta; }

std::pair<float, float> Input::GetMouseScrollDelta() { return sMouseWheel; }

/// ==================================================================================
///                          Internal functions
/// ==================================================================================

void Input::Update() {
    // Update the keyboard state
    for (auto& ks : sKeyStates) {
        if (ks.second == KeyState::Pressed) {
            ks.second = KeyState::Held;
        }
        if (ks.second == KeyState::Released) {
            ks.second = KeyState::None;
        }
    }

    // Update the mouse button state
    for (auto& mbs : sMouseButtonStates) {
        if (mbs.second == KeyState::Pressed) {
            mbs.second = KeyState::Held;
        }
        if (mbs.second == KeyState::Released) {
            mbs.second = KeyState::None;
        }
    }

    // reset mouse delta position and delta scroll
    sMouseDelta = {};
    sMouseWheel = {};
}

void Input::OnEvent(const Engine::Event& e) {
    //
    // Keybord
    //
    e.dispatch<KeyEvent>([](const KeyEvent& keyEvent) {
        if (keyEvent.isPressed()) {
            if (!keyEvent.isRepeated()) {
                // Mark the key as pressed
                // We can ignore the repeate state, the key will be marked has held down
                // on the next call to Update().
                sKeyStates[keyEvent.getKey()] = KeyState::Pressed;
            }
        } else {
            // Mark the key as pressed
            sKeyStates[keyEvent.getKey()] = KeyState::Released;
        }
    });

    //
    // Mouse
    //
    e.dispatch<MouseButtonEvent>([](const MouseButtonEvent& mouseButtonEvent) {
        if (mouseButtonEvent.isDown()) {
            // Mark the button as pressed
            sMouseButtonStates[mouseButtonEvent.getButton()] = KeyState::Pressed;
        } else {
            // Mark the button as released
            sMouseButtonStates[mouseButtonEvent.getButton()] = KeyState::Released;
        }
    });

    e.dispatch<MouseMovedEvent>([](const MouseMovedEvent& mouseMovedEvent) {
        sMousePosition = mouseMovedEvent.getMousePosition();
        sMouseDelta    = mouseMovedEvent.getMouseDelta();
    });

    e.dispatch<MouseScrolledEvent>([](const MouseScrolledEvent& mouseScrolledEvent) {
        sMouseWheel = mouseScrolledEvent.getDelta();
    });
}

}; // namespace Engine
