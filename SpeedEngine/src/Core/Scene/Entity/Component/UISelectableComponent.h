#pragma once

#include "Transform.h"
#include "Core/Features/Serializable.h"

namespace SE
{
	struct UISelectableComponent
	{
		int navigationOrder{ 0 };
		bool focused{ false };
		bool hovered{ false };
		bool pressed{ false };

		// Optional keys in UIElementComponent's canvas
		// empty means unassigned
		std::string upElementKey;
		std::string downElementKey;
		std::string leftElementKey;
		std::string rightElementKey;

		glm::vec4 normalColor{ 0.8f, 0.8f, 1.0f, 1.0f };
		glm::vec4 hoverColor{ 0.4f, 0.4f, 1.0f, 1.0f };
		glm::vec4 focusedColor{ 0.8f, 0.9f, 1.0f, 1.0f };
		glm::vec4 pressedColor{ 0.7f, 0.7f, 0.7f, 1.0f };
		glm::vec4 disabledColor{ 0.5f, 0.5f, 0.5f, 1.0f };

		DEFINE_TYPE(
			UISelectableComponent,
			member("navigationOrder", &UISelectableComponent::navigationOrder),
			member("upElementKey", &UISelectableComponent::upElementKey),
			member("downElementKey", &UISelectableComponent::downElementKey),
			member("leftElementKey", &UISelectableComponent::leftElementKey),
			member("rightElementKey", &UISelectableComponent::rightElementKey),
			member("normalColor", &UISelectableComponent::normalColor),
			member("hoverColor", &UISelectableComponent::hoverColor),
			member("focusedColor", &UISelectableComponent::focusedColor),
			member("pressedColor", &UISelectableComponent::pressedColor),
			member("disabledColor", &UISelectableComponent::disabledColor)
		);
	};
}
