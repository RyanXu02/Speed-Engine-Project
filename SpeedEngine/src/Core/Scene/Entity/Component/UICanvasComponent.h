#pragma once

#include "Transform.h"
#include "Core/Features/Serializable.h"

namespace SE
{
	struct UICanvasComponent
	{
		std::string canvasKey; // Unique key for this canvas
		// Each element's key is unique in its canvas

		glm::vec2 referenceResolution{ 1920.0f, 1080.0f };

		bool visible{ true };
		bool inputEnabled{ true };

		std::string defaultFocusElementKey; // empty means no default

		DEFINE_TYPE(
			UICanvasComponent,
			member("canvasKey", &UICanvasComponent::canvasKey),
			member("referenceResolution", &UICanvasComponent::referenceResolution),
			member("visible", &UICanvasComponent::visible),
			member("inputEnabled", &UICanvasComponent::inputEnabled),
			member("defaultFocusElementKey", &UICanvasComponent::defaultFocusElementKey)
		);
	};
}
