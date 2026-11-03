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
		void update(double deltaTime) override;
		void shutdown() override;

		// @brief Set the current viewport size, in pixels. This is used to resolve UI element layouts
		// @param size The viewport size, in pixels
		void setViewportSize(glm::vec2 size) { m_viewportSize = size; }
		glm::vec2 getViewportSize() const { return m_viewportSize; }

		// @brief Get the resolved UI elements in back to front draw order
		const std::vector<ResolvedUIElement>& getResolvedElements() const { return m_resolvedElements; }

	private:
		UISystem() : SubSystem("UISystem") {}

		glm::vec2 m_viewportSize{ 0.0f };
		std::vector<ResolvedUIElement> m_resolvedElements;
	};
}
