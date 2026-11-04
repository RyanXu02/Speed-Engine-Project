#include "pch.h"

#include "Scene.h"

#include "Core/SubSystems/Events/EventSystem.h"
#include "Core/SubSystems/Physics/PhysicsSystem.h"
#include "Core/SubSystems/Scenes/SceneSystem.h"
#include "Engine.h"
#include <SubSystems/Scriptings/ScriptingSystem.h>
#include <SubSystems/Audio/AudioSystem.h>

namespace SE
{
	using MT = ActiveSceneModified::ModifyType;

	void Scene::initializeScene()
	{
		m_logger.info("Initializing Scene");

		//auto eventSystem = Engine::Instance().getSubSystem<EventSystem>();
		//auto event = eventSystem->subscribe(EventType::KeyPressed, [](Event& event) {

		//});


		m_isInitialized = true;
	}

	void Scene::updateScene(double deltaTime)
	{
		if (!m_entitiesToRemove.empty())
		{
			for (uint32_t id : m_entitiesToRemove)
			{
				m_registry.removeEntity(id);
			}
			m_entitiesToRemove.clear();
		}

		for (auto& [id, components] : m_registry)
		{
			Entity entity = getEntity(id);
			auto* lifeComponent = entity.getComponent<LifeComponent>();
			if (lifeComponent && !lifeComponent->isAlive)
			{
				removeEntity(entity.id());
			}

			//update all sfx components on entities
			auto* sfxComponent = entity.getComponent<AudioSFXComponent>();
			if (sfxComponent && sfxComponent->isGameSFX())
			{
				//do the position setting here....
				const glm::vec3 pos = entity.getComponent<Transform>()->getTranslation();
				sfxComponent->updateMyPosition(pos);
			}

			//update all particle emitter components on entities
			auto* PEComponent = entity.getComponent<ParticleEmitter>();
			if (PEComponent)
			{
				PEComponent->acquireTransform(entity.getComponent<Transform>());
				PEComponent->update(deltaTime);
				if (PEComponent->GetOneShot() && PEComponent->AmIDone()) { //oneshot is done, time to KILL MY PARENT >:) or stop emitter :)
					removeEntity(entity.id());
					//PEComponent->StopEmitter();
				}

			}
		}

	}

	void Scene::shutdownScene()
	{
		m_logger.info("Shutting down");
		m_registry.clear();
		m_entitiesToRemove.clear();

		m_isInitialized = false;
	}

	uint32_t Scene::addEntity(const std::string& name)
	{
		uint32_t entityID = m_registry.addEntity();
		Entity entity = getEntity(entityID);

		entity.addComponent<Transform>(); // default values that all entities have
		entity.addComponent<NameComponent>(name);
		entity.addComponent<LifeComponent>(true);
		NameComponent* component = entity.getComponent<NameComponent>();

		return entityID;
	}

	bool Scene::hasEntity(uint32_t id)
	{
		if (!m_registry.hasEntity(id))
		{
			// m_logger.warn("there is no entity with given name");
			return false;
		}

		return m_registry.hasEntity(id);
	}

	Entity Scene::getEntity(uint32_t id)
	{
		if (!hasEntity(id))
		{
			m_logger.warn("retrieving invalid entity");
			return Entity(0, &m_registry);
		}
		return Entity(id, &m_registry);
	}

	bool Scene::removeEntity(uint32_t id)
	{
		if (m_registry.hasEntity(id))
		{
			m_entitiesToRemove.push_back(id);
			return true;
		}

		return false;
	}

	bool Scene::readScene(std::string_view path, nlohmann::json& sceneJson) const
	{
		try
		{
			std::ifstream input{ std::filesystem::path(path) };
			if (!input)
			{
				m_logger.error("Could not open scene file '{}'", path);
				return false;
			}

			auto parsed = nlohmann::json::parse(input);
			if (input.bad())
			{
				m_logger.error("Failed reading scene file '{}'", path);
				return false;
			}
			if (!validateSceneJson(parsed))
			{
				m_logger.error("Invalid scene file '{}'", path);
				return false;
			}

			sceneJson = std::move(parsed);
			return true;
		}
		catch (const std::exception& e)
		{
			m_logger.error("Failed reading scene '{}': {}", path, e.what());
			return false;
		}
	}

	bool Scene::saveScene(std::string_view path)
	{
		try
		{
			const auto filePath = std::filesystem::absolute(std::filesystem::path(path)).lexically_normal();
			nlohmann::json sceneJson;
			serialize(sceneJson);
			const auto contents = sceneJson.dump(2);
			std::filesystem::create_directories(filePath.parent_path());

			std::ofstream out(filePath, std::ios::out | std::ios::trunc);
			if (!out)
			{
				m_logger.error("Could not open scene file '{}' for writing", filePath.generic_string());
				return false;
			}

			out << contents << '\n';
			out.close();
			if (!out)
			{
				m_logger.error("Failed writing scene file '{}'", filePath.generic_string());
				return false;
			}

			m_path = filePath.generic_string();
			m_logger.info("Scene saved to '{}': {} entities written", m_path, sceneJson["entities"].size());
			return true;
		}
		catch (const std::exception& e)
		{
			m_logger.error("Failed saving scene '{}': {}", path, e.what());
			return false;
		}
	}

	bool Scene::loadScene(std::string_view path)
	{
		std::error_code error;
		const auto filePath = std::filesystem::absolute(std::filesystem::path(path), error).lexically_normal().generic_string();
		if (error)
		{
			m_logger.error("Invalid scene path '{}': {}", path, error.message());
			return false;
		}

		nlohmann::json sceneJson;
		if (!readScene(filePath, sceneJson))
			return false;

		deserialize(sceneJson);
		m_path = filePath;
		m_logger.info("Scene loaded from '{}'", m_path);
		return true;
	}

	std::unordered_map<uint32_t, std::string> Scene::getEntityList()
	{
		std::unordered_map<uint32_t, std::string> entityList;
		for (auto& [id, components] : m_registry)
		{
			Entity entity = getEntity(id);
			LifeComponent* life = entity.getComponent<LifeComponent>();
			if (life && life->isAlive)
			{
				entityList[entity.id()] = entity.getComponent<NameComponent>()->name;
			}
		}
		return entityList;
	}

	void Scene::serialize(nlohmann::json& j) const
	{
		j = nlohmann::json::object();
		j["scene_name"] = m_name;

		m_registry.serialize(j["entities"]);
	}

	void Scene::deserialize(const nlohmann::json& j)
	{
		if (!validateSceneJson(j))
			return;

		Engine::Instance().getSubSystem<ScriptingSystem>()->reset();
		m_registry.clear();
		m_entitiesToRemove.clear();

		if (j.contains("scene_name"))
			m_name = j["scene_name"].get<std::string>();

		Engine::Instance().getSubSystem<AudioSystem>()->clearSoundLibraries();

		m_registry.deserialize(j["entities"]);

		if (!m_registry.empty())
			Engine::Instance().getSubSystem<PhysicsSystem>()->createAndAddComponents();

		Engine::Instance().getSubSystem<SceneSystem>()->sceneContentsReplaced(this);
	}

	bool Scene::validateSceneJson(const nlohmann::json& j) const
	{
		if (!j.is_object() || !j.contains("entities") || !j["entities"].is_array()
			|| (j.contains("scene_name") && !j["scene_name"].is_string()))
		{
			m_logger.error("Expected a scene object with an entities array and optional string scene_name");
			return false;
		}

		const auto validComponents = [](const nlohmann::json& components)
		{
			return components.is_object() && std::ranges::all_of(components,
				[](const nlohmann::json& component) { return component.is_object(); });
		};

		for (size_t index = 0; index < j["entities"].size(); ++index)
		{
			const auto& entity = j["entities"][index];
			if (!entity.is_object() || (!entity.contains("components") && !entity.contains("prefab_ref"))
				|| (entity.contains("components") && !validComponents(entity["components"]))
				|| (entity.contains("prefab_ref") && !entity["prefab_ref"].is_string()))
			{
				m_logger.error("Scene entity {} needs a components object or string prefab_ref; component values must be objects", index);
				return false;
			}

			if (entity.contains("overrides")
				&& (!entity.contains("prefab_ref") || !entity["overrides"].is_object()
					|| (entity["overrides"].contains("components") && !validComponents(entity["overrides"]["components"]))))
			{
				m_logger.error("Scene entity {} has invalid prefab overrides; expected an object with optional components object", index);
				return false;
			}
		}
		return true;
	}

} // namespace SE
