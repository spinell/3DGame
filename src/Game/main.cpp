#include <SDL2/SDL.h>
#include <spdlog/spdlog.h>
#include "spdlog/fmt/ostr.h" // must be included

#include <cstdlib>
#include <iostream>
#include <format>
#include <print>

#if SPDLOG_USE_STD_FORMAT
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
#else
std::ostream& operator<<(std::ostream& os, const SDL_version& sdlVersion)
{
    return os << std::format("{}.{}.{}", sdlVersion.major, sdlVersion.minor, sdlVersion.patch);
}

template <> struct fmt::formatter<SDL_version> : fmt::ostream_formatter {};
#endif

int main(int argc, char* argv[]) {
    std::print("{0} {2}{1}!\n", "Hello", 23, "C++");

    SDL_version sdlRuntimeVersion{};
    SDL_version sdlCompileVersion{};
    SDL_VERSION(&sdlCompileVersion);
    SDL_GetVersion(&sdlRuntimeVersion);

#if SPDLOG_USE_STD_FORMAT
    spdlog::info("Hello from spdlog (std::format)!");
    spdlog::info("SDL link    version {}", sdlCompileVersion);
    spdlog::info("SDL runtime version {}", sdlRuntimeVersion);
#else
    spdlog::info("Hello from spdlog (fmt lib)!");
    spdlog::info("SDL link    version {}", sdlCompileVersion);
    spdlog::info("SDL runtime version {}", sdlRuntimeVersion);
#endif
    std::cout << std::format("SDL link    version {}", sdlCompileVersion) << std::endl;
    std::cout << std::format("SDL runtime version {}", sdlRuntimeVersion) << std::endl;

#ifdef SDL_AUDIO_DRIVER_SNDIO
    std::cout << std::format("SDL_AUDIO_DRIVER_SNDIO {}", SDL_AUDIO_DRIVER_SNDIO) << std::endl;
    std::cout << std::format("SDL_AUDIO_DRIVER_SNDIO_DYNAMIC  {}", SDL_AUDIO_DRIVER_SNDIO_DYNAMIC ) << std::endl;
#endif
#ifdef SDL_AUDIO_DRIVER_ALSA
    std::cout << std::format("SDL_AUDIO_DRIVER_ALSA {}",  SDL_AUDIO_DRIVER_ALSA) << std::endl;
    std::cout << std::format("SDL_AUDIO_DRIVER_ALSA_DYNAMIC {}",  SDL_AUDIO_DRIVER_ALSA_DYNAMIC) << std::endl;
#endif
#ifdef SDL_AUDIO_DRIVER_NAS
    std::cout << std::format("SDL_AUDIO_DRIVER_NAS {}",  SDL_AUDIO_DRIVER_NAS) << std::endl;
    std::cout << std::format("SDL_AUDIO_DRIVER_NAS_DYNAMIC {}",  SDL_AUDIO_DRIVER_NAS_DYNAMIC) << std::endl;
#endif
#ifdef SDL_AUDIO_DRIVER_PULSEAUDIO
    std::cout << std::format("SDL_AUDIO_DRIVER_PULSEAUDIO {}",  SDL_AUDIO_DRIVER_PULSEAUDIO) << std::endl;
    std::cout << std::format("SDL_AUDIO_DRIVER_PULSEAUDIO_DYNAMIC {}",  SDL_AUDIO_DRIVER_PULSEAUDIO_DYNAMIC) << std::endl;
#endif
#ifdef SDL_AUDIO_DRIVER_PIPEWIRE
    std::cout << std::format("SDL_AUDIO_DRIVER_PIPEWIRE {}",  SDL_AUDIO_DRIVER_PIPEWIRE) << std::endl;
    std::cout << std::format("SDL_AUDIO_DRIVER_PIPEWIRE_DYNAMIC {}",  SDL_AUDIO_DRIVER_PIPEWIRE_DYNAMIC) << std::endl;
#endif
#ifdef SDL_AUDIO_DRIVER_JACK
    std::cout << std::format("SDL_AUDIO_DRIVER_JACK {}",  SDL_AUDIO_DRIVER_JACK) << std::endl;
    std::cout << std::format("SDL_AUDIO_DRIVER_JACK_DYNAMIC {}",  SDL_AUDIO_DRIVER_JACK_DYNAMIC) << std::endl;
#endif
#ifdef SDL_AUDIO_DRIVER_DISK
    std::cout << std::format("SDL_AUDIO_DRIVER_DISK {}",  SDL_AUDIO_DRIVER_DISK) << std::endl;
    //std::cout << std::format("SDL_AUDIO_DRIVER_DISK_DYNAMIC {}",  SDL_AUDIO_DRIVER_DISK_DYNAMIC) << std::endl;
#endif

    for(int i = 0; i < SDL_GetNumAudioDrivers(); i++) {
        std::cout << std::format("Audio Driver # {} : {}", i, SDL_GetAudioDriver(i)) << std::endl;
    }

    for(int i = 0; i < SDL_GetNumVideoDrivers(); i++) {
        std::cout << std::format("Video Driver # {} : {}", i, SDL_GetVideoDriver(i)) << std::endl;
    }

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    SDL_Window* win = SDL_CreateWindow("Hello World!", 100, 100, 620, 387, SDL_WINDOW_SHOWN);
    if (win == NULL) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

    int close = 0;
   while (!close) {
        SDL_Event event;

        // Events management
        while (SDL_PollEvent(&event)) {
            switch (event.type) {

            case SDL_QUIT:
                // handling of close button
                close = 1;
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

    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}
