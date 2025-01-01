cmake_minimum_required (VERSION 3.8)

project(OpenGlEngine)
message("OpenGlEngine")

function(GroupSourcesByFolder target)
	set(SOURCE_GROUP_DELIMITER "/")
	set(last_dir "")
	set(files "")
	
	get_target_property(sources ${target} SOURCES)
	
  foreach(file ${sources})
	file(RELATIVE_PATH relative_file "${PROJECT_SOURCE_DIR}" ${file})
	get_filename_component(dir "${relative_file}" PATH)
		if(NOT "${dir}" STREQUAL "${last_dir}")
			if(files)
				source_group("${last_dir}" FILES ${files})
			endif()
			set(files "")
		endif()
		set(files ${files} ${file})
		set(last_dir "${dir}")
	endforeach()
	
	if(files)
		source_group("${last_dir}" FILES ${files})
	endif()
endfunction()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add source files
file(GLOB_RECURSE SOURCES_API ${CMAKE_SOURCE_DIR}/src "*.c" "*.h" "*.hpp" "*.cpp")
file(GLOB_RECURSE SOURCES_HEADERS ${CMAKE_SOURCE_DIR}/src "*.h" "*.hpp")

foreach(SOURCE IN ITEMS ${SOURCES_API})
	get_filename_component(SOURCE_PATH "${SOURCE}" PATH)
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP_PATH "${SOURCE_PATH}")
	string(REPLACE "/" "\\" GROUP_PATH "${GROUP_PATH}")
	source_group("${GROUP_PATH}" FILES "${SOURCE}")
endforeach()

add_executable(OpenGlEngine ${SOURCES_API} ${SOURCES_HEADERS})
GroupSourcesByFolder(OpenGlEngine)

target_include_directories(OpenGlEngine
	PUBLIC ${CMAKE_SOURCE_DIR}
	PUBLIC ${CMAKE_SOURCE_DIR}/src
	PUBLIC ${CMAKE_SOURCE_DIR}/vendor
	PUBLIC ${CMAKE_SOURCE_DIR}/vendor/glad/include
	PUBLIC ${CMAKE_SOURCE_DIR}/vendor/quill/include
	PUBLIC ${CMAKE_SOURCE_DIR}/vendor/fmt/include
	PUBLIC ${CMAKE_SOURCE_DIR}/vendor/hlslpp/include
	PUBLIC ${CMAKE_SOURCE_DIR}/vendor/box2d/include
	PUBLIC ${CMAKE_SOURCE_DIR}/vendor/imgui
)

target_link_libraries(OpenGlEngine
	PRIVATE glad
	PRIVATE fmt
	PRIVATE quill
	PRIVATE box2d
	PRIVATE imgui
	PRIVATE tinyobjloader
)

target_compile_definitions(OpenGlEngine
	PRIVATE NOMINMAX
	PRIVATE UNICODE
	PRIVATE NOGDI
	PRIVATE WIN32_LEAN_AND_MEAN
	PRIVATE HLSLPP_FEATURE_TRANSFORM
	PRIVATE QUILL_DISABLE_NON_PREFIXED_MACROS
)

# Compile definitions
target_compile_definitions(OpenGlEngine
	PRIVATE
	# If the debug configuration pass the DEBUG define to the compiler
	$<$<CONFIG:Debug>:ENABLE_ASSERT>
)

# Copy application assets to output folder
add_custom_command(
	TARGET OpenGlEngine
	PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/assets
	${CMAKE_BINARY_DIR}/artifacts/$<CONFIG>/assets
	COMMENT "Copying assets..."
)

# Set as startup project
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT OpenGlEngine)
set_property(TARGET OpenGlEngine PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/artifacts/$<CONFIG>")