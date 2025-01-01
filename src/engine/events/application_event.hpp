#pragma once

#include "event.hpp"
#include <fmt/format.h>

namespace engine::events {

	class WindowResizeEvent : public Event {
	public:
		WindowResizeEvent(uint32_t width, uint32_t height)
			: width(width), height(height) {}

		uint32_t width, height;

		std::string toString() const override {
			return fmt::format("WindowResizeEvent: {}, {}", width, height);
		}

		EVENT_CLASS_TYPE(WindowResize)
		EVENT_CLASS_CATEGORY(EventCategory_Application)
	private:
	};

	class WindowCloseEvent : public Event {
	public:
		WindowCloseEvent() = default;

		EVENT_CLASS_TYPE(WindowClose)
		EVENT_CLASS_CATEGORY(EventCategory_Application)
	};
}