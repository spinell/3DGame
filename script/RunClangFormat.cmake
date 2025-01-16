################################################################################
#
# Run Clang-format on the source tree
# Call with cmake in script mode (cmake -P)
#
################################################################################

# Check executable exists
find_program(CLANG_FORMAT_EXECUTABLE clang-format)
if(CLANG_FORMAT_EXECUTABLE-NOTFOUND)
    message(FATAL_ERROR "clang-format not found!")
endif()

cmake_path(GET CMAKE_SCRIPT_MODE_FILE PARENT_PATH  parent_dir)
set(root ${parent_dir}/..)

set(SOURCES "")
foreach(FOLDER IN ITEMS ${root}/src/Engine ${root}/src/Editor ${root}/src/Game)
    file(GLOB_RECURSE folder_files "${FOLDER}/*.h" "${FOLDER}/*.hpp" "${FOLDER}/*.inl" "${FOLDER}/*.cpp" "${FOLDER}/*.mm" "${FOLDER}/*.m")
    list(APPEND SOURCES ${folder_files})
endforeach()

foreach(file IN ITEMS ${SOURCES})
    if(NOT ${file} MATCHES "fonts")
        execute_process(
            COMMAND ${CLANG_FORMAT_EXECUTABLE} -i --style=file --verbose ${file}
        )
    endif()
endforeach()
