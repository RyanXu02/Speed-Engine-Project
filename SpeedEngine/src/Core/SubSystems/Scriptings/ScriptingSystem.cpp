#include "pch.h"
#include "ScriptingSystem.h"
#include "EventToLuaBindings.h"

namespace SE
{
	void SE::ScriptingSystem::init()
	{
		SubSystem::init();

		m_luaState.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math);

		EventToLuaBindings::exportEvents(m_luaState);
	}

	void SE::ScriptingSystem::update(double deltaTime)
	{
		m_logger->info("script updating");

	}

	void SE::ScriptingSystem::shutdown()
	{
		SubSystem::shutdown();
	}

	void ScriptingSystem::loadScript(Entity& entity, std::string_view path) {
	
	}

	void ScriptingSystem::loadScript(std::string_view path) {
	
	}

	void ScriptingSystem::reloadScript(std::string_view path) {
	
	}

	void ScriptingSystem::runScript(std::string_view path) {
	
	}

	void ScriptingSystem::_reloadScript(const std::string& path) {
	
	}

}
