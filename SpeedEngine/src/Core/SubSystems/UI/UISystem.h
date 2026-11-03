#pragma once

#include "SubSystems/SubSystem.h"
#include "UILayout.h"

namespace SE
{
	struct ResolvedUIElement
	{
		uint32_t entityID;
		uint32_t canvasEntityID;
		int drawOrder;
		float scale;
		UIRectLayout rect;
	};

	class UISystem : public SubSystem
	{
		friend class Engine;

	public:
		// @brief Read the latest input snapshot and resolve UI layouts on the simulation thread
		void update(double deltaTime) override;
		void shutdown() override;

		// @brief Set the scene input region for this frame on the main thread
		// @param windowTopLeft The image's top-left corner in window content logical units
		// @param windowSize The displayed image size in window content logical units
		// @param pointerEnabled Whether the region allows scene pointer interaction
		void setSceneInputRegion(glm::vec2 windowTopLeft, glm::vec2 windowSize, bool pointerEnabled)
		{
			m_sceneInputRegion = { windowTopLeft, windowSize, pointerEnabled };
		}

		// @brief Capture and publish the latest UI input on the main thread after rendering
		// @param renderTargetSize The actual scene render-target dimensions, in pixels
		void captureInput(glm::uvec2 renderTargetSize);

		// @brief Get the latest scene render-target dimensions in pixels
		glm::vec2 getRenderTargetSize() const { return m_input.renderTargetSize; }
		
		// @brief Get the pointer position in render-target pixels, or no value when unavailable
		const std::optional<glm::vec2>& getPointerRenderTargetPosition() const { return m_input.pointerPosition; }
		// @brief Get the pointer position in the element's canvas space, or no value when unavailable
		// @param element The resolved element whose canvas scale is used for conversion
		std::optional<glm::vec2> getPointerCanvasPosition(const ResolvedUIElement& element) const
		{
			if (!m_input.pointerPosition)
				return std::nullopt;
			return *m_input.pointerPosition / element.scale;
		}

		// @brief Get the resolved UI elements in back to front draw order
		const std::vector<ResolvedUIElement>& getResolvedElements() const { return m_resolvedElements; }

	private:
		UISystem() : SubSystem("UISystem") {}

		struct SceneInputRegion
		{
			glm::vec2 windowTopLeft{ 0.0f };
			glm::vec2 windowSize{ 0.0f };
			bool pointerEnabled{ false };
		};
		struct InputSnapshot
		{
			glm::vec2 renderTargetSize{ 0.0f };
			std::optional<glm::vec2> pointerPosition;
		};

		SceneInputRegion m_sceneInputRegion; // Main thread only
		std::mutex m_inputMutex;
		InputSnapshot m_pendingInput;
		InputSnapshot m_input; // Simulation thread only
		std::vector<ResolvedUIElement> m_resolvedElements;
	};
}
