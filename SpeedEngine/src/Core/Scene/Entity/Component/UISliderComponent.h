#pragma once

#include "Core/Features/Serializable.h"

namespace SE
{
	enum class UISliderOrientation { Horizontal, Vertical };

	struct UISliderComponent
	{
		float value{ 0.0f };
		float minimum{ 0.0f };
		float maximum{ 1.0f };

		float step{ 0.0f }; // 0 means continuous

		UISliderOrientation orientation{ UISliderOrientation::Horizontal };

		DEFINE_TYPE(
			UISliderComponent,
			member("value", &UISliderComponent::value),
			member("minimum", &UISliderComponent::minimum),
			member("maximum", &UISliderComponent::maximum),
			member("step", &UISliderComponent::step),
			member("orientation", &UISliderComponent::orientation)
		);
	};
}
