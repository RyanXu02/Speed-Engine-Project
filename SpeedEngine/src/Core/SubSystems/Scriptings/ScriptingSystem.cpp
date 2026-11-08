#include "pch.h"
#include "ScriptingSystem.h"
#include "EventToLuaBindings.h"
#include "ComponentToLuaBindings.h"
#include "MathToLuaBindings.h"
#include "SubSystems/Scenes/SceneSystem.h"
#include "Core/Engine.h"
#include "Core/Scene/Entity/Entity.h"
#include "Core/Features/CameraFrustum.h"
#include "Core/EngineStates.h"
#include "SubSystems/Input/InputSystem.h"

namespace SE
{
	void SE::ScriptingSystem::init()
	{
		SubSystem::init();

		m_luaState.open_libraries(sol::lib::base, sol::lib::package, sol::lib::math);

		MathToLuaBindings::exportMath(m_luaState);
		EventToLuaBindings::exportEvents(m_logger.get(), m_luaState);
		ComponentToLuaBindings::exportComponents(m_logger.get(), m_luaState);

		sol::table loggerTable = m_luaState.create_named_table("Logger");
		loggerTable["verbose"] = [&](const std::string& msg) { m_logger->verbose(msg); };
		loggerTable["debug"] = [&](const std::string& msg) { m_logger->debug(msg); };
		loggerTable["info"] = [&](const std::string& msg) { m_logger->info(msg); };
		loggerTable["warn"] = [&](const std::string& msg) { m_logger->warn(msg); };
		loggerTable["error"] = [&](const std::string& msg) { m_logger->error(msg); };
		loggerTable["critical"] = [&](const std::string& msg) { m_logger->critical(msg); };

		sceneSystem = Engine::Instance().getSubSystem<SceneSystem>();
		
		//TODO: break down export to multiple files or compile time will go up
		sol::table sceneSystemTable = m_luaState.create_named_table("SceneSystem");
		sceneSystemTable["requestNewScene"] = [&](const std::string& name) -> void
		{
			sceneSystem->requestNewScene(name);
		};
		sceneSystemTable["requestOpenScene"] = [&](const std::string& path) -> void
		{
			sceneSystem->requestOpenScene(path);
		};
		sceneSystemTable["requestReloadScene"] = [&]() -> void
		{
			sceneSystem->requestReloadScene();
		};
		sceneSystemTable["newScene"] = sceneSystemTable["requestNewScene"];
		sceneSystemTable["setCurrentScene"] = sceneSystemTable["requestOpenScene"];
		sceneSystemTable["getCurrentScene"] = [&]() -> Scene*
		{
			return sceneSystem->getCurrentScene();
		};

		m_luaState.new_usertype<Scene>("Scene",
			"getName", &Scene::getName,
			"addEntity", &Scene::addEntity,
			"hasEntity", &Scene::hasEntity,
			"getEntity", &Scene::getEntity,
			"removeEntity", &Scene::removeEntity,
			"isEntityPendingRemoval", &Scene::isEntityPendingRemoval
		);

		sol::table inputTable = m_luaState.create_named_table("Input");
		inputTable["isKeyDown"] = [](KeyCodes key) {
			return Engine::Instance().getSubSystem<InputSystem>()->isKeyDown(key);
		};
		inputTable["isMouseDown"] = [](MouseCodes button) {
			return Engine::Instance().getSubSystem<InputSystem>()->isMouseDown(button);
		};
		inputTable["getMouseDelta"] = []() {
			const MousePos& delta = Engine::Instance().getSubSystem<InputSystem>()->getMouseDelta();
			return std::make_tuple(delta.x, delta.y);
		};

		sol::table engineTable = m_luaState.create_named_table("Engine");
		engineTable["isPlaying"] = []() {
			return EngineStates::isPlaying();
		};


		m_luaState.new_usertype<CameraFrustum>("CameraFrustum",
			"move", &CameraFrustum::move,
			"getPosition", &CameraFrustum::getPosition,
			"setPosition", &CameraFrustum::setPosition,
			"getTarget", &CameraFrustum::getTarget,
			"setTarget", &CameraFrustum::setTarget,
			"getFOV", &CameraFrustum::getFOV,
			"setFOV", &CameraFrustum::setFOV,
			"getNearPlane", &CameraFrustum::getNearPlane,
			"setNearPlane", &CameraFrustum::setNearPlane,
			"getFarPlane", &CameraFrustum::getFarPlane,
			"setFarPlane", &CameraFrustum::setFarPlane
		);
	}

	void SE::ScriptingSystem::update(double deltaTime)
	{
		if (EngineStates::isPaused())
		{
			return;
		}

		if (!m_scriptsToReload.empty())
		{
			for (const auto& path : m_scriptsToReload)
			{
				reloadScript(path);
			}
			m_scriptsToReload.clear();
		}

		Scene* scene = sceneSystem->getCurrentScene();
		if (!scene)
			return;

		auto func = [&](Entity entity) -> void {
			if (scene->isEntityPendingRemoval(entity.id()))
				return;
			ScriptComponent* scriptComponent = entity.getComponent<ScriptComponent>();
			_initScript(*scriptComponent);

			if (scriptComponent->onUpdate)
			{
				scriptComponent->onUpdate(deltaTime);
			}
		};

		scene->forEntitiesWith<ScriptComponent>(func);

	}

	void SE::ScriptingSystem::shutdown()
	{
		SubSystem::shutdown();

		// release every lua instance held by script components while EventSystem is still alive
		Scene* scene = sceneSystem ? sceneSystem->getCurrentScene() : nullptr;
		if (scene)
		{
			auto func = [&](Entity entity) -> void {
				ScriptComponent* scriptComponent = entity.getComponent<ScriptComponent>();
				if (scriptComponent->onDestroy)
				{
					scriptComponent->onDestroy();
				}
				scriptComponent->onInit = []() {};
				scriptComponent->onUpdate = [](double) {};
				scriptComponent->onDestroy = []() {};
				scriptComponent->initilized = false;
			};
			scene->forEntitiesWith<ScriptComponent>(func);
		}

		EngineStates::setCamera(nullptr);
		m_scripts.clear();
		m_scriptCache.clear();
		m_scriptInstances.clear();

		m_luaState["Entity"] = sol::nil;
		m_luaState.collect_garbage();
		//keep this, apparently some lua projects had to clean twice to fully clear memory... what?
		//m_luaState.collect_garbage();
	}

	//load the script and function call details into the script component
	//TODO: VERY prone to unexpeceted crash, handled most edge cases but some might be missed, add acordingly
	void ScriptingSystem::loadScript(Entity& entity, std::string_view path) {

		std::string filePath(path);
		if (filePath.empty() || filePath == "None")
		{
			return;
		}

		try
		{
			if (!entity.getComponent<ScriptComponent>())
			{
				m_logger->error("ScriptComponent not found in entity {}", entity.id());
				return;
			}

			// use file name as the class name definition so the script defined classname and it's file name
			// must match char by char, only allow one class per script
			sol::table scriptClass;
			auto scriptComponent = entity.getComponent<ScriptComponent>();

			auto it = m_scriptCache.find(filePath);
			if (it != m_scriptCache.end())
			{
				scriptClass = it->second;
			}
			else
			{
				// validate file path
				if (!std::filesystem::exists(filePath))
				{
					m_logger->error("Script loading failed: File does not exist at path '{}'", filePath);
					return;
				}

				// validate if there's any compile error within the loaded script
				auto scriptResult = m_luaState.safe_script_file(filePath);
				if (!scriptResult.valid())
				{
					sol::error err = scriptResult;
					m_logger->error("Failed to compile/execute Lua script file {}: {}", filePath, err.what());
					return;
				}

				//use file name as class name access, hence it must match what defined in the lua file
				std::string className = std::filesystem::path(filePath).stem().string();
				auto proxy = m_luaState[className];
				
				// if the class is defined in the file exists as promised
				if (!proxy.valid())
				{
					m_logger->error("Failed to find Lua class table: {}", className);
					m_logger->error("Make sure file name matches defined class name");
					return;
				}

				// even if it exists still need to check if it's a legit table with no random bs
				sol::table loadedClass = proxy;
				if (!loadedClass.valid())
				{
					m_logger->error("Failed to validate Lua class table: {}", className);
					return;
				}

				m_scriptCache[filePath] = loadedClass;
				scriptClass = loadedClass;
			}

			//cache only the script code but construction of instance still needs to happen per script component load
			//every implemented class script must have new, onInit, onUpdate, onDestroy declared
			sol::protected_function constructor = scriptClass["new"];
			if (!constructor.valid())
			{
				std::string className = std::filesystem::path(filePath).stem().string();
				m_logger->error("Class {} does not have a 'new' constructor function!", className);
				return;
			}

			// since entity is just a handle id, pass a copy of the entity to the script
			// this is why we want no data inside entity for easy copy
			sol::protected_function_result result = constructor(Entity(entity));
			if (!result.valid())
			{
				sol::error err = result;
				std::string className = std::filesystem::path(filePath).stem().string();
				m_logger->error("Failed to construct instance for Lua class {}: {}", className, err.what());
				return;
			}

			// this is the instance we just created with the entity copied to it
			sol::table luaInstance = result;
			m_scriptInstances[entity.id()] = luaInstance;

			sol::protected_function initFunc = luaInstance["onInit"];
			if (initFunc.valid())
			{
				if (scriptComponent->initilized) {
					scriptComponent->initilized = false;
				}
				scriptComponent->onInit = [luaInstance, initFunc, this]() mutable{
					auto result = initFunc(luaInstance);
					if (!result.valid()) {
						sol::error err = result;
						m_logger->error("Lua script init error: {}", err.what());
					}
				};
			}
			else // fail check if script doesnt have onInit, store an empty lambda
			{
				scriptComponent->onInit = []() {};
			}

			sol::protected_function updateFunc = luaInstance["onUpdate"];
			if (updateFunc.valid())
			{
				scriptComponent->onUpdate = [luaInstance, updateFunc, this](double dt) mutable
				{
					auto result = updateFunc(luaInstance, dt);
					if (!result.valid()) {
						sol::error err = result;
						m_logger->error("Lua script update error: {}", err.what());
					}
				};
			}
			else // fail check if script doesnt have onUpdate, store an empty lambda 
			{
				scriptComponent->onUpdate = [](double) {};
			}

			sol::protected_function destroyFunc = luaInstance["onDestroy"];
			if (destroyFunc.valid())
			{
				if (scriptComponent->onDestroy != nullptr) {
					scriptComponent->onDestroy(); // clean up first before reload new script
				}
				scriptComponent->onDestroy = [luaInstance, destroyFunc]() mutable {
					destroyFunc(luaInstance);
				};
			}
			else // fail check if script doesnt have onDestroy, store an empty lambda 
			{
				scriptComponent->onDestroy = []() {};
			}
		}
		catch (const std::runtime_error& e)
		{
			m_logger->error("{}", e.what());
		}
	}

	//load single time script, fire and forget, no entity is owning this script
	void ScriptingSystem::loadScript(std::string_view path) {
		sol::load_result fx = m_luaState.load_file(std::string(path));
		if (!fx.valid())
		{
			sol::error err = fx;
			m_logger->error("failed to load script into the program: {}", err.what());
		}
		else
		{
			m_scripts[std::string(path)] = fx.get<sol::protected_function>();
		}
	}

	void ScriptingSystem::runScript(std::string_view path) {
		if (path == "None")
		{
			return;
		}

		auto it = m_scripts.find(std::string(path));
		if (it == m_scripts.end())
		{
			m_logger->error("Cannot find script at: {}", std::string(path));
			return;
		}

		auto result = it->second();

		if (!result.valid())
		{
			sol::error err = result;
			m_logger->error("Failed to run script at: {}, error: {}", std::string(path), err.what());
		}
	}

	void ScriptingSystem::removeScript(std::string_view path) {
	
	}

	void ScriptingSystem::reloadScript(std::string_view path) {
		std::string filePath = path.data();

		auto itGlobal = m_scripts.find(path.data());
		if (itGlobal != m_scripts.end())
		{
			m_scripts.erase(itGlobal);
			loadScript(filePath);
			m_logger->info("Global script reloaded: {}", filePath);
			return;
		}

		//clear the cache or it wont load the new file with the same name
		m_scriptCache.erase(filePath);

		Scene* scene = sceneSystem->getCurrentScene();

		auto func = [&](Entity entity) -> void {
			auto scriptComp = entity.getComponent<ScriptComponent>();

			if (scriptComp->path == filePath)
			{
				m_logger->info("Hot-reloading live entity instance utilizing script: {}", filePath);

				loadScript(entity, filePath);
			}
		};

		scene->forEntitiesWith<ScriptComponent>(func);

		m_luaState.collect_garbage();
	}

	// the lua instance's new() call returned for this entity's script, return an invalid table when nothing is loaded
	sol::table ScriptingSystem::getScriptInstance(const Entity& entity) const
	{
		auto it = m_scriptInstances.find(entity.id());
		return it != m_scriptInstances.end() ? it->second : sol::table();
	}

	void ScriptingSystem::_destroyAllScripts()
	{
		m_scriptInstances.clear();	// in case entity ids get reused by different scenes

		Scene* scene = sceneSystem ? sceneSystem->getCurrentScene() : nullptr;
		if (!scene)
		{
			return;
		}

		scene->forEntitiesWith<ScriptComponent>([](Entity entity) {
			ScriptComponent* scriptComponent = entity.getComponent<ScriptComponent>();
			if (scriptComponent->onDestroy)
			{
				scriptComponent->onDestroy();
			}
			scriptComponent->onInit = []() {};
			scriptComponent->onUpdate = [](double) {};
			scriptComponent->onDestroy = []() {};
			scriptComponent->initilized = false;
		});
	}

	void ScriptingSystem::reset() 
	{
		_destroyAllScripts();
		EngineStates::setCamera(nullptr);
		m_scripts.clear();
		m_scriptCache.clear();
		m_luaState.collect_garbage();
		m_luaState.collect_garbage();
	}

	void ScriptingSystem::_initScript(ScriptComponent& scriptComponent) {
		if (!scriptComponent.initilized)
		{
			scriptComponent.onInit();
			scriptComponent.initilized = true;
		}
	}

}
