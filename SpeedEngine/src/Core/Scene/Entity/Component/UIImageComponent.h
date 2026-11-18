#pragma once

#include "Core/Features/MathToJSONBindings.h"
#include "Core/Features/Serializable.h"

namespace SE
{
	struct UIImageComponent
	{
		glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

		std::string texturePath; // empty means no texture

		DEFINE_TYPE(
			UIImageComponent,
			member("color", &UIImageComponent::color),
			member("texturePath", &UIImageComponent::texturePath)
		);
	};
}
