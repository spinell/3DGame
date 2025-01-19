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
        KeyCode.cpp
        KeyCode.h
        Event.h
        Layer.h
        LayerStack.h
        LayerStack.cpp
        ImGuiLayer.h
        ImGuiLayer.cpp
        Input.h
        Input.cpp
        SDL3/SDL3Window.h
        SDL3/SDL3Window.cpp
        SDL3/SDL3Helper.h
        SDL3/SDL3Helper.cpp
)

target_include_directories(Engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(Engine
    PUBLIC
        spdlog::spdlog # spdlog is exposed in Log.h
    PRIVATE
        #$<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
        SDL3::SDL3
        imgui::imgui
        OpenGL::GL
)


target_compile_options(Engine
    PUBLIC
        $<$<CXX_COMPILER_ID:MSVC>:/Zc:preprocessor>
)

###############################################################
#		Group sources for IDI like Visual Studio
###############################################################
get_target_property(Engine_SOURCES Engine SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${Engine_SOURCES})
