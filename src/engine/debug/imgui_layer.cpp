#include "imgui_layer.hpp"

#include <imgui.h>
#include <imgui_internal.h>

// imgui implementation
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "engine/app.hpp"

// most of the code for setting up imgui itself is taken from vendor/imgui/examples/example_glfw_opengl3/main.cpp
namespace engine {
    ImguiLayer::ImguiLayer(gpu::DeviceManager* deviceManager, managers::AssetManager* assetManager)
        : ILayer(deviceManager, assetManager, "ImGuiLayer")
    {}

    void ImguiLayer::attach() {


        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        GLFWwindow* glfwWindow = reinterpret_cast<GLFWwindow*>(engine::App::getInstance()->getWindow()->getHandle());

        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
        ImGui_ImplOpenGL3_Init("#version 330 core");
    }

    void ImguiLayer::detach() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImguiLayer::event(events::Event& event) {
        if (m_eventsBlocked) {
            ImGuiIO& io = ImGui::GetIO();
            event.handled |= event.isInCategory(events::EventCategory_Mouse) & io.WantCaptureMouse;
            event.handled |= event.isInCategory(events::EventCategory_Keyboard) & io.WantCaptureKeyboard;
        }
    }

    void ImguiLayer::begin() {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImguiLayer::end() {

        // resize imgui window
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(
            (float) engine::App::getInstance()->getWindow()->getWidth(),
            (float) engine::App::getInstance()->getWindow()->getHeight()
        );

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    uint32_t ImguiLayer::getActiveWidgetId() const {
        return GImGui->ActiveId;
    }
}