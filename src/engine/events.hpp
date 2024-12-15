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
		Repeat = 2,
	};

	// Enum describing input keycodes
	enum class Keycode : uint8_t {
		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,

		Num0,
		Num1,
		Num2,
		Num3,
		Num4,
		Num5,
		Num6,
		Num7,
		Num8,
		Num9,

		Numpad0,
		Numpad1,
		Numpad2,
		Numpad3,
		Numpad4,
		Numpad5,
		Numpad6,
		Numpad7,
		Numpad8,
		Numpad9,
		
		Space,
		Enter,
		Backslash,
		Slash,
		Equals,
		Minus,
		Backspace,
		Tab,
		Tilde,
		Capslock,
		LeftShift,
		RightShift,
		LeftCtrl,
		RightCtrl,
		LeftAlt,
		RightAlt,
		Comma,
		Period,
		PageUp,
		PageDown,
		Insert,
		Delete,
		PrintScreen,
		Escape,
		SemiColon,
		Quote,
		LeftSquareBracket,
		RightSquareBracket,

		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		F13,
		F14,
		F15,
		F16,
		F17,
		F18,
		F19,
		F20,
		F21,
		F22,
		F23,
		F24,
		
		ArrowUp,
		ArrowDown,
		ArrowLeft,
		ArrowRight,

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
		char keyCode = '0';
	};
}