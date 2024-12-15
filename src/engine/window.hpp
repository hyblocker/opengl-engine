#pragma once

#include <inttypes.h>
#include <string>
#include <memory>
#include <dexode/EventBus.hpp>

typedef uintptr_t WindowHandle;

struct WindowDesc {
	uint32_t width = 1820;
	uint32_t height = 720;
	std::string title = "OpenGL Engine";
};

struct GLFWwindow;

class Window {
public:
	Window(WindowDesc desc, int32_t openglMajor, int32_t openglMinor, std::shared_ptr<dexode::EventBus>& eventBus);
	~Window();

	void createNativeWindow();
	void close() const;
	void shutdown() const;

	void setTitle(std::string& title);

	inline const uint32_t getWidth() const { return m_desc.width; }
	inline const uint32_t getHeight() const { return m_desc.height; }
	inline const WindowHandle getHandle() const { return m_windowHandle; }

	inline const WindowDesc getDesc() const { return m_desc; }
	
	inline static const uint32_t getWindowCount() { return s_windowCount; }

private:
	int shouldCloseWindow() const;
	void windowPresent() const;

private:
	static void onResizeCallback(GLFWwindow* glfwWindow, int width, int height);
	static void onKeyCallback(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods);

private:
	WindowHandle m_windowHandle;
	WindowDesc m_desc;
	int32_t m_openglMajor;
	int32_t m_openglMinor;
	std::shared_ptr<dexode::EventBus> m_eventBus;

	static uint32_t s_windowCount;

	friend class App;
};