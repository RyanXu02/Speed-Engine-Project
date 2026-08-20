#pragma once
#include "../SubSystem.h"
#include "KeyCodes.h"

namespace SE {

	// @brief 0 at top left cornder, measured in screen space (sub)pixels
	struct MousePos
	{
		double x;
		double y;
	};

	class InputSystem : public SubSystem
	{
	public:
		InputSystem() : SubSystem("InputSystem") {}

		static InputSystem& Instance()
		{
			if (s_instance)
			{
				return *s_instance;
			}
			static InputSystem instance;
			s_instance = &instance;
			return *s_instance;
		}

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

		const MousePos& getMousePosition() const;

	private:
		void _setKeyState(KeyCodes key);
		void _clearKeyState(KeyCodes key);

		void _setMouseState(MouseCodes key);
		void _clearMouseState(MouseCodes key);

		static InputSystem* s_instance;

		std::bitset<512> m_keyCurrState{ 0 };
		std::bitset<512> m_keyPrevState{ 0 };
		std::bitset<512> m_keyRising{ 0 };
		std::bitset<512> m_keyFalling{ 0 };

		std::bitset<8> m_mouseCurrState{ 0 };
		std::bitset<8> m_mousePrevState{ 0 };
		std::bitset<8> m_mouseRising{ 0 };
		std::bitset<8> m_mouseFalling{ 0 };

		MousePos m_mousePosition{ 0.0, 0.0 };
	};
}

