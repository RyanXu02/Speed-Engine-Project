#pragma once

#include "Core/Features/Serializable.h"


namespace SE
{
	struct UIElementComponent
	{
		std::string canvasKey; // Unique key for this canvas

		std::string elementKey; // Unique key for this element within its canvas

		bool visible{ true };
		bool enabled{ true };

		int drawOrder{ 0 };

		DEFINE_TYPE(
			UIElementComponent,
			member("canvasKey", &UIElementComponent::canvasKey),
			member("elementKey", &UIElementComponent::elementKey),
			member("visible", &UIElementComponent::visible),
			member("enabled", &UIElementComponent::enabled),
			member("drawOrder", &UIElementComponent::drawOrder)
		);
	};
}
