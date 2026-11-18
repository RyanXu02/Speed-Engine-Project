#pragma once
#include "Core/SubSystems/SubSystem.h"
#include "KeyCodes.h"
#include "InputEventQueue.h"

namespace SE
{
	// @brief Window content coordinates in logical units, with (0,0) at the top left.
	struct MousePos
	{
		double x;
		double y;
	};

	struct MouseScroll
	{
		double xoffset;
		double yoffset;
	};

	class InputSystem : public SubSystem
	{
		friend class Engine;

	public:
		void init() override;
		void update(double deltaTime) override;
		void shutdown() override;
		bool hasFocus() const { return m_hasFocus; }

		// @brief Gets the region in the window where input events are accepted and mapped to a render target
		const InputRegion& getInputRegion() const { return m_inputRegion; }

		bool isAcceptingInput() const { return m_hasFocus && m_inputRegion.acceptsInput; }

		// @brief Check if the mouse pointer is inside the input region
		bool isPointerInInputRegion() const;
		// @brief Get the mouse pointer position in the input region, 
		std::optional<glm::vec2> getPointerPositionInInputRegion() const;

		// @brief Rising edge (key 0 -> 1)
		bool isKeyRising(KeyCodes key) const;
		// @brief Falling edge (key 1 -> 0)
		bool isKeyFalling(KeyCodes key) const;
		// @brief Key is currently down (key 1)
		bool isKeyDown(KeyCodes key) const;
		// @brief Key is currently up (key 0)
		bool isKeyUp(KeyCodes key) const;

		// @brief Rising edge (mouse button 0 -> 1)
		bool isMouseRising(MouseCodes key) const;
		// @brief Falling edge (mouse button 1 -> 0)
		bool isMouseFalling(MouseCodes key) const;
		// @brief Mouse button is currently down (button 1)
		bool isMouseDown(MouseCodes key) const;
		// @brief Mouse button is currently up (button 0)
		bool isMouseUp(MouseCodes key) const;

		// @brief Get mouse position in GLFW screen coordinates, with (0,0) at the top left of the window
		// @brief Use getPointerPositionInInputRegion() for render target coordinates
		const MousePos& getMousePosition() const;
		// @brief Get mouse position delta between last and this frame
		const MousePos& getMouseDelta() const;
		// @brief Get mouse scroll offset, automatically reset to 0 after each frame
		const MouseScroll& getMouseScroll() const;

	private:
		InputSystem(InputEventQueue& inputEvents)
			: SubSystem("InputSystem"), m_inputEvents(inputEvents)
		{}

		InputEventQueue& m_inputEvents;
		std::vector<InputEvent> m_pendingEvents;
		bool m_hasFocus{ true };

		InputRegion m_inputRegion;

		std::bitset<512> m_keyCurrState{ 0 };
		std::bitset<512> m_keyPrevState{ 0 };
		std::bitset<512> m_keyRising{ 0 };
		std::bitset<512> m_keyFalling{ 0 };

		std::bitset<8> m_mouseCurrState{ 0 };
		std::bitset<8> m_mousePrevState{ 0 };
		std::bitset<8> m_mouseRising{ 0 };
		std::bitset<8> m_mouseFalling{ 0 };

		MousePos m_mousePosition{ 0.0, 0.0 };
		MousePos m_mouseDelta{ 0.0, 0.0 }; // per simulation tick
		bool m_hasMousePosition = false;

		MouseScroll m_mouseScroll{ 0.0, 0.0 };
	};
} // namespace SE
