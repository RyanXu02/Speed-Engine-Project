#pragma once

#include "Transform.h"
#include "Core/Features/Serializable.h"

namespace SE
{
	struct UIRectComponent
	{
		glm::vec2 anchor{ 0.5f, 0.5f }; // Point on canvas to attach to
		glm::vec2 pivot{ 0.5f, 0.5f }; // Point on the element attached to the anchor

		// in canvas space
		glm::vec2 offset{ 0.0f, 0.0f }; // Displacement from the anchor
		glm::vec2 size{ 100.0f, 100.0f };

		DEFINE_TYPE(
			UIRectComponent,
			member("anchor", &UIRectComponent::anchor),
			member("pivot", &UIRectComponent::pivot),
			member("offset", &UIRectComponent::offset),
			member("size", &UIRectComponent::size)
		);
	};
}
