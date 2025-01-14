add_executable(Editor
    main.cpp
    EditorLayer.h
    EditorLayer.cpp
)

target_link_libraries(Editor
    PRIVATE
        Engine::Engine
)
