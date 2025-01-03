#pragma once
#include "Application.h"

namespace Engine {
extern Application* CreateApplication();

int main(int argc, char* argv[]) {
    Engine::Application* app    = Engine::CreateApplication();
    const int            retVal = app->run();
    delete app;
    return retVal;
}
} // namespace Engine

#ifdef _WIN32

//
// Entrypoint for Windows
//
// Define both WinMain and main.
// The linker will pick up the right one base on /Subsystem flag

#include <Windows.h>

int CALLBACK WinMain(_In_ HINSTANCE hInstance,
                     _In_ HINSTANCE hPrevInstance,
                     _In_ LPSTR     lpCmdLine,
                     _In_ int       nCmdShow) {
    return Engine::main(__argc, __argv);
}

int main(int argc, char* argv[]) { return Engine::main(argc, argv); }

#else

//
// Entrypoint for other platform
//

int main(int argc, char* argv[]) { return Engine::main(argc, argv); }

#endif
