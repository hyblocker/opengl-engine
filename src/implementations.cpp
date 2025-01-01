// implement single header libs here

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"]

// imgui core lib
#include <imgui.cpp>
#include <imgui_demo.cpp>
#include <imgui_draw.cpp>
#include <imgui_tables.cpp>
#include <imgui_widgets.cpp>

// imgui helper
#include <misc/cpp/imgui_stdlib.cpp>

// imgui renderer
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <backends/imgui_impl_opengl3.cpp>
#include <backends/imgui_impl_glfw.cpp>