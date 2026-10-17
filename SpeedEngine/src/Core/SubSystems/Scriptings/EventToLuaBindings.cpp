#include "pch.h"
#include "EventToLuaBindings.h"

namespace SE
{
	//TODO: only add necessary function to lua since
	// it will generate more unecessary types binding and will bloat the binary
	void SE::EventToLuaBindings::exportEvents(sol::state& luaState) 
	{
		// export enum types to lua
		luaState.new_enum("EventType",
			"None", EventType::None,
			"WindowClose", EventType::WindowClose,
			"WindowResize", EventType::WindowResize,
			"WindowFocus", EventType::WindowFocus,
			"WindowLostFocus", EventType::WindowLostFocus,
			"WindowMoved", EventType::WindowMoved,
			"KeyPressed", EventType::KeyPressed,
			"KeyReleased", EventType::KeyReleased,
			"KeyTyped", EventType::KeyTyped,
			"MouseButtonPressed", EventType::MouseButtonPressed,
			"MouseButtonReleased", EventType::MouseButtonReleased,
			"MouseMoved", EventType::MouseMoved,
			"MouseScrolled", EventType::MouseScrolled,
			"SceneChanged", EventType::SceneChanged,
			"ActiveSceneModified", EventType::ActiveSceneModified
			// "EntitySelected", EventType::EntitySelected 
			// "ResourceChanged", EventType::ResourceChanged
		);

		// export Event type to lua
		luaState.new_usertype<Event>("Event",
			"getEventType", &Event::getEventType,
			"getName", &Event::getName
		);

	
		_registerEvent<MouseMove>(luaState);
	}
}
