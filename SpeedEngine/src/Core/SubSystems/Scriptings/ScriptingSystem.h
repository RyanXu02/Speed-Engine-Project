#pragma once

#include <sol/sol.hpp>
#include "Scene/Entity/Entity.h"
#include "Core/SubSystems/SubSystem.h"

namespace SE
{
	class SceneSystem;
	class ScriptingSystem : public SubSystem
	{
	public:
		ScriptingSystem(const std::string& name = "ScriptingSystem")
		    : SubSystem(name)
		{}
		~ScriptingSystem() override = default;

		virtual void init() override;
		virtual void update(double deltaTime) override;
		virtual void shutdown() override;

		virtual void loadScript(Entity& entity, std::string_view path);
		virtual void loadScript(std::string_view path);
		virtual void runScript(std::string_view path);
		virtual void removeScript(Entity& entity);
		virtual void reloadScript(std::string_view path);
		virtual void reset();

		sol::table getScriptInstance(const Entity& entity) const;

	private:
		virtual void _initScript(ScriptComponent& scriptComponent);
		virtual void _destroyAllScripts();

		std::vector<std::string> m_scriptsToReload;

		sol::state m_luaState;
		std::unordered_map<std::string, sol::protected_function> m_scripts;
		std::unordered_map<std::string, sol::table> m_scriptCache;
		std::unordered_map<uint32_t, sol::table> m_scriptInstances;
		SceneSystem* sceneSystem;
	};

}