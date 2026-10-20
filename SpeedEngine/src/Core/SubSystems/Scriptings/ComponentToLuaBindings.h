#pragma once

#include <sol/sol.hpp>
#include <Scene/Entity/Entity.h>

namespace SE
{
	class Logger;
	class ComponentToLuaBindings
	{
	public:
		static void exportComponents(Logger* logger, sol::state& luaState);

	private:
		using ComponentGetter = std::function<sol::object(sol::state_view, Entity)>;
		using ComponentAdder = std::function<void(Entity, sol::table)>;
		using ComponentRemover = std::function<void(Entity)>;

		static inline std::unordered_map<std::string, ComponentGetter> componentGetters{};
		static inline std::unordered_map<std::string, ComponentAdder> componentAdders{};
		static inline std::unordered_map<std::string, ComponentRemover> componentRemovers{};

		// automatically resolve cpp Component types to export to lua
		template<typename T>
		static void _registerComponent(sol::state& luaState)
		{
			// create a lua type, match cpp type
			auto luaType = luaState.new_usertype<T>(T::getStaticType());

			auto resolver = [&luaType](const char* name, auto memberPtr)
			{
				luaType.set(name, memberPtr);
			};

			//static call to the visitFields function that each event registers itself to
			T::visitFields(resolver);

			// lua reference to the component pointer
			componentGetters[T::getStaticType()] = [](sol::state_view lua, Entity e) -> sol::object {
				T* component = e.getComponent<T>();
				if (component)
				{
					return sol::make_object(lua, component);
				}
				return sol::object(lua, sol::nil);
			};

			// safe add component function to the lua table
			componentAdders[T::getStaticType()] = [](Entity e, sol::table luaTable)
			{
				T* comp = e.addComponent<T>(); //only default constructor support for now and less error prone
			};

			// safe remove component function to the lua table
			componentRemovers[T::getStaticType()] = [](Entity e)
			{
				e.removeComponent<T>();
			};
		}
	};
}
