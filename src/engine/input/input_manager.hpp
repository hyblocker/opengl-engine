#pragma once

#include <inttypes.h>
#include <fmt/format.h>
#include <string>
#include <hlsl++.h>

#include "input_enums.hpp"
#include "engine/events/event.hpp"
#include "engine/events/keyboard_event.hpp"
#include "engine/events/mouse_event.hpp"

namespace engine {
	class App;
}

namespace engine::input {
	
	class InputManager {
		friend class engine::App;
	public:

		InputManager() = default;
		~InputManager() = default;

		// Keyboard state
		// if the key is currently not pressed this frame
		[[nodiscard]] const bool keyUp(Keycode key) const;
		// if the key is currently pressed this frame
		[[nodiscard]] const bool keyDown(Keycode key) const;
		// if the key was released this frame (ie pressed last frame, released this frame)
		[[nodiscard]] const bool keyReleased(Keycode key) const;

		// Mouse button state
		// if the mouse button is currently not pressed this frame
		[[nodiscard]] const bool mouseUp(MouseButton button) const;
		// if the mouse button is currently pressed this frame
		[[nodiscard]] const bool mouseDown(MouseButton button) const;
		// if the mouse button was released this frame (ie pressed last frame, released this frame)
		[[nodiscard]] const bool mouseReleased(MouseButton button) const;

		// Other mouse data
		[[nodiscard]] inline const hlslpp::float2 mousePos() const { return m_mousePos; }
		// Y = normal scroll, X = side scroll on supported hardware
		[[nodiscard]] inline const hlslpp::float2 mouseScroll() const { return m_mouseScroll; }

		[[nodiscard]] static inline InputManager* getInstance() { return s_instance; }

	private:

		void init();
		void event(events::Event& event);
		void update();

		// event handlers, these mutate internal state
		bool onKeyPressedEvent(const events::KeyPressedEvent& event);
		bool onKeyReleasedEvent(const events::KeyReleasedEvent& event);
		bool onKeyTypedEvent(const events::KeyTypedEvent& event);

		bool onMousePressedEvent(const events::MouseButtonPressedEvent& event);
		bool onMouseReleasedEvent(const events::MouseButtonReleasedEvent& event);
		bool onMouseMovedEvent(const events::MouseMovedEvent& event);
		bool onMouseScrolledEvent(const events::MouseScrolledEvent& event);

		bool m_currentKeyboardState[(size_t)Keycode::Count] = {};
		bool m_lastKeyboardState[(size_t)Keycode::Count] = {};

		bool m_currentMouseState[(size_t)MouseButton::Count] = {};
		bool m_lastMouseState[(size_t)MouseButton::Count] = {};

	private:
		hlslpp::float2 m_mousePos = {};
		hlslpp::float2 m_mouseScroll = {};

		hlslpp::float2 m_bufferMousePos = {};
		hlslpp::float2 m_bufferMouseScroll = {};

		static InputManager* s_instance;
	};
}