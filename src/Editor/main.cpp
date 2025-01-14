#include "EditorLayer.h"

#include <Engine/Application.h>
#include <Engine/Entrypoint.h>
#include <Engine/Log.h>

class EditorApplication : public Engine::Application {
public:
    EditorApplication() {
        ENGINE_INFO("EditorApplication()");
        pushLayer(new EditorLayer());
    }

    ~EditorApplication() { ENGINE_INFO("~EditorApplication()"); }

private:
};

Engine::Application* Engine::CreateApplication() { return new EditorApplication(); }
