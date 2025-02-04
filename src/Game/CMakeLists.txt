find_package(Vulkan REQUIRED COMPONENTS glslc glslangValidator shaderc_combined SPIRV-Tools)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

add_executable(Game
    main.cpp
    TestLayer.h
    TestLayer.cpp
    SpirV/SpirvReflection.h
    SpirV/SpirvReflection.cpp
    vulkan/vulkan.h
    vulkan/VulkanContext.h
    vulkan/VulkanContext.cpp
    vulkan/VulkanUtils.h
    vulkan/VulkanUtils.cpp
    vulkan/VulkanSwapchain.h
    vulkan/VulkanSwapchain.cpp
    vulkan/vma/vma.h
    vulkan/vma/vma_build.cpp
    vulkan/vma/vma_custom_configuration.h
)

#
# Add shaders
#
target_glsl_shaders(Game
	CPP_HEADER
	PRIVATE
		Shaders/fullscreen_quad.vert.glsl
		Shaders/fullscreen_quad.frag.glsl
        Shaders/triangle.vert.glsl
        Shaders/triangle.frag.glsl
)

target_include_directories(Game
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/generated
)


target_link_libraries(Game
    PRIVATE
        Engine::Engine
        GPUOpen::VulkanMemoryAllocator
        Vulkan::Vulkan
        Vulkan::Headers
        unofficial::spirv-reflect
)

target_compile_definitions(Game
    PRIVATE
        VK_USE_PLATFORM_WIN32_KHR
)


###############################################################
#		Group sources for IDI like Visual Studio
###############################################################
#get_target_property(Game_SOURCES Game SOURCES)
#source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${Game_SOURCES})
