#pragma once

#include "event.hpp"
#include "engine/input/input_enums.hpp"
#include <fmt/format.h>

namespace engine::events {
	class KeyEvent : public Event
	{
	public:
		engine::input::Keycode getKeyCode() const { return m_keycode; }

		EVENT_CLASS_CATEGORY(EventCategory_Keyboard | EventCategory_Input)
	protected:
		KeyEvent(const engine::input::Keycode keycode)
			: m_keycode(keycode) {}

		engine::input::Keycode m_keycode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(const engine::input::Keycode keycode, bool isRepeat = false)
			: KeyEvent(keycode), m_is_repeat(isRepeat) {}

		bool isRepeat() const { return m_is_repeat; }

		std::string toString() const override {
			return fmt::format("KeyPressedEvent: {} (repeat = {})", m_keycode, m_is_repeat);
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		bool m_is_repeat;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(const engine::input::Keycode keycode)
			: KeyEvent(keycode) {}

		std::string toString() const override {
			return fmt::format("KeyReleasedEvent: {}", m_keycode);
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(const engine::input::Keycode keycode)
			: KeyEvent(keycode) {}

		std::string toString() const override {
			return fmt::format("KeyTypedEvent: {}", m_keycode);
		}

		EVENT_CLASS_TYPE(KeyTyped)
	};
}