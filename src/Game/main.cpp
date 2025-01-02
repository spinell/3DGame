#include <Application.h>
#include <Entrypoint.h>

class GameApplication : public Engine::Application {
public:
    GameApplication() {}
    ~GameApplication() {}

private:

};

Engine::Application* Engine::CreateApplication() {
    return new GameApplication();
}