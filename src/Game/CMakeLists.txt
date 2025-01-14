add_executable(Game
    main.cpp
    TestLayer.h
    TestLayer.cpp
)

target_link_libraries(Game
    PRIVATE
        Engine::Engine
)
