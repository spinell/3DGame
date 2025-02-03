find_package(Vulkan REQUIRED COMPONENTS glslc glslangValidator shaderc_combined SPIRV-Tools)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

add_executable(Game
    main.cpp
    TestLayer.h
    TestLayer.cpp
    vulkan/vulkan.h
    vulkan/VulkanContext.h
    vulkan/VulkanContext.cpp
    vulkan/VulkanUtils.h
    vulkan/VulkanUtils.cpp
    vulkan/vma/vma.h
    vulkan/vma/vma_build.cpp
    vulkan/vma/vma_custom_configuration.h
    )

target_link_libraries(Game
    PRIVATE
        Engine::Engine
        GPUOpen::VulkanMemoryAllocator
        Vulkan::Vulkan
        Vulkan::Headers
)

target_compile_definitions(Game
    PRIVATE
        VK_USE_PLATFORM_WIN32_KHR
)


###############################################################
#		Group sources for IDI like Visual Studio
###############################################################
get_target_property(Game_SOURCES Game SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${Game_SOURCES})
