#pragma once

namespace SE
{
	struct InputEvent
	{
		enum class Type { Key, MouseButton, CursorPosition, Scroll, Focus, WindowClose };
		enum class Action { Release, Press, Repeat };

		Type type;
		int code = 0;
		Action action = Action::Release;
		double x = 0.0;
		double y = 0.0;
	};
}
