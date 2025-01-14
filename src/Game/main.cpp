#include <Engine/Application.h>
#include <Engine/Entrypoint.h>
#include <Engine/Event.h>
#include <Engine/ImGuiLayer.h>
#include <Engine/Input.h>
#include <Engine/Layer.h>
#include <Engine/Log.h>
#include <imgui.h>

class TestLayer1 : public Engine::Layer {
public:
    TestLayer1(const char* name) : Engine::Layer(name) {}

    ~TestLayer1() {}

    void onAttach() override {}

    void onDetach() override {}

    void onUpdate(float timeStep) override {}

    void onImGuiRender() override {
        bool show_demo_window{true};
        ImGui::ShowDemoWindow(&show_demo_window);

        if (ImGui::Begin("Test")) {
            ImGui::Text("Hello !");
        }
        ImGui::End();
    }

    bool onEvent(const Engine::Event& event) override {
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
};

class GameApplication : public Engine::Application {
public:
    GameApplication() {
        ENGINE_INFO("GameApplication()");
        pushLayer(new TestLayer1("Layer1"));
    }

    ~GameApplication() { ENGINE_INFO("~GameApplication()"); }

private:
};

Engine::Application* Engine::CreateApplication() { return new GameApplication(); }
