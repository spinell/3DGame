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

target_include_directories(Engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(Engine
    PUBLIC
        spdlog::spdlog # spdlog is exposed in Log.h
    PRIVATE
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
)


target_compile_options(Engine
    PUBLIC
        $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>
)


