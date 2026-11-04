#pragma once

#include "Core/SubSystems/SubSystem.h"

namespace SE
{
	class Scene;
	class InputSystem;

	enum class UICommand
	{
		NavigateUp, NavigateDown, NavigateLeft, NavigateRight,
		Next, Previous, Submit, Cancel
	};

	class UISystem : public SubSystem
	{
		friend class Engine;

	public:
		void update(double deltaTime) override;

		// @brief Activate a canvas for input processing. Returns true if the canvas was successfully activated
		bool activateCanvas(uint32_t canvasId);
		// @brief Set focus to a specific UI element. Returns true if the element was successfully focused
		bool setFocus(uint32_t elementId);

		
		uint32_t getActiveCanvas() const { return m_activeCanvas; }
		uint32_t getFocusedElement() const { return m_focusedElement; }

		// @brief Process a UI command (e.g. navigation, submit, cancel)
		void processCommand(UICommand command);
		
		// @brief Get if UISystem is accpeting commands
		bool isAcceptingCommands() const;

	private:
		UISystem() : SubSystem("UISystem") {}

		// @brief Refresh the focus state based on the current scene. Returns the current scene
		Scene* _refreshFocus();

		// @brief Change focus to a specific element in the scene
		void _changeFocus(Scene& scene, uint32_t elementId);
		
		// @brief process keyboard input for commands
		void _processKeyboard(const InputSystem& input);

		uint32_t m_activeCanvas{ 0 };
		uint32_t m_focusedElement{ 0 };
		uint64_t m_sceneRevision{ 0 };
	};
}
