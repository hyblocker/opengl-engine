# glad
message("Adding glad...")
project(glad C)
add_library(glad STATIC ${CMAKE_SOURCE_DIR}/vendor/glad/src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/vendor/glad/include)

# glfw
message("Adding glfw...")
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(glfw)

# fmt
message("Adding fmt...")
set(BUILD_SHARED_LIBS FALSE)
add_subdirectory(fmt)

# quill
message("Adding quill...")
set(QUILL_NO_EXCEPTIONS ON)
set(QUILL_X86ARCH OFF)
set(QUILL_DISABLE_NON_PREFIXED_MACROS ON)
add_subdirectory(quill)

# hlslpp
message("Adding hlslpp...")
set (CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/vendor/hlslpp")
include(hlslpp-config)

# eventbus
message("Adding EventBus...")
set(ENABLE_TEST OFF CACHE BOOL "Enable test")
set(ENABLE_PERFORMANCE OFF CACHE BOOL "Enable performance subproject")
set(ENABLE_LIBCXX OFF CACHE BOOL "Enable build with libc++")
add_subdirectory(EventBus)

# Dear ImGui
message("Adding ImGui...")
add_library(imgui STATIC
	${CMAKE_SOURCE_DIR}/vendor/imgui/imgui.cpp
	${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_demo.cpp
	${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_draw.cpp
	${CMAKE_SOURCE_DIR}/vendor/imgui/imgui_widgets.cpp
	${CMAKE_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_glfw.cpp
	${CMAKE_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui
	PUBLIC ${CMAKE_SOURCE_DIR}/vendor/imgui
	PUBLIC ${CMAKE_SOURCE_DIR}/vendor/imgui/backends)

# box2d
message("Adding box2D...")
set(BOX2D_ENABLE_SIMD ON CACHE BOOL "Enable SIMD math (faster)")
set(BOX2D_SANITIZE OFF CACHE BOOL "Enable sanitizers for some builds")
add_subdirectory(box2d)

# Assign dependencies to folders
set_property(TARGET glad PROPERTY FOLDER "vendor")

set_property(TARGET glfw PROPERTY FOLDER "vendor/glfw")
set_property(TARGET update_mappings PROPERTY FOLDER "vendor/glfw")

set_property(TARGET fmt PROPERTY FOLDER "vendor")

set_property(TARGET quill PROPERTY FOLDER "vendor")

set_property(TARGET EventBus PROPERTY FOLDER "vendor/EventBus")
set_property(TARGET UseCase_Basic PROPERTY FOLDER "vendor/EventBus")
set_property(TARGET UseCase_TaggedEvents PROPERTY FOLDER "vendor/EventBus")

set_property(TARGET box2d PROPERTY FOLDER "vendor")

set_property(TARGET imgui PROPERTY FOLDER "vendor")
