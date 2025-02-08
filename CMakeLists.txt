cmake_minimum_required(VERSION 3.28)

include(cmake/OutOfSourceBuild.cmake)
include(cmake/glslang_compiler.cmake)

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

#
# Find dependencies
#
find_package(SDL3   3.1.6   EXACT CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

find_package(spdlog 1.15.0  EXACT CONFIG REQUIRED)
find_package(glm    1.0.1   EXACT CONFIG REQUIRED)
find_package(entt   3.14.0  EXACT CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(Stb)
find_package(unofficial-spirv-reflect CONFIG REQUIRED)

add_subdirectory(src/Engine)
add_subdirectory(src/Editor)
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
