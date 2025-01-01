add_executable(Game main.cpp
    Vulkan/VulkanUtils.cpp
    Vulkan/VulkanUtils.h
    Vulkan/VulkanRenderer.cpp
    Vulkan/VulkanRenderer.h
)

target_link_libraries(Game
    PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
        spdlog::spdlog
        volk::volk
)


#
# FIXME: UNIX will also catch osx
target_compile_definitions(Game
	PRIVATE
        # Define VK_NO_PROTOTYPES to avoid having function prototype from vulkan.h.
        VK_NO_PROTOTYPES
        $<$<BOOL:${WIN32}>:VK_USE_PLATFORM_WIN32_KHR>
        $<$<BOOL:${UNIX}>:VK_USE_PLATFORM_XLIB_KHR>
        $<$<BOOL:${UNIX}>:VK_USE_PLATFORM_XCB_KHR>
        $<$<BOOL:${UNIX}>:VK_USE_PLATFORM_WAYLAND_KHR>
)
