find_package(Vulkan REQUIRED COMPONENTS glslc glslangValidator shaderc_combined SPIRV-Tools)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

find_program(SLANGC_EXE "slangc"
	DOC "Path to slangc"
)
mark_as_advanced(FORCE SLANGC_EXE)

if(SLANGC_EXE)
  message(STATUS "slangc found: ${SLANGC_EXE}")
else()
  message(FATAL_ERROR "slangc not found!")
endif()

add_executable(Game
    main.cpp
    TestLayer.h
    TestLayer.cpp
    SpirV/SpirvReflection.h
    SpirV/SpirvReflection.cpp
    Camera.h
    Camera.cpp
    CameraController.h
    CameraController.cpp
    GeometryGenerator.h
    GeometryGenerator.cpp
    Mesh.h
    Mesh.cpp
    Renderer.h
    Renderer.cpp
    SceneRenderer.h
    SceneRenderer.cpp
    AssimpImporter.h
    AssimpImporter.cpp
    Terrain.h
    Terrain.cpp
    vulkan/VulkanBuffer.cpp
    vulkan/VulkanBuffer.h
    vulkan/VulkanGraphicPipeline.h
    vulkan/VulkanGraphicPipeline.cpp
    vulkan/VulkanTexture.h
    vulkan/VulkanTexture.cpp
    vulkan/VulkanImGuiRenderer.h
    vulkan/VulkanImGuiRenderer.cpp
    vulkan/vulkan.h
    vulkan/VulkanContext.h
    vulkan/VulkanContext.cpp
    vulkan/VulkanUtils.h
    vulkan/VulkanUtils.cpp
    vulkan/VulkanSwapchain.h
    vulkan/VulkanSwapchain.cpp
    vulkan/VulkanDescriptorPool.cpp
    vulkan/VulkanDescriptorPool.h
    vulkan/VulkanShaderProgram.cpp
    vulkan/VulkanShaderProgram.h
    vulkan/vma/vma.h
    vulkan/vma/vma_build.cpp
    vulkan/vma/vma_custom_configuration.h
)

target_include_directories(Game
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}/generated
        ${Stb_INCLUDE_DIR}
)


target_link_libraries(Game
    PRIVATE
        GPUOpen::VulkanMemoryAllocator
        Vulkan::Vulkan
        Vulkan::Headers
        Vulkan::shaderc_combined
        unofficial::spirv-reflect
        glm::glm-header-only
        EnTT::EnTT
        Engine::Engine
        assimp::assimp
)

target_compile_definitions(Game
    PRIVATE
        VK_USE_PLATFORM_WIN32_KHR
        GLM_FORCE_RADIANS
        GLM_FORCE_DEPTH_ZERO_TO_ONE
)

############################################################################################################
#									Add Shader files
############################################################################################################
target_sources(Game
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/mesh.slang
        ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/mesh_aabb.slang
        ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/mesh_show_normals.slang
        ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/fullscreen.slang
        ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/skybox.slang
        ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/terrain.slang
        ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/include/buffers.slang
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/shaders/mesh_vert.spv
        ${CMAKE_CURRENT_BINARY_DIR}/shaders/mesh_frag.spv
    COMMAND ${SLANGC_EXE} -warnings-as-errors 39019 -fvk-invert-y -O0 ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/mesh.slang -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/mesh_vert.spv -stage vertex -entry vs_main
    COMMAND ${SLANGC_EXE} -warnings-as-errors 39019 -fvk-invert-y -O0 ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/mesh.slang -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/mesh_frag.spv -stage pixel  -entry ps_main
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/mesh.slang
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/include/buffers.slang
    VERBATIM
    USES_TERMINAL
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/shaders/mesh_show_normals_vert.spv
        ${CMAKE_CURRENT_BINARY_DIR}/shaders/mesh_show_normals_geo.spv
        ${CMAKE_CURRENT_BINARY_DIR}/shaders/mesh_show_normals_frag.spv
    COMMAND ${SLANGC_EXE} -warnings-as-errors 39019 -fvk-invert-y -O0 ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/mesh_show_normals.slang -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/mesh_show_normals_vert.spv -entry vs_main
    COMMAND ${SLANGC_EXE} -warnings-as-errors 39019 -fvk-invert-y -O0 ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/mesh_show_normals.slang -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/mesh_show_normals_geo.spv  -entry gs_main
    COMMAND ${SLANGC_EXE} -warnings-as-errors 39019 -fvk-invert-y -O0 ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/mesh_show_normals.slang -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/mesh_show_normals_frag.spv -entry ps_main
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/mesh_show_normals.slang
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/include/buffers.slang
    VERBATIM
    USES_TERMINAL
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/shaders/fullscreen_vert.spv
        ${CMAKE_CURRENT_BINARY_DIR}/shaders/fullscreen_frag.spv
    COMMAND ${SLANGC_EXE} -warnings-as-errors 39019 ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/fullscreen.slang -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/fullscreen_vert.spv -stage vertex -entry vs_main
    COMMAND ${SLANGC_EXE} -warnings-as-errors 39019 ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/fullscreen.slang -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/fullscreen_frag.spv -stage pixel  -entry ps_main
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/fullscreen.slang
    VERBATIM
    USES_TERMINAL
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/shaders/skybox_vert.spv
        ${CMAKE_CURRENT_BINARY_DIR}/shaders/skybox_frag.spv
    COMMAND ${SLANGC_EXE} -warnings-as-errors 39019 -fvk-invert-y ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/skybox.slang -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/skybox_vert.spv -stage vertex -entry vs_main
    COMMAND ${SLANGC_EXE} -warnings-as-errors 39019 -fvk-invert-y ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/skybox.slang -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/skybox_frag.spv -stage pixel  -entry ps_main
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/skybox.slang
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/include/buffers.slang
    VERBATIM
    USES_TERMINAL
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/shaders/mesh_aabb_vert.spv
        ${CMAKE_CURRENT_BINARY_DIR}/shaders/mesh_aabb_geo.spv
        ${CMAKE_CURRENT_BINARY_DIR}/shaders/mesh_aabb_frag.spv
    COMMAND ${SLANGC_EXE} -warnings-as-errors 39019 -fvk-invert-y ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/mesh_aabb.slang -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/mesh_aabb_vert.spv -entry vs_main
    COMMAND ${SLANGC_EXE} -warnings-as-errors 39019 -fvk-invert-y ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/mesh_aabb.slang -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/mesh_aabb_geo.spv  -entry gs_main
    COMMAND ${SLANGC_EXE} -warnings-as-errors 39019 -fvk-invert-y ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/mesh_aabb.slang -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/mesh_aabb_frag.spv -entry ps_main
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/mesh_aabb.slang
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/include/buffers.slang
    VERBATIM
    USES_TERMINAL
)

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/shaders/terrain_vert.spv
        ${CMAKE_CURRENT_BINARY_DIR}/shaders/terrain_hull.spv
        ${CMAKE_CURRENT_BINARY_DIR}/shaders/terrain_dom.spv
        ${CMAKE_CURRENT_BINARY_DIR}/shaders/terrain_frag.spv
    COMMAND ${SLANGC_EXE} -warnings-as-errors 39019 -fvk-invert-y -O0 ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/terrain.slang -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/terrain_vert.spv -entry vs_main
    COMMAND ${SLANGC_EXE} -warnings-as-errors 39019 -fvk-invert-y -O0 ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/terrain.slang -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/terrain_hull.spv -entry hs_main
    COMMAND ${SLANGC_EXE} -warnings-as-errors 39019 -fvk-invert-y -O0 ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/terrain.slang -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/terrain_dom.spv  -entry ds_main
    COMMAND ${SLANGC_EXE} -warnings-as-errors 39019 -fvk-invert-y -O0 ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/terrain.slang -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/terrain_frag.spv -entry ps_main
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/terrain.slang
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/include/buffers.slang
    VERBATIM
    USES_TERMINAL
)

#
# Add shaders
#
#[[
target_glsl_shaders(Game
	CPP_HEADER
	PRIVATE
		Shaders/fullscreen_quad.vert.glsl
		Shaders/fullscreen_quad.frag.glsl
        Shaders/triangle.vert.glsl
        Shaders/triangle.frag.glsl
        Shaders/triangle_tex.vert.glsl
        Shaders/triangle_tex.frag.glsl
        Shaders/mesh.vert.glsl
        Shaders/mesh.frag.glsl
)
]]
############################################################################################################
#									Copy shaders and data
############################################################################################################
ADD_CUSTOM_COMMAND (
    TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink  ${CMAKE_CURRENT_SOURCE_DIR}/data $<TARGET_FILE_DIR:Game>/data
    VERBATIM
)
get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(is_multi_config)
    ADD_CUSTOM_COMMAND (
        TARGET Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink  ${CMAKE_CURRENT_BINARY_DIR}/shaders $<TARGET_FILE_DIR:Game>/shaders
        VERBATIM
    )
endif()

###############################################################
#		Group sources for IDI like Visual Studio
###############################################################
#get_target_property(Game_SOURCES Game SOURCES)
#source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${Game_SOURCES})

set_property(TARGET Game PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:Game>)
