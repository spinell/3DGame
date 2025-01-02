#include <Engine/Application.h>
#include <Engine/Entrypoint.h>
#include <Engine/Log.h>

class GameApplication : public Engine::Application {
public:
    GameApplication() {
        ENGINE_INFO("GameApplication()");
    }

    ~GameApplication() {
        ENGINE_INFO("~GameApplication()");
    }

private:

};

Engine::Application* Engine::CreateApplication() {
    return new GameApplication();
}