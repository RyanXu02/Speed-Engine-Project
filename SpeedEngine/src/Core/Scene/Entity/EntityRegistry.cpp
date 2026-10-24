#include "pch.h"

#include "EntityRegistry.h"

#include "Component/ComponentStructs.h"
#include "Core/Engine.h"
#include "Core/Scene/Scene.h"
#include "Entity.h"
#include "SubSystems/Scenes/SceneSystem.h"


//loadHook determines what needs to be done after a component is loaded
//unloadHook determines what needs to be done after a component is unloaded
#define REGISTER_COMPONENT(Type, LoadHook, UnloadHook)        \
	REGISTER_TYPE(                                            \
	    Type,                                                 \
	    [this](uint32_t entityID, const nlohmann::json& data) \
	    {                                                     \
		    addComponent<Type>(entityID, data.get<Type>());   \
		    LoadHook(Entity(entityID, this));                 \
	    },                                                    \
	    [this](uint32_t entityID, const nlohmann::json& data) \
	    {                                                     \
		    /* TODO: add unload unloadfunction if needed */   \
		    UnloadHook(Entity(entityID, this));               \
	    })

#define NO_DEFINE [](Entity entity) {}

namespace SE
{
	EntityRegistry::EntityRegistry()
	    : Serializable()
	{
		REGISTER_COMPONENT(NameComponent, Entity::_onNameComponentAdded(), Entity::_onNameComponentRemoved());
		REGISTER_COMPONENT(LifeComponent, NO_DEFINE, NO_DEFINE);
		REGISTER_COMPONENT(Transform, NO_DEFINE, NO_DEFINE);
		REGISTER_COMPONENT(MeshComponent, Entity::_onMeshComponentAdded(), Entity::_onMeshComponentRemoved());
		REGISTER_COMPONENT(PrefabComponent, NO_DEFINE, NO_DEFINE);
		REGISTER_COMPONENT(ScriptComponent, Entity::_onScriptComponentAdded(), Entity::_onScriptComponentRemoved());
		REGISTER_COMPONENT(RigidBody, NO_DEFINE, NO_DEFINE);
		REGISTER_COMPONENT(PhysicsShapeBox, NO_DEFINE, NO_DEFINE);
		REGISTER_COMPONENT(PhysicsShapeCapsule, NO_DEFINE, NO_DEFINE);
		REGISTER_COMPONENT(PhysicsShapeSphere, NO_DEFINE, NO_DEFINE);
		//REGISTER_COMPONENT(PhysicsShapeMesh, NO_DEFINE, NO_DEFINE);
		REGISTER_COMPONENT(AudioSFXComponent, NO_DEFINE, NO_DEFINE);
		REGISTER_COMPONENT(AudioMusicComponent, NO_DEFINE, NO_DEFINE);

	}

	EntityRegistry::~EntityRegistry() {}

	uint32_t EntityRegistry::addEntity()
	{
		m_registry.try_emplace(m_entityID, ComponentRegistry());

		return _assignID();
	}

	const EntityRegistry::ComponentRegistry* EntityRegistry::getComponentRegistry(uint32_t entityID) const
	{
		auto it = m_registry.find(entityID);
		if (it == m_registry.end())
		{
			return nullptr;
		}
		return &it->second;
	}

	bool EntityRegistry::hasEntity(uint32_t entityID)
	{
		return m_registry.find(entityID) != m_registry.end();
	}

	bool EntityRegistry::removeEntity(uint32_t entityID)
	{
		auto it = m_registry.find(entityID);
		if (it == m_registry.end())
		{
			return false;
		}

		m_registry.erase(entityID);

		return true;
	}

	void EntityRegistry::clear()
	{
		m_registry.clear();
	}

	bool EntityRegistry::empty()
	{
		return m_registry.empty();
	}

	void EntityRegistry::serialize(nlohmann::json& j) const
	{
		Scene* scene = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene();
		auto logger = scene->getLogger();

		j["entities"] = nlohmann::json::array();

		for (const auto& [entityID, components] : m_registry)
		{
			bool isPrefabEntity = false;
			std::string prefabPath = "";
			nlohmann::json componentsJson = nlohmann::json::object();

			for (const auto& [typeID, component] : components)
			{
				std::string type = demagleTypeInfo(typeID);

				auto typeResult = m_typeResolver.Get(type);
				if (!typeResult.isValid())
				{
					continue;
				}

				// ideally avoid catching errors but this does not happen every frame
				// nlohmann throws errors on fail parse which crash the app if left unhandled
				try
				{
					nlohmann::json componentAsJson = typeResult.serialize(component);

					if (type == "PrefabComponent")
					{
						isPrefabEntity = true;
						prefabPath = componentAsJson["PrefabComponent"]["prefab_path"];
					}
					else
					{
						componentsJson[type] = componentAsJson;
					}
				}
				catch (const nlohmann::json::exception& e)
				{
					logger.warn("failed to serialize {}: {}", type, e.what());
					continue;
				}
			}

			nlohmann::json entityObj;
			if (isPrefabEntity)
			{
				entityObj["prefab_ref"] = prefabPath;
				if (!componentsJson.empty())
				{
					entityObj["overrides"]["components"] = componentsJson;
				}
				j["entities"].push_back(entityObj);
			}
			else if (!componentsJson.empty())
			{
				entityObj["components"] = componentsJson;
				j["entities"].push_back(entityObj);
			}
		}
	}

	void EntityRegistry::deserialize(const nlohmann::json& j)
	{
		assert(j.is_array() && !j.is_null() && !j.is_discarded() && "Registry: failed to deserialize json");

		Scene* scene = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene();
		auto logger = scene->getLogger();

		for (const auto& entity : j)
		{
			uint32_t entityID = addEntity();
			if (!entity.contains("components") || !entity["components"].is_object())
			{
				continue;
			}

			if (entity.contains("prefab_ref"))
			{
				addComponent<PrefabComponent>(entityID, entity["prefab_ref"]);
			}

			for (auto& [type, json] : entity["components"].items())
			{
				auto typeResult = m_typeResolver.Get(type);
				if (!typeResult.isValid())
				{
					continue;
				}

				// ideally avoid catching errors but this does not happen every frame
				// nlohmann throws errors on fail parse which crash the app if left unhandled
				try
				{

					typeResult.invoke<ConstructorFunc>(entityID, json);
				}
				catch (const nlohmann::json::exception& e)
				{
					logger.warn("failed to deserialize {}: {}", type, e.what());

					continue;
				}
			}
		}
	}

	const EntityRegistry::Registry::const_iterator EntityRegistry::begin() const
	{
		return m_registry.begin();
	}

	const EntityRegistry::Registry::const_iterator EntityRegistry::end() const
	{
		return m_registry.end();
	}


	uint32_t EntityRegistry::_assignID()
	{
		return m_entityID.fetch_add(1, std::memory_order_relaxed);
	}

}
