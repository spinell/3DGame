#include "TestLayer.h"

#include <Engine/Application.h>
#include <Engine/Event.h>
#include <Engine/Input.h>
#include <Engine/Layer.h>
#include <Engine/Log.h>
#include <imgui.h>
#include "vulkan/VulkanContext.h"

TestLayer1::TestLayer1(const char* name) : Engine::Layer(name) {}

TestLayer1::~TestLayer1() {}

void TestLayer1::onAttach() {
    VulkanContext::Initialize();
}

void TestLayer1::onDetach() {
    VulkanContext::Shutdown();
}

void TestLayer1::onUpdate(float timeStep) {}

void TestLayer1::onImGuiRender() {}

bool TestLayer1::onEvent(const Engine::Event& event) {
    event.dispatch<Engine::KeyEvent>([](const Engine::KeyEvent& e) {
        if (e.isPressed()) {
            if (e.getKey() == Engine::KeyCode::Escape) {
                Engine::Application::Get().close();
            }
            if (e.getKey() == Engine::KeyCode::Key1) {
                Engine::Application::Get().GetWindow().setFullScreen(true);
            }
            if (e.getKey() == Engine::KeyCode::Key2) {
                Engine::Application::Get().GetWindow().setFullScreen(false);
            }
            if (e.getKey() == Engine::KeyCode::KeyPad0) {
                Engine::Application::Get().GetWindow().toogleMouseGrab();
            }
            if (e.getKey() == Engine::KeyCode::KeyPad1) {
                Engine::Application::Get().GetWindow().toogleMouseRelativeMode();
            }
        }
    });
    event.dispatch<Engine::MouseButtonEvent>([](const Engine::MouseButtonEvent& e) {
        // ENGINE_INFO("{}: {}", __FUNCTION__, e.toString());
    });
    event.dispatch<Engine::MouseMovedEvent>([](const Engine::MouseMovedEvent& e) {
        // ENGINE_INFO("{}: {}", __FUNCTION__, e.toString());
    });
    return false;
}
