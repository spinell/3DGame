add_executable(Game
    main.cpp
    TestLayer.h
    TestLayer.cpp
)

target_link_libraries(Game
    PRIVATE
        Engine::Engine
)

###############################################################
#		Group sources for IDI like Visual Studio
###############################################################
get_target_property(Game_SOURCES Game SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${Game_SOURCES})
