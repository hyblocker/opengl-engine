#pragma once

#include <inttypes.h>
#include <string>

namespace engine::events {

	enum class EventType {
		None = 0,
		WindowClose, WindowResize,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonReleased, MouseButtonPressed, MouseMoved, MouseScrolled,
	};

	// bitwise flags, an event may be in more than one category at the same time
	enum EventCategory {
		None = 0,
		EventCategory_Application	= 1 << 0,
		EventCategory_Input			= 1 << 1,
		EventCategory_Keyboard		= 1 << 2,
		EventCategory_Mouse			= 1 << 3,
		EventCategory_MouseButton	= 1 << 4,
	};

#define EVENT_CLASS_TYPE(type) \
	static EventType getStaticType() { return EventType::type; } \
	virtual EventType getEventType() const override { return getStaticType(); } \
	virtual const char* getName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) \
	virtual int getCategoryFlags() const override { return category; }

#define EVENT_BIND_FUNC(func) [this](auto&&... args) -> decltype(auto) { return this->func(std::forward<decltype(args)>(args)...); }

	class Event {
	public:
		virtual ~Event() = default;

		virtual EventType getEventType() const = 0;
		virtual const char* getName() const = 0;
		virtual int getCategoryFlags() const = 0;
		virtual std::string toString() const { return getName(); }

		bool isInCategory(EventCategory category) {
			return getCategoryFlags() & category;
		}

		// allow us to disable propagation of events to other layers
		bool handled = false;
	};
	
	class EventDispatcher {
	public:
		EventDispatcher(Event& event)
			: m_event(event)
		{}

		// F is a function returning bool, where bool sets the handled property of event to true if set to true
		template<typename T, typename F>
		typename std::enable_if<
			std::is_base_of<::engine::events::Event, T>::value && // T : engine::events::Event
			std::is_invocable_r<bool, F, const T&>::value, bool // bool F(const T& event)
		>::type
		dispatch(const F& func) {
			if (m_event.getEventType() == T::getStaticType()) {
				m_event.handled |= func(static_cast<T&>(m_event));
				return true;
			}
			return false;
		}

	private:
		Event& m_event;
	};
}