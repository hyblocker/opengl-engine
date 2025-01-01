#pragma once

#include <inttypes.h>
#include <string>

typedef uintptr_t WindowHandle;

struct WindowDesc {
	uint32_t width = 1820;
	uint32_t height = 720;
	std::string title = "OpenGL Engine";
};

struct GLFWwindow;

namespace engine {

	class Window {
	public:
		Window(WindowDesc desc, int32_t openglMajor, int32_t openglMinor);
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
		WindowHandle m_windowHandle;
		WindowDesc m_desc;
		int32_t m_openglMajor;
		int32_t m_openglMinor;

		static uint32_t s_windowCount;

		friend class App;
	};
}