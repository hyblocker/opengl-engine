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

# box2d
message("Adding box2D...")
set(BOX2D_ENABLE_SIMD ON CACHE BOOL "Enable SIMD math (faster)")
set(BOX2D_SANITIZE OFF CACHE BOOL "Enable sanitizers for some builds")
add_subdirectory(box2d)

# tinyobj
message("Adding tinyobjloader...")
set(TINYOBJLOADER_USE_DOUBLE OFF CACHE BOOL "Build library with double precision instead of single (float)")
set(TINYOBJLOADER_WITH_PYTHON OFF CACHE BOOL "Build Python module(for developer). Use pyproject.toml/setup.py to build Python module for end-users")
set(TINYOBJLOADER_PREFER_LOCAL_PYTHON_INSTALLATION OFF CACHE BOOL "Prefer locally-installed Python interpreter than system or conda/brew installed Python. Please specify your Python interpreter   with `Python3_EXECUTABLE` cmake option if you enable this option.")
set(TINYOBJLOADER_BUILD_TEST_LOADER OFF CACHE BOOL "Build Example Loader Application")
set(TINYOBJLOADER_BUILD_OBJ_STICHER OFF CACHE BOOL "Build OBJ Sticher Application")
add_subdirectory(tinyobjloader)

# Assign dependencies to folders
set_property(TARGET glad PROPERTY FOLDER "vendor")

set_property(TARGET glfw PROPERTY FOLDER "vendor/glfw")
set_property(TARGET update_mappings PROPERTY FOLDER "vendor/glfw")

set_property(TARGET fmt PROPERTY FOLDER "vendor")

set_property(TARGET quill PROPERTY FOLDER "vendor")

set_property(TARGET box2d PROPERTY FOLDER "vendor")

set_property(TARGET tinyobjloader PROPERTY FOLDER "vendor/tinyobj")
set_property(TARGET uninstall PROPERTY FOLDER "vendor/tinyobj")