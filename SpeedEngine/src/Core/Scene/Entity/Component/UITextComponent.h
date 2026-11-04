#pragma once

#include "Transform.h"
#include "Core/Features/Serializable.h"

namespace SE
{
	enum class UIHorizontalAlignment { Left, Center, Right };
	enum class UIVerticalAlignment { Top, Middle, Bottom };

	struct UITextComponent
	{
		std::string text;
		std::string fontPath;

		uint32_t fontPixelHeight{ 48 }; // font rasterization config only

		float fontSize{ 24.0f };
		glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

		UIHorizontalAlignment horizontalAlignment{ UIHorizontalAlignment::Left };
		UIVerticalAlignment verticalAlignment{ UIVerticalAlignment::Top };

		DEFINE_TYPE(
			UITextComponent,
			member("text", &UITextComponent::text),
			member("fontPath", &UITextComponent::fontPath),
			member("fontPixelHeight", &UITextComponent::fontPixelHeight),
			member("fontSize", &UITextComponent::fontSize),
			member("color", &UITextComponent::color),
			member("horizontalAlignment", &UITextComponent::horizontalAlignment),
			member("verticalAlignment", &UITextComponent::verticalAlignment)
		);
	};
}
