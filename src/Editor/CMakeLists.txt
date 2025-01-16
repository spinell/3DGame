add_executable(Editor
    main.cpp
    EditorLayer.h
    EditorLayer.cpp
    Panels/EditorPanel.h
    Panels/LogPanel.h
    Panels/LogPanel.cpp
    Panels/HierarchyPanel.cpp
    Panels/HierarchyPanel.h
    Panels/InspectorPanel.h
    Panels/InspectorPanel.cpp
    Panels/PanelManager.h
    Panels/PanelManager.cpp
)

target_link_libraries(Editor
    PRIVATE
        Engine::Engine
)

target_compile_definitions(Editor
    PRIVATE
        IMGUI_DEFINE_MATH_OPERATORS
)
