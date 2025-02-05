#include "window.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdio.h>

#include "core.hpp"

#include "gpu/idevice.hpp"
#include "engine/app.hpp"
#include "engine/log.hpp"
#include "engine/events/application_event.hpp"
#include "engine/events/keyboard_event.hpp"
#include "engine/events/mouse_event.hpp"

// for some reason this is defined in the internal header in GLFW, instead of exposing internal data we shall just redefine the constant here.
#define GL_CONTEXT_FLAGS 0x821e

namespace engine {

    // Initialise to 0 windows
    uint32_t Window::s_windowCount = 0;

    Window::Window(WindowDesc desc, int32_t openglMajor, int32_t openglMinor)
        : m_desc(desc),
        m_windowHandle(0),
        m_openglMajor(openglMajor),
        m_openglMinor(openglMinor)
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
#ifdef __APPLE__
            // macOS supposedly needs this
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
            // Request 4x MSAA
            glfwWindowHint(GLFW_SAMPLES, 4);
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
            // Enable vsync
            glfwSwapInterval(1);
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

        // assign callbacks to get input events
        glfwSetWindowSizeCallback(reinterpret_cast<GLFWwindow*>(m_windowHandle), [](GLFWwindow* glfwWindow, int width, int height) {
            Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
            window->m_desc.width = width;
            window->m_desc.height = height;

            engine::events::WindowResizeEvent eventData{
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height),
            };

            ::engine::App::getInstance()->event(eventData);
        });

        glfwSetKeyCallback(reinterpret_cast<GLFWwindow*>(m_windowHandle), [](GLFWwindow* glfwWindow, int key, int scancode, int action, int mods) {
            Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));

            switch (action)
            {
            case GLFW_PRESS:
            {
                engine::events::KeyPressedEvent eventData(static_cast<engine::input::Keycode>(key), false);
                ::engine::App::getInstance()->event(eventData);
                break;
            }
            case GLFW_RELEASE:
            {
                engine::events::KeyReleasedEvent eventData(static_cast<engine::input::Keycode>(key));
                ::engine::App::getInstance()->event(eventData);
                break;
            }
            case GLFW_REPEAT:
            {
                engine::events::KeyPressedEvent eventData(static_cast<engine::input::Keycode>(key), true);
                ::engine::App::getInstance()->event(eventData);
                break;
            }
            }
        });

        glfwSetCharCallback(reinterpret_cast<GLFWwindow*>(m_windowHandle), [](GLFWwindow* glfwWindow, unsigned int keycode) {
            Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));

            engine::events::KeyTypedEvent eventData(static_cast<engine::input::Keycode>(keycode));
            ::engine::App::getInstance()->event(eventData);
        });

        glfwSetMouseButtonCallback(reinterpret_cast<GLFWwindow*>(m_windowHandle), [](GLFWwindow* glfwWindow, int button, int action, int mods) {
            Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));

            switch (action)
            {
            case GLFW_PRESS:
            {
                engine::events::MouseButtonPressedEvent eventData(static_cast<engine::input::MouseButton>(button));
                ::engine::App::getInstance()->event(eventData);
                break;
            }
            case GLFW_RELEASE:
            {
                engine::events::MouseButtonReleasedEvent eventData(static_cast<engine::input::MouseButton>(button));
                ::engine::App::getInstance()->event(eventData);
                break;
            }
            }
        });

        glfwSetScrollCallback(reinterpret_cast<GLFWwindow*>(m_windowHandle), [](GLFWwindow* glfwWindow, double xOffset, double yOffset) {
            Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));

            engine::events::MouseScrolledEvent eventData((float)xOffset, (float)yOffset);
            ::engine::App::getInstance()->event(eventData);
        });

        glfwSetCursorPosCallback(reinterpret_cast<GLFWwindow*>(m_windowHandle), [](GLFWwindow* glfwWindow, double xPos, double yPos) {
            Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));

            engine::events::MouseMovedEvent eventData((float)xPos, (float)yPos);
            ::engine::App::getInstance()->event(eventData);
        });
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
}