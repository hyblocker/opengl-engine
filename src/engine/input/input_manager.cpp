#include "input_manager.hpp"
#include "engine/log.hpp"

namespace engine::input {

	InputManager* InputManager::s_instance = nullptr;

	void InputManager::init() {

		if (s_instance != nullptr) {
			LOG_FATAL("Tried creating InputManager more than once! Breaking singleton. Aborting...");
			return;
		}
		s_instance = this;

		// Init to false
		for (size_t i = 0; i < (size_t)Keycode::Count; i++) {
			m_currentKeyboardState[i] = false;
			m_lastKeyboardState[i] = false;
		}
		for (size_t i = 0; i < (size_t)MouseButton::Count; i++) {
			m_currentMouseState[i] = false;
			m_lastMouseState[i] = false;
		}
		m_mousePos.x = 0;
		m_mousePos.y = 0;
		m_mouseScroll.x = 0;
		m_mouseScroll.y = 0;
	}

	void InputManager::event(events::Event& event) {
		events::EventDispatcher dispatcher(event);
		
		if (event.handled)
			return;

		dispatcher.dispatch<events::KeyPressedEvent>(EVENT_BIND_FUNC(InputManager::onKeyPressedEvent));
		dispatcher.dispatch<events::KeyReleasedEvent>(EVENT_BIND_FUNC(InputManager::onKeyReleasedEvent));
		dispatcher.dispatch<events::KeyTypedEvent>(EVENT_BIND_FUNC(InputManager::onKeyTypedEvent));

		dispatcher.dispatch<events::MouseButtonPressedEvent>(EVENT_BIND_FUNC(InputManager::onMousePressedEvent));
		dispatcher.dispatch<events::MouseButtonReleasedEvent>(EVENT_BIND_FUNC(InputManager::onMouseReleasedEvent));
		dispatcher.dispatch<events::MouseMovedEvent>(EVENT_BIND_FUNC(InputManager::onMouseMovedEvent));
		dispatcher.dispatch<events::MouseScrolledEvent>(EVENT_BIND_FUNC(InputManager::onMouseScrolledEvent));
	}

	void InputManager::update() {
		// Copy from current to last, and buffer to current, for both mouse and keyboard state
		// This keeps them in sync
		memcpy(m_lastKeyboardState, m_currentKeyboardState, sizeof(m_currentKeyboardState));
		memcpy(m_lastMouseState, m_currentMouseState, sizeof(m_currentMouseState));

		// Copy mouse state
		m_mousePos = m_bufferMousePos;
		m_mouseScroll = m_bufferMouseScroll;
	}

	bool InputManager::onKeyPressedEvent(const events::KeyPressedEvent& event) {
		m_currentKeyboardState[(size_t)event.getKeyCode()] = true;
		return false;
	}
	bool InputManager::onKeyReleasedEvent(const events::KeyReleasedEvent& event) {
		m_currentKeyboardState[(size_t)event.getKeyCode()] = false;
		m_lastKeyboardState[(size_t)event.getKeyCode()] = true;
		return false;
	}
	bool InputManager::onKeyTypedEvent(const events::KeyTypedEvent& event) {
		m_currentKeyboardState[(size_t)event.getKeyCode()] = true;
		return false;
	}

	bool InputManager::onMousePressedEvent(const events::MouseButtonPressedEvent& event) {
		m_currentMouseState[(size_t)event.getMouseButton()] = true;
		return false;
	}
	bool InputManager::onMouseReleasedEvent(const events::MouseButtonReleasedEvent& event) {
		m_currentMouseState[(size_t)event.getMouseButton()] = false;
		m_lastMouseState[(size_t)event.getMouseButton()] = true;
		return false;
	}
	bool InputManager::onMouseMovedEvent(const events::MouseMovedEvent& event) {
		m_bufferMousePos.x = event.getX();
		m_bufferMousePos.y = event.getY();
		return false;
	}
	bool InputManager::onMouseScrolledEvent(const events::MouseScrolledEvent& event) {
		m_bufferMouseScroll.x = event.getXOffset();
		m_bufferMouseScroll.y = event.getYOffset();
		return false;
	}

	[[nodiscard]] const bool InputManager::keyUp(Keycode key) const {
		return m_currentKeyboardState[(size_t)key] == false;
	}
	
	[[nodiscard]] const bool InputManager::keyDown(Keycode key) const {
		return m_currentKeyboardState[(size_t)key] == true;
	}

	[[nodiscard]] const bool InputManager::keyReleased(Keycode key) const {
		return m_lastKeyboardState[(size_t)key] == true && m_currentKeyboardState[(size_t)key] == false;
	}

	[[nodiscard]] const bool InputManager::mouseUp(MouseButton button) const {
		return m_currentMouseState[(size_t)button] == false;
	}
	[[nodiscard]] const bool InputManager::mouseDown(MouseButton button) const {
		return m_currentMouseState[(size_t)button] == true;
	}
	[[nodiscard]] const bool InputManager::mouseReleased(MouseButton button) const {
		return m_lastMouseState[(size_t)button] == true && m_currentMouseState[(size_t)button] == false;
	}

	[[nodiscard]] const bool InputManager::anyKeyReleased(char* pReleasedKey) const {
		for (size_t i = (size_t)Keycode::Space; i < (uint16_t)Keycode::Count; i++) {
			if (m_lastKeyboardState[(size_t)i] == true && m_currentKeyboardState[(size_t)i] == false) {
				if (pReleasedKey) {
					*pReleasedKey = (char)i;
				}
				return true;
			}
		}

		return false;
	}
}