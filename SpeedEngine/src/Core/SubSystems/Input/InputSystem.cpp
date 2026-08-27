#include "pch.h"
#include "InputSystem.h"
#include "Engine.h"

#include "GLFW/glfw3.h"

#include "../../Logger/Logger.h"

namespace SE
{
	void InputSystem::init()
	{
		SubSystem::init();

		glfwSetScrollCallback(glfwGetCurrentContext(),
			[](GLFWwindow* window, double xoffset, double yoffset)
			{
				Engine::Instance().getSubSystem<InputSystem>()->scroll_callback(window, xoffset, yoffset);
			});
	}

	void InputSystem::update(double deltaTime)
	{
		SubSystem::update(deltaTime);

		m_mouseScroll = m_mouseScrollAccumulated;
		m_mouseScrollAccumulated = { 0.0, 0.0 };

		// update key states
		m_keyPrevState = m_keyCurrState;
		for (int key = KEY_SPACE; key <= KEY_MENU; key++)
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
			default:
				break;
			}
		}

		// update mouse states
		m_mousePrevState = m_mouseCurrState;
		for (int button = MOUSE_BUTTON_1; button <= MOUSE_BUTTON_LAST; button++)
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
			default:
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
		m_prevMousePosition = m_mousePosition;
		glfwGetCursorPos(glfwGetCurrentContext(), &m_mousePosition.x, &m_mousePosition.y);
		m_mouseDelta.x = m_mousePosition.x - m_prevMousePosition.x;
		m_mouseDelta.y = m_mousePosition.y - m_prevMousePosition.y;
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

	const MousePos& InputSystem::getMouseDelta() const
	{
		return m_mouseDelta;
	}

	const MouseScroll& InputSystem::getMouseScroll() const
	{
		return m_mouseScroll;
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

	void InputSystem::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
		m_mouseScrollAccumulated.xoffset += xoffset;
		m_mouseScrollAccumulated.yoffset += yoffset;
	}
}