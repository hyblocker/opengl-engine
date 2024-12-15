#include "window.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>

#include "core.hpp"

#include "gpu/idevice.hpp"
#include "engine/events.hpp"
#include "engine/log.hpp"

// for some reason this is defined in the internal header in GLFW, instead of exposing internal data we shall just redefine the constant here.
#define GL_CONTEXT_FLAGS 0x821e

// Initialise to 0 windows
uint32_t Window::s_windowCount = 0;

Window::Window(WindowDesc desc, int32_t openglMajor, int32_t openglMinor, std::shared_ptr<dexode::EventBus>& eventBus)
    : m_desc(desc),
    m_windowHandle(0),
    m_openglMajor(openglMajor),
    m_openglMinor(openglMinor),
    m_eventBus(eventBus)
{
}

Window::~Window() {
    close();
}

#if _DEBUG
// Forward declare debug hook
void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam);
#endif

void Window::createNativeWindow() {
    if (s_windowCount == 0) {
        // Init GLFW
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, m_openglMajor);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, m_openglMinor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if _DEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif
    }

    GLFWwindow* window = glfwCreateWindow(m_desc.width, m_desc.height, m_desc.title.c_str(), NULL, NULL);
    if (window == NULL) {
        LOG_FATAL("Failed to create GLFW window");
        glfwTerminate();
        return;
    }

    // Store window handle
    m_windowHandle = reinterpret_cast<uintptr_t>(window);
    glfwSetWindowUserPointer(window, this);
    
    if (s_windowCount == 0) {
        // Load GLAD
        glfwMakeContextCurrent(window);
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
            LOG_FATAL("Failed to initialise GLAD");
            return;
        }
    }

#if _DEBUG
    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
#endif

    s_windowCount++;

    glfwSetFramebufferSizeCallback(window, Window::onResizeCallback);
}

void Window::close() const {
    glfwSetWindowShouldClose(reinterpret_cast<GLFWwindow*>(m_windowHandle), true);
    s_windowCount--;
    if (s_windowCount == 0) {
        shutdown();
    }
}

void Window::shutdown() const {
    ASSERT(s_windowCount == 0);
    glfwTerminate();
}

void Window::setTitle(std::string& title) {
    m_desc.title = title;
    glfwSetWindowTitle(reinterpret_cast<GLFWwindow*>(m_windowHandle), title.c_str());
}

void Window::onResizeCallback(GLFWwindow* glfwWindow, int width, int height) {

    // update window desc to have new dimensions
    Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
    window->m_desc.width = width;
    window->m_desc.height = height;

    // send callback to inform other parts of the engine
    engine::events::EventWindowResize eventData{
        .newWidth = static_cast<uint32_t>(width),
        .newHeight = static_cast<uint32_t>(height),
    };
    window->m_eventBus->postpone(eventData);
}

int Window::shouldCloseWindow() const {
    return glfwWindowShouldClose(reinterpret_cast<GLFWwindow*>(m_windowHandle));
}

void Window::windowPresent() const {
    glfwSwapBuffers(reinterpret_cast<GLFWwindow*>(m_windowHandle));
    glfwPollEvents();
}

#if _DEBUG

void APIENTRY glDebugOutput(
    GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    const char* message,
    const void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::string sourceStr;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             sourceStr = "API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sourceStr = "Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceStr = "Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     sourceStr = "Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     sourceStr = "Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           sourceStr = "Other"; break;
    }

    std::string typeStr;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               typeStr = "Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = "Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         typeStr = "Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = "Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              typeStr = "Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          typeStr = "Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           typeStr = "Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               typeStr = "Other"; break;
    }

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
    {
        LOG_ERROR("GL: [{0}] ({1}) {2}", sourceStr, typeStr, message);
        break;
    }
    case GL_DEBUG_SEVERITY_MEDIUM:
    {
        LOG_WARNING("GL: [{0}] ({1}) {2}", sourceStr, typeStr, message);
        break;
    }
    case GL_DEBUG_SEVERITY_LOW:
    {
        LOG_INFO("GL: [{0}] ({1}) {2}", sourceStr, typeStr, message);
        break;
    }
    case GL_DEBUG_SEVERITY_NOTIFICATION:
    {
        LOG_DEBUG("GL: [{0}] ({1}) {2}", sourceStr, typeStr, message);
        break;
    }
    }
}

#endif