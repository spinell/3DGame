find_program(GLSLANG_EXE "glslang"
	DOC "Path to glslang/glslangValidator"
)
mark_as_advanced(FORCE GLSLANG_EXE)

if(GLSLANG_EXE)
  message(STATUS "glslang found: ${GLSLANG_EXE}")
else()
  message(FATAL_ERROR "glslang not found!")
endif()

#
#
# Arguments:
#   CPP_HEADER
#   PRIVATE         <list of shader file>
#   PUBLIC          <list of shader file>
#   INTERFACE       <list of shader file>
#   COMPILE_OPTIONS <list of glslang argument>
function(target_glsl_shaders TARGET_NAME)
	if(NOT TARGET ${TARGET_NAME})
		message(FATAL_ERROR "Target ${TARGET_NAME} notfound.")
	endif()

	# Parse arguments
	set(OPTIONS CPP_HEADER)
	set(SINGLE_VALUE_KEYWORDS)
	set(MULTI_VALUE_KEYWORDS INTERFACE PUBLIC PRIVATE COMPILE_OPTIONS)
	cmake_parse_arguments(PARSE_ARGV 1 arg "${OPTIONS}" "${SINGLE_VALUE_KEYWORDS}" "${MULTI_VALUE_KEYWORDS}")

	if(DEFINED arg_UNPARSED_ARGUMENTS)
		message(FATAL_ERROR "Unused paramater: ${arg_UNPARSED_ARGUMENTS}.")
	endif()
	if(DEFINED arg_KEYWORDS_MISSING_VALUES)
		message(FATAL_ERROR "Missing value for: ${arg_KEYWORDS_MISSING_VALUES}.")
	endif()

	# Get the target source directory so we can set glslang working directory
	# to this path. This will allow shader source file to be reletave.
	get_target_property(TARGET_SOURCE_DIR ${TARGET_NAME} SOURCE_DIR)

	# call glslang on each shader file
	foreach(GLSL_FILE IN LISTS arg_PRIVATE arg_PUBLIC arg_INTERFACE)

	    cmake_path(GET GLSL_FILE FILENAME SHADER_FILENAME)

		#
        # generate spirv file
		#
		#[[
		set(SPIRV_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/spirv/${SHADER_FILENAME}.spirv)

		set(GLSLANG_CMD ${GLSLANG_EXE} ${arg_COMPILE_OPTIONS} -V "${GLSL_FILE}" -o "${SPIRV_FILE_NAME}")
		add_custom_command(
			OUTPUT  ${SPIRV_FILE_NAME}
            COMMAND ${CMAKE_COMMAND} -E echo "Command: " ${GLSLANG_CMD}
			COMMAND ${GLSLANG_CMD}
			WORKING_DIRECTORY ${TARGET_SOURCE_DIR}
			MAIN_DEPENDENCY   ${GLSL_FILE}
			USES_TERMINAL
		)
		]]

		#
        # generate spirv c++ header
		#

		# The glslang command to run.
		set(GLSLANG_CMD)
		set(GENERATED_FILENAME)

		list(APPEND GLSLANG_CMD ${GLSLANG_EXE})
		list(APPEND GLSLANG_CMD ${arg_COMPILE_OPTIONS})
		list(APPEND GLSLANG_CMD -V)        # build for vulkan
		list(APPEND GLSLANG_CMD -e "main") # build for vulkan
		if(arg_CPP_HEADER)
			# build the name of the c++ header and the c++ array.
			# The c++ array will be the name of the shader file with pre appended "spirv_"
			# All "." is replaced with "_" to become a valid c++ name.
			# Ex: MyShader.vert.glsl -> spirv_myshader_vert_glsl
			# The name of the header will be the name of the c++ array with ".h"
			# Ex: spirv_myshader_vert_glsl -> spirv_myshader_vert_glsl.h
			string(REGEX REPLACE "\\.|-" "_" CPP_ARRAY_NAME spirv_${SHADER_FILENAME})
			string(TOLOWER ${CPP_ARRAY_NAME} CPP_ARRAY_NAME)

			list(APPEND GLSLANG_CMD --vn ${CPP_ARRAY_NAME}) # generate c++ header

			# The full path of the c++ header file
			# It will be the shader file name concatened with ".spirv.h"
			set(GENERATED_FILENAME ${CMAKE_CURRENT_BINARY_DIR}/generated/${CPP_ARRAY_NAME}.h)

			# Add the generated file name in the arget source.
			# This will force the generation before building anything else ...
			# See: https://discourse.cmake.org/t/how-to-install-an-interface-library-with-generated-source-in-build-folder/5449
			target_sources(${TARGET_NAME} PRIVATE ${GENERATED_FILENAME})
    	else()
			set(GENERATED_FILENAME ${CMAKE_CURRENT_BINARY_DIR}/spirv/${SHADER_FILENAME}.spirv)
		endif()

		list(APPEND GLSLANG_CMD -o ${GENERATED_FILENAME})  # The output file
		list(APPEND GLSLANG_CMD ${GLSL_FILE})              # The input file

		add_custom_command(
			OUTPUT  ${GENERATED_FILENAME}
			COMMAND ${CMAKE_COMMAND} -E echo "Command: " ${GLSLANG_CMD}
			COMMAND ${GLSLANG_CMD}
			WORKING_DIRECTORY ${TARGET_SOURCE_DIR}
			MAIN_DEPENDENCY   ${GLSL_FILE}
			USES_TERMINAL
		)

	endforeach()

	# Add all shader file to the target
	target_sources(${TARGET_NAME} PRIVATE   ${arg_PRIVATE})
	target_sources(${TARGET_NAME} PUBLIC    ${arg_PUBLIC})
    target_sources(${TARGET_NAME} INTERFACE ${arg_INTERFACE})

endfunction()
