#pragma once

#include "event.hpp"
#include "engine/input/input_enums.hpp"
#include <fmt/format.h>

namespace engine::events {

	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(const float x, const float y)
			: m_mouseX(x), m_mouseY(y) {}

		float getX() const { return m_mouseX; }
		float getY() const { return m_mouseY; }

		std::string toString() const override {
			return fmt::format("MouseMovedEvent: {}, {}", m_mouseX, m_mouseY);
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategory_Mouse | EventCategory_Input)
	private:
		float m_mouseX, m_mouseY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(const float xOffset, const float yOffset)
			: m_XOffset(xOffset), m_YOffset(yOffset) {}

		float getXOffset() const { return m_XOffset; }
		float getYOffset() const { return m_YOffset; }

		std::string toString() const override {
			return fmt::format("MouseScrolledEvent: {}, {}", getXOffset(), getYOffset());
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategory_Mouse | EventCategory_Input)
	private:
		float m_XOffset, m_YOffset;
	};

	class MouseButtonEvent : public Event
	{
	public:
		engine::input::MouseButton getMouseButton() const { return m_button; }

		EVENT_CLASS_CATEGORY(EventCategory_Mouse | EventCategory_Input | EventCategory_MouseButton)
	protected:
		MouseButtonEvent(const engine::input::MouseButton button)
			: m_button(button) {}

		engine::input::MouseButton m_button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(const engine::input::MouseButton button)
			: MouseButtonEvent(button) {}

		std::string toString() const override {
			return fmt::format("MouseButtonReleasedEvent: {}", m_button);
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(const engine::input::MouseButton button)
			: MouseButtonEvent(button) {}

		std::string toString() const override {
			return fmt::format("MouseButtonReleasedEvent: {}", m_button);
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};

}