#include "Application.h"
#include "Log.h"

#include <spdlog/spdlog.h>
#include <SDL2/SDL.h>

template <>
struct std::formatter<SDL_version> {

    template<class ParseContext>
    constexpr ParseContext::iterator parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template<class FmtContext>
    FmtContext::iterator format(const SDL_version& sdlVersion, FmtContext& ctx) const {
        return std::format_to(ctx.out(), "{}.{}.{}", sdlVersion.major, sdlVersion.minor, sdlVersion.patch);
    }
};

namespace {
    void printSDL2Info() {
        SDL_version sdlRuntimeVersion{};
        SDL_version sdlCompileVersion{};
        SDL_VERSION(&sdlCompileVersion);
        SDL_GetVersion(&sdlRuntimeVersion);


        ENGINE_CORE_INFO("SDL link    version {}", sdlCompileVersion);
        ENGINE_CORE_INFO("SDL runtime version {}", sdlRuntimeVersion);

        ENGINE_CORE_INFO("SDL has found {} audio driver.", SDL_GetNumAudioDrivers());
        for(int i = 0; i < SDL_GetNumAudioDrivers(); i++) {
            ENGINE_CORE_INFO(" - {}", SDL_GetAudioDriver(i));
        }

        ENGINE_CORE_INFO("SDL has found {} video driver.", SDL_GetNumVideoDrivers());
        for(int i = 0; i < SDL_GetNumVideoDrivers(); i++) {
            ENGINE_CORE_INFO(" - {}", SDL_GetVideoDriver(i));
        }

#ifdef SDL_AUDIO_DRIVER_SNDIO
    ENGINE_CORE_INFO("SDL_AUDIO_DRIVER_SNDIO {}", SDL_AUDIO_DRIVER_SNDIO);
    ENGINE_CORE_INFO("SDL_AUDIO_DRIVER_SNDIO_DYNAMIC  {}", SDL_AUDIO_DRIVER_SNDIO_DYNAMIC );
#endif
#ifdef SDL_AUDIO_DRIVER_ALSA
    ENGINE_CORE_INFO("SDL_AUDIO_DRIVER_ALSA {}",  SDL_AUDIO_DRIVER_ALSA);
    ENGINE_CORE_INFO("SDL_AUDIO_DRIVER_ALSA_DYNAMIC {}",  SDL_AUDIO_DRIVER_ALSA_DYNAMIC);
#endif
#ifdef SDL_AUDIO_DRIVER_NAS
    ENGINE_CORE_INFO("SDL_AUDIO_DRIVER_NAS {}",  SDL_AUDIO_DRIVER_NAS);
    ENGINE_CORE_INFO("SDL_AUDIO_DRIVER_NAS_DYNAMIC {}",  SDL_AUDIO_DRIVER_NAS_DYNAMIC);
#endif
#ifdef SDL_AUDIO_DRIVER_PULSEAUDIO
    ENGINE_CORE_INFO("SDL_AUDIO_DRIVER_PULSEAUDIO {}",  SDL_AUDIO_DRIVER_PULSEAUDIO);
    ENGINE_CORE_INFO("SDL_AUDIO_DRIVER_PULSEAUDIO_DYNAMIC {}",  SDL_AUDIO_DRIVER_PULSEAUDIO_DYNAMIC);
#endif
#ifdef SDL_AUDIO_DRIVER_PIPEWIRE
    ENGINE_CORE_INFO("SDL_AUDIO_DRIVER_PIPEWIRE {}",  SDL_AUDIO_DRIVER_PIPEWIRE);
    ENGINE_CORE_INFO("SDL_AUDIO_DRIVER_PIPEWIRE_DYNAMIC {}",  SDL_AUDIO_DRIVER_PIPEWIRE_DYNAMIC);
#endif
#ifdef SDL_AUDIO_DRIVER_JACK
    ENGINE_CORE_INFO("SDL_AUDIO_DRIVER_JACK {}", SDL_AUDIO_DRIVER_JACK);
    ENGINE_CORE_INFO("SDL_AUDIO_DRIVER_JACK_DYNAMIC {}", SDL_AUDIO_DRIVER_JACK_DYNAMIC);
#endif
#ifdef SDL_AUDIO_DRIVER_DISK
    ENGINE_CORE_INFO("SDL_AUDIO_DRIVER_DISK {}", SDL_AUDIO_DRIVER_DISK);
    //ENGINE_CORE_INFO("SDL_AUDIO_DRIVER_DISK_DYNAMIC {}",  SDL_AUDIO_DRIVER_DISK_DYNAMIC);
#endif
    }
}

Engine::Application::Application() {
    Engine::Log::Initialize();
    ENGINE_CORE_INFO("Engine::Application()");

    printSDL2Info();

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        ENGINE_CORE_CRITICAL("Failed to initialize SDL2 : {}", SDL_GetError());
        return;
    }

    mWindow = SDL_CreateWindow("Hello World!", 100, 100, 620, 387, SDL_WINDOW_SHOWN);
    if (mWindow == NULL) {
        ENGINE_CORE_CRITICAL("SDL2 failedto create the windows : {}", SDL_GetError());
        return;
    }
}

Engine::Application::~Application() {
    SDL_DestroyWindow(mWindow);
    SDL_Quit();

    ENGINE_CORE_INFO("Engine::~Application()");
    Engine::Log::Shutdown();
}

int Engine::Application::run() {
	onInit();

	while (mRunning) {
		processEvent();
	}

	onShutdown();
    return 0;
}

void Engine::Application::close() {
	mRunning = false;
}


void Engine::Application::processEvent() {
    SDL_Event event;

    // Events management
    while (SDL_PollEvent(&event)) {
        switch (event.type) {

        case SDL_QUIT:
            mRunning = false;
            break;
        case SDL_KEYDOWN:
            // keyboard API for key pressed
            switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP:
                break;
            case SDL_SCANCODE_A:
            case SDL_SCANCODE_LEFT:
                break;
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_DOWN:
                break;
            case SDL_SCANCODE_D:
            case SDL_SCANCODE_RIGHT:
                break;
            default:
                break;
            }
        }
    }
}
