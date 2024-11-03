#pragma once

#include <inttypes.h>
#include <string>

namespace engine::events {

	enum class MouseButton : uint8_t {
		Left,
		Right,
		Scroll,
		Button1,
		Button2,
	};

	enum class InputState : uint8_t {
		Released,
		Down,
		Held,
	};

	struct EventWindowClose {
		bool terminateImmediately = false;
	};

	struct EventWindowResize {
		uint32_t newWidth = 0;
		uint32_t newHeight = 0;
	};

	struct EventMouseMoved {
		uint32_t newMousePosX = 0;
		uint32_t newMousePosY = 0;
	};

	struct EventMouseStateChanged {
		MouseButton mouseButton = MouseButton::Left;
		InputState mouseButtonState = InputState::Released;
	};

	struct EventKeyboard {
		InputState keyboardState = InputState::Released;
	};
}