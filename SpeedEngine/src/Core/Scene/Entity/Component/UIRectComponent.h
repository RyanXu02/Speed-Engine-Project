#pragma once

#include "Transform.h"
#include "Core/Features/Serializable.h"

namespace SE
{
	enum class UIAnchor 
	{ 
		TopLeft, TopMiddle, TopRight, 
		CenterLeft, Center, CenterRight,
		BottomLeft, BottomMiddle, BottomRight, 
	};

	struct UIRectComponent
	{
		glm::vec2 anchor{ 0.5f, 0.5f }; // Point on canvas to attach to
		glm::vec2 pivot{ 0.5f, 0.5f }; // Point on the element attached to the anchor

		// in canvas space
		glm::vec2 offset{ 0.0f, 0.0f }; // Displacement from the anchor
		glm::vec2 size{ 100.0f, 100.0f };

		// @brief Set the anchor and pivot to a preset value
		// @brief This sets the anchor and pivot to the same value
		void setAnchorPreset(UIAnchor where)
		{
			switch (where)
			{
			case UIAnchor::TopLeft: anchor = { 0.0f, 0.0f }; break;
			case UIAnchor::TopMiddle: anchor = { 0.5f, 0.0f }; break;
			case UIAnchor::TopRight: anchor = { 1.0f, 0.0f }; break;
			case UIAnchor::CenterLeft: anchor = { 0.0f, 0.5f }; break;
			case UIAnchor::Center: anchor = { 0.5f, 0.5f }; break;
			case UIAnchor::CenterRight: anchor = { 1.0f, 0.5f }; break;
			case UIAnchor::BottomLeft: anchor = { 0.0f, 1.0f }; break;
			case UIAnchor::BottomMiddle: anchor = { 0.5f, 1.0f }; break;
			case UIAnchor::BottomRight: anchor = { 1.0f, 1.0f }; break;
			}
			pivot = anchor;
		}

		DEFINE_TYPE(
			UIRectComponent,
			member("anchor", &UIRectComponent::anchor),
			member("pivot", &UIRectComponent::pivot),
			member("offset", &UIRectComponent::offset),
			member("size", &UIRectComponent::size)
		);
	};
}
