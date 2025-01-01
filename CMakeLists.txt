cmake_minimum_required(VERSION 3.28)

include(cmake/OutOfSourceBuild.cmake)

project(3DGame
    VERSION 0.0.1
    DESCRIPTION "3D Game for learning purpose"
    HOMEPAGE_URL "https://github.com/spinell/3DGame"
    LANGUAGES CXX
)

# Fix Visual Studio Intellisence for system include with Clang
# from https://discourse.cmake.org/t/imported-projects-dont-resolve-include-paths-in-intellisense/9185
# CMAKE_INCLUDE_SYSTEM_FLAG_CXX is defined to -imsvc
if(MSVC_IDE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(WARNING "Unset CMAKE_INCLUDE_SYSTEM_FLAG_CXX and _CMAKE_INCLUDE_SYSTEM_FLAG_CXX_WARNING")
    message(WARNING "CMAKE_INCLUDE_SYSTEM_FLAG_CXX is set to \"${CMAKE_INCLUDE_SYSTEM_FLAG_CXX}\"")
    message(WARNING "_CMAKE_INCLUDE_SYSTEM_FLAG_CXX_WARNING is set to \"${_CMAKE_INCLUDE_SYSTEM_FLAG_CXX_WARNING}\"")
    unset(CMAKE_INCLUDE_SYSTEM_FLAG_CXX)
    unset(_CMAKE_INCLUDE_SYSTEM_FLAG_CXX_WARNING)
endif()

#
# Force using C++ 23 or higher
#
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TEST    "Build all the tests." OFF)
option(BUILD_DOC     "Build the documentation." OFF)


find_package(Vulkan REQUIRED MODULE
    COMPONENTS
        glslang
        glslc
        glslangValidator
        shaderc_combined
        SPIRV-Tools
        dxc
        volk
)


include(CMakePrintHelpers)
message(STATUS "Found vulkan                  ${Vulkan_FOUND}")
message(STATUS "Found vulkan headers          ${Vulkan_INCLUDE_DIRS}")
message(STATUS "Found vulkan libs             ${Vulkan_LIBRARIES}")
message(STATUS "Found vulkan version          ${Vulkan_VERSION}")
message(STATUS "Found vulkan glslc            ${Vulkan_glslc_FOUND}")
message(STATUS "Found vulkan glslc            ${Vulkan_GLSLC_EXECUTABLE}")
message(STATUS "Found vulkan glslangValidator ${Vulkan_glslangValidator_FOUND}")
message(STATUS "Found vulkan glslangValidator ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE}")
message(STATUS "Found vulkan shaderc          ${Vulkan_shaderc_combined_FOUND}")
message(STATUS "Found vulkan SPIRV-Tools      ${Vulkan_SPIRV-Tools_FOUND}")
message(STATUS "Found vulkan volk             ${Vulkan_volk_FOUND}")
message(STATUS "Found vulkan volk             ${Vulkan_volk_LIBRARY}")
message(STATUS "Found vulkan dxc lib          ${Vulkan_dxc_lib_FOUND}")
message(STATUS "Found vulkan dxc lib          ${Vulkan_dxc_LIBRARY}")
message(STATUS "Found vulkan dxc exe          ${Vulkan_dxc_exe_FOUND}")
message(STATUS "Found vulkan dxc exe          ${Vulkan_dxc_EXECUTABLE}")
cmake_print_properties(TARGETS Vulkan::volk             PROPERTIES LOCATION INTERFACE_INCLUDE_DIRECTORIES)
cmake_print_properties(TARGETS Vulkan::glslangValidator PROPERTIES LOCATION INTERFACE_INCLUDE_DIRECTORIES)
cmake_print_properties(TARGETS Vulkan::glslang          PROPERTIES LOCATION INTERFACE_INCLUDE_DIRECTORIES)

#
# Find dependencies
#
find_package(SDL2   2.30.10 EXACT CONFIG REQUIRED)
find_package(spdlog 1.15.0  EXACT CONFIG REQUIRED)
find_package(volk CONFIG REQUIRED)

add_subdirectory(src/Game)

install(TARGETS Game)

if(BUILD_TEST)
    # Enable testing for current directory and below.
    # This command should be in the source directory root because ctest expects
    # to find a test file in the build directory root.
    enable_testing()
    add_subdirectory(tests)
endif(BUILD_TEST)

if(BUILD_DOC)

endif(BUILD_DOC)
