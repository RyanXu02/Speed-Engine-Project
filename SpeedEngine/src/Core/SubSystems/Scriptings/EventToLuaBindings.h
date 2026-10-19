#pragma once


#include <sol/sol.hpp>
#include "core/SubSystems/Events/Event.h"

namespace SE
{
	class EventToLuaBindings
	{
	public:
		static void exportEvents(sol::state& luaState);

	private:
		// automatically resolve cpp Event types to export to lua
		template<typename T> requires std::derived_from<T, Event>
		static void _registerEvent(sol::state& luaState)
		{
			// create a lua type, match cpp type
			auto luaType = luaState.new_usertype<T>(T::getStaticType(), sol::base_classes, sol::bases<Event>());

			auto resolver = [&luaType](const char* name, auto memberPtr) {
				luaType.set(name, memberPtr);
			};

			//static call to the visitFields function that each event registers itself to
			T::visitFields(resolver);

			//for lua to cast cpp event type to the appropriate type
			std::string castFunc = std::string("as_") + T::getStaticType();
			luaState.set_function(castFunc, [](Event& event) -> T* {
				if (event.getName() == T::getStaticType())
				{
					return static_cast<T*>(&event);
				}
				return nullptr;
			});
		}
	};
}
