#pragma once

#include "Core/Scene/Entity/Component/UIRectComponent.h"

namespace SE
{
	struct UICanvasLayout
	{
		float scale{ 1.0f }; // Viewport pixels per canvas pixel
		glm::vec2 size{ 0.0f }; // Canvas space
	};

	struct UIRectLayout
	{
		// Canvas space
		glm::vec2 topLeft{ 0.0f };
		glm::vec2 size{ 0.0f };
		// viewport space
		glm::vec2 pixelTopLeft{ 0.0f };
		glm::vec2 pixelSize{ 0.0f };
	};

	// Dimensions must be positive. Coordinates start at the top left, with positive Y downward.

	// @brief Resolve the layout of a UICanvasComponent
	// @param viewportSize The size of the viewport, in pixels
	// @param referenceResolution The reference resolution of the canvas, in pixels
	// @return The resolved layout of the canvas in canvas space and the scale factor to convert to viewport space
	inline UICanvasLayout resolveUICanvasLayout(glm::vec2 viewportSize, glm::vec2 referenceResolution)
	{
		const float scale = std::min(viewportSize.x / referenceResolution.x, viewportSize.y / referenceResolution.y);
		return { scale, viewportSize / scale };
	}

	// @brief Resolve the layout of a UIRectComponent
	// @param rect The UIRectComponent to resolve
	// @param canvas The resolved layout of the canvas the rect is attached to
	// @return The resolved layout of the rect, in both canvas and viewport space
	inline UIRectLayout resolveUIRectLayout(const UIRectComponent& rect, const UICanvasLayout& canvas)
	{
		const glm::vec2 topLeft = rect.anchor * canvas.size + rect.offset - rect.pivot * rect.size;
		return { topLeft, rect.size, topLeft * canvas.scale, rect.size * canvas.scale };
	}
}
