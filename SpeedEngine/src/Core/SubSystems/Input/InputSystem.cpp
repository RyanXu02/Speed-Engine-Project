#include "pch.h"

#include "InputSystem.h"

#include "Engine.h"
#include "Core/SubSystems/Events/EventSystem.h"

namespace SE
{
	void InputSystem::init()
	{
		SubSystem::init();
	}

	void InputSystem::update(double deltaTime)
	{
		SubSystem::update(deltaTime);

		m_keyPrevState = m_keyCurrState;
		m_mousePrevState = m_mouseCurrState;
		m_keyRising.reset();
		m_keyFalling.reset();
		m_mouseRising.reset();
		m_mouseFalling.reset();
		m_mouseDelta = { 0.0, 0.0 };
		m_mouseScroll = { 0.0, 0.0 };

		auto* ES = Engine::Instance().getSubSystem<EventSystem>();

		// drain input events from the queue and process them
		m_inputEvents.drain(m_pendingEvents);
		for (const auto& inputEvent : m_pendingEvents)
		{
			// update current state rising and falling edges for keys and mouse
			const auto update = [&inputEvent](auto& current, auto& rising, auto& falling)
			{
				const auto code = static_cast<size_t>(inputEvent.code);
				if (inputEvent.action == InputEvent::Action::Press && !current.test(code))
					rising.set(code);
				else if (inputEvent.action == InputEvent::Action::Release && current.test(code))
					falling.set(code);
				current.set(code, inputEvent.action != InputEvent::Action::Release);
			};

			switch (inputEvent.type)
			{
			case InputEvent::Type::Key:
				update(m_keyCurrState, m_keyRising, m_keyFalling);
				if (inputEvent.action != InputEvent::Action::Release)
					ES->publish(std::make_unique<KeyPressedEvent>(inputEvent.code, inputEvent.action == InputEvent::Action::Repeat));
				break;

			case InputEvent::Type::MouseButton:
				update(m_mouseCurrState, m_mouseRising, m_mouseFalling);
				break;

			case InputEvent::Type::CursorPosition:
				if (m_hasMousePosition)
				{
					m_mouseDelta.x += inputEvent.x - m_mousePosition.x;
					m_mouseDelta.y += inputEvent.y - m_mousePosition.y;
				}
				m_mousePosition = { inputEvent.x, inputEvent.y };
				m_hasMousePosition = true;
				ES->publish(std::make_unique<MouseMovedEvent>(inputEvent.x, inputEvent.y));
				break;

			case InputEvent::Type::Scroll:
				m_mouseScroll.xoffset += inputEvent.x;
				m_mouseScroll.yoffset += inputEvent.y;
				break;

			case InputEvent::Type::Focus:
				if (inputEvent.action == InputEvent::Action::Release)
				{
					m_keyFalling |= m_keyCurrState;
					m_mouseFalling |= m_mouseCurrState;
					m_keyCurrState.reset();
					m_mouseCurrState.reset();
					m_mouseDelta = { 0.0, 0.0 };
					m_hasMousePosition = false;
				}
				break;

			case InputEvent::Type::WindowClose:
				ES->publish(std::make_unique<WindowClose>());
				break;
			}
		}
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

} // namespace SE
