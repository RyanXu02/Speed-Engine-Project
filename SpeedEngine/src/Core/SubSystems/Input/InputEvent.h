#pragma once

#include <glm/vec2.hpp>

namespace SE
{
	// @brief Represents the region in a window where input events are accepted and mapped to a render target
	struct InputRegion
	{
		glm::vec2 imageTopLeft{ 0.0f }; // GLFW screen coordinates
		glm::vec2 imageSize{ 0.0f };
		glm::uvec2 renderTargetSize{ 0u };
		bool acceptsInput{ false };
	};

	// @brief Input event structure used to communicate input events from the windowing system to the input system
	struct InputEvent
	{
		enum class Type { Key, MouseButton, CursorPosition, Scroll, Focus, WindowClose, InputRegion };
		enum class Action { Release, Press, Repeat };

		Type type;
		int code = 0;
		Action action = Action::Release;
		double x = 0.0;
		double y = 0.0;
		InputRegion inputRegion;
	};
}
