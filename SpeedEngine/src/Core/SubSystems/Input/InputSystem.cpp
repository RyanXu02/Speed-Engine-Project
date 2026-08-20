#include "pch.h"
#include "InputSystem.h"

#include "GLFW/glfw3.h"

#include "../../Logger/Logger.h"

namespace SE
{
	InputSystem* InputSystem::s_instance = nullptr;

	void InputSystem::init()
	{
		SubSystem::init();
		s_instance = this;
	}

	void InputSystem::update(double deltaTime)
	{
		SubSystem::update(deltaTime);

		// update key states
		m_keyPrevState = m_keyCurrState;
		for (int key = 0; key != KEY_MENU; key++)
		{
			KeyCodes thisKey = static_cast<KeyCodes>(key);
			switch (glfwGetKey(glfwGetCurrentContext(), key))
			{
			case GLFW_PRESS:
				_setKeyState(thisKey);
				break;
			case GLFW_RELEASE:
				_clearKeyState(thisKey);
				break;
			}
		}

		// update mouse states
		m_mousePrevState = m_mouseCurrState;
		for (int button = 0; button != MOUSE_BUTTON_LAST; button++)
		{
			MouseCodes thisButton = static_cast<MouseCodes>(button);
			switch (glfwGetMouseButton(glfwGetCurrentContext(), button))
			{
			case GLFW_PRESS:
				_setMouseState(thisButton);
				break;
			case GLFW_RELEASE:
				_clearMouseState(thisButton);
				break;
			}
		}

		// update key rises and falls
		std::bitset<512> keyChanges = m_keyCurrState ^ m_keyPrevState;
		m_keyRising = keyChanges & m_keyCurrState;
		m_keyFalling = keyChanges & (~m_keyCurrState);
		// update mouse rises and falls
		std::bitset<8> mouseChanges = m_mouseCurrState ^ m_mousePrevState;
		m_mouseRising = mouseChanges & m_mouseCurrState;
		m_mouseFalling = mouseChanges & (~m_mouseCurrState);

		// update mouse pos
		glfwGetCursorPos(glfwGetCurrentContext(), &m_mousePosition.x, &m_mousePosition.y);
	}

	void InputSystem::shutdown()
	{
		SubSystem::shutdown();
	}

	bool InputSystem::isKeyRising(KeyCodes key) const
	{
		return m_keyRising.test(static_cast<size_t>(key));
	}

	bool InputSystem::isKeyFalling(KeyCodes key) const
	{
		return m_keyFalling.test(static_cast<size_t>(key));
	}

	bool InputSystem::isKeyDown(KeyCodes key) const
	{
		return m_keyCurrState.test(static_cast<size_t>(key));
	}

	bool InputSystem::isKeyUp(KeyCodes key) const
	{
		return !m_keyCurrState.test(static_cast<size_t>(key));
	}

	bool InputSystem::isMouseRising(MouseCodes button) const
	{
		return m_mouseRising.test(static_cast<size_t>(button));
	}

	bool InputSystem::isMouseFalling(MouseCodes button) const
	{
		return m_mouseFalling.test(static_cast<size_t>(button));
	}

	bool InputSystem::isMouseDown(MouseCodes button) const
	{
		return m_mouseCurrState.test(static_cast<size_t>(button));
	}

	bool InputSystem::isMouseUp(MouseCodes button) const
	{
		return !m_mouseCurrState.test(static_cast<size_t>(button));
	}

	const MousePos& InputSystem::getMousePosition() const
	{
		return m_mousePosition;
	}

	void InputSystem::_setKeyState(KeyCodes key)
	{
		m_keyCurrState.set(static_cast<size_t>(key));
	}

	void InputSystem::_clearKeyState(KeyCodes key)
	{
		m_keyCurrState.reset(static_cast<size_t>(key));
	}

	void InputSystem::_setMouseState(MouseCodes key)
	{
		m_mouseCurrState.set(static_cast<size_t>(key));
	}

	void InputSystem::_clearMouseState(MouseCodes key)
	{
		m_mouseCurrState.reset(static_cast<size_t>(key));
	}
}