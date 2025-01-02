add_library(Engine STATIC)
add_library(Engine::Engine ALIAS Engine)

target_sources(Engine
    PRIVATE
        Application.cpp
        Application.h
        Assert_.h
        Log.cpp
        Log.h
        Entrypoint.h
)

target_include_directories(Engine PUBLIC .)

target_link_libraries(Engine
    PRIVATE
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
        spdlog::spdlog
)


target_compile_options(Engine
    PUBLIC
        /Zc:preprocessor
)


