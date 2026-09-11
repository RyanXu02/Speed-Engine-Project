#pragma once
#include "../SubSystem.h"
#include "KeyCodes.h"

struct GLFWwindow;

namespace SE {
	// @brief 0 at top left corner, measured in screen space (sub)pixels
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

		// @brief Get mouse position in screen space (0,0 top left corner)
		const MousePos& getMousePosition() const;
		// @brief Get mouse position delta between last and this frame
		const MousePos& getMouseDelta() const;
		// @brief Get mouse scroll offset, automatically reset to 0 after each frame
		const MouseScroll& getMouseScroll() const;

	private:
		InputSystem() : SubSystem("InputSystem") {}

		void _setKeyState(KeyCodes key);
		void _clearKeyState(KeyCodes key);

		void _setMouseState(MouseCodes key);
		void _clearMouseState(MouseCodes key);

		void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

		std::bitset<512> m_keyCurrState{ 0 };
		std::bitset<512> m_keyPrevState{ 0 };
		std::bitset<512> m_keyRising{ 0 };
		std::bitset<512> m_keyFalling{ 0 };

		std::bitset<8> m_mouseCurrState{ 0 };
		std::bitset<8> m_mousePrevState{ 0 };
		std::bitset<8> m_mouseRising{ 0 };
		std::bitset<8> m_mouseFalling{ 0 };

		MousePos m_mousePosition{ 0.0, 0.0 };
		MousePos m_prevMousePosition{ 0.0, 0.0 };
		MousePos m_mouseDelta{ 0.0, 0.0 }; // per-frame

		MouseScroll m_mouseScroll{ 0.0, 0.0 };
		MouseScroll m_mouseScrollAccumulated{ 0.0, 0.0 };
	};
}

