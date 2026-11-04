#pragma once

#include "Core/SubSystems/SubSystem.h"

namespace SE
{
	class Scene;
	class InputSystem;
	class ButtonActivated;

	enum class UICommand
	{
		NavigateUp, NavigateDown, NavigateLeft, NavigateRight,
		Next, Previous, SubmitPressed, SubmitReleased, Cancel
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
		
		// @brief A focused element is an element targetted by commands
		// @brief e.g. arrowkeys, joystick, submit, etc.
		uint32_t getFocusedElement() const { return m_focusedElement; }
		// @brief A captured element is an element that is being pressed
		// @brief A captured element can lost its pressed state while the pointer is dragged off it
		uint32_t getCapturedElement() const { return m_capturedElement; }

		// @brief Process a UI command (e.g. navigation, submit, cancel)
		void processCommand(UICommand command);
		
		// @brief Get if UISystem is accpeting commands
		bool isAcceptingCommands() const;

	private:
		UISystem() : SubSystem("UISystem") {}

		enum class CaptureSource { Pointer, Command };

		// @brief Refresh the focus state based on the current scene. Returns the current scene
		Scene* _refreshFocus();

		// @brief Change focus to a specific element in the scene
		void _changeFocus(Scene& scene, uint32_t elementId);

		// @brief Begin pressing a specific element in the scene
		void _beginPress(Scene& scene, uint32_t elementId, CaptureSource source);
		// @brief Release pressing a specific element in the scene
		void _releasePress(Scene& scene, uint32_t elementId, CaptureSource source, std::vector<ButtonActivated>& activations);
		
		// @brief Cancel the current capture
		void _cancelCapture(Scene& scene);

		void _processCommand(UICommand command, std::vector<ButtonActivated>& activations);
		
		// @brief process inputs for commands
		void _processKeyboard(const InputSystem& input, std::vector<ButtonActivated>& activations);
		void _processPointer(Scene& scene, const InputSystem& input, std::vector<ButtonActivated>& activations);

		uint32_t m_activeCanvas{ 0 };
		uint32_t m_focusedElement{ 0 };
		uint32_t m_capturedElement{ 0 };

		// Who started the current press?
		CaptureSource m_captureSource{ CaptureSource::Pointer };
		bool m_submitDown{ false };

		uint64_t m_sceneRevision{ 0 };
	};
}
