#include "pch.h"

#include "EntityRegistry.h"

#include "Component/ComponentStructs.h"
#include "Entity.h"


// @brief Registers a component type with the EntityRegistry for serialzation, deserialization, and lifecycle hooks when the component is added or removed
// @brief Your component should use this macro to provide its necessary onAdd and onRemove behaviour if it needs to be specially handled when added or removed from an entity
// @param LoadHook The callback function that is called when the component is added to an entity
// @param UnloadHook The callback function that is called when the component is removed from an entity
#define REGISTER_COMPONENT(Type, LoadHook, UnloadHook)                       \
	do                                                                       \
	{                                                                        \
		REGISTER_TYPE(                                                       \
			Type,                                                            \
			[this](uint32_t entityID, const nlohmann::json& data)            \
			{                                                                \
				Entity(entityID, this).addComponent<Type>(data.get<Type>()); \
			});                                                              \
		m_componentHooks.try_emplace(typeid(Type), ComponentHooks{           \
			[this](uint32_t entityID)                                        \
			{                                                                \
				LoadHook(Entity(entityID, this));                            \
			},                                                               \
			[this](uint32_t entityID)                                        \
			{                                                                \
				UnloadHook(Entity(entityID, this));                          \
			}});                                                             \
	} while (false)

#define NO_DEFINE [](Entity entity) {}

namespace SE
{
	EntityRegistry::EntityRegistry(const LoggerProxy& logger)
		: Serializable(),
		  m_logger(logger)
	{
		REGISTER_COMPONENT(NameComponent, NO_DEFINE, NO_DEFINE);
		REGISTER_COMPONENT(Transform, NO_DEFINE, NO_DEFINE);
		REGISTER_COMPONENT(MeshComponent, Entity::_onMeshComponentAdded(), NO_DEFINE);
		REGISTER_COMPONENT(PrefabComponent, NO_DEFINE, NO_DEFINE);
		REGISTER_COMPONENT(ScriptComponent, Entity::_onScriptComponentAdded(), Entity::_onScriptComponentRemoved());
		REGISTER_COMPONENT(CameraComponent, Entity::_onCameraComponentAdded(), Entity::_onCameraComponentRemoved());
		REGISTER_COMPONENT(RigidBody, Entity::_onRigidBodyComponentAdded(), NO_DEFINE);
		REGISTER_COMPONENT(PhysicsShapeBox, NO_DEFINE, NO_DEFINE);
		REGISTER_COMPONENT(PhysicsShapeCapsule, NO_DEFINE, NO_DEFINE);
		REGISTER_COMPONENT(PhysicsShapeSphere, NO_DEFINE, NO_DEFINE);
		REGISTER_COMPONENT(PhysicsShapeMesh, Entity::_onPhysicsShapeMeshComponentAdded(), NO_DEFINE);
		REGISTER_COMPONENT(AudioSFXComponent, Entity::_onSFXComponentAdded(), Entity::_onSFXComponentRemoved());
		REGISTER_COMPONENT(AudioMusicComponent, Entity::_onMusicComponentAdded(), NO_DEFINE);
		REGISTER_COMPONENT(ParticleEmitter, Entity::_onParticleEmitterAdded(), NO_DEFINE);
		REGISTER_COMPONENT(LightComponent, NO_DEFINE, NO_DEFINE);
		REGISTER_COMPONENT(UICanvasComponent, NO_DEFINE, NO_DEFINE);
		REGISTER_COMPONENT(UIRectComponent, NO_DEFINE, NO_DEFINE);
		REGISTER_COMPONENT(UIElementComponent, NO_DEFINE, NO_DEFINE);
		REGISTER_COMPONENT(UIImageComponent, NO_DEFINE, NO_DEFINE);
		REGISTER_COMPONENT(UITextComponent, NO_DEFINE, NO_DEFINE);
		REGISTER_COMPONENT(UISelectableComponent, NO_DEFINE, NO_DEFINE);
		REGISTER_COMPONENT(UIButtonComponent, NO_DEFINE, NO_DEFINE);
		REGISTER_COMPONENT(UISliderComponent, NO_DEFINE, NO_DEFINE);

	}

	EntityRegistry::~EntityRegistry()
	{
		clear();
	}

	uint32_t EntityRegistry::addEntity()
	{
		while (m_entitySSID == 0 || m_serializedToRuntime.contains(m_entitySSID))
			++m_entitySSID;
		return _addEntity(m_entitySSID++);
	}

	uint32_t EntityRegistry::_addEntity(uint64_t entitySSID)
	{
		const uint32_t entityID = _assignID();
		m_registry.try_emplace(entityID, ComponentRegistry{});
		m_runtimeToSerialized.emplace(entityID, entitySSID);
		m_serializedToRuntime.emplace(entitySSID, entityID);

		return entityID;
	}

	uint64_t EntityRegistry::getEntitySSID(uint32_t entityID) const
	{
		auto it = m_runtimeToSerialized.find(entityID);
		return it != m_runtimeToSerialized.end() ? it->second : 0;
	}

	uint32_t EntityRegistry::getEntityID(uint64_t entitySSID) const
	{
		auto it = m_serializedToRuntime.find(entitySSID);
		return it != m_serializedToRuntime.end() ? it->second : 0;
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

		std::vector<const OnComponentRemoved*> removalHooks;
		for (const auto& [type, component] : it->second)
		{
			auto hookIt = m_componentHooks.find(type);
			if (hookIt != m_componentHooks.end())
			{
				removalHooks.push_back(&hookIt->second.onRemoved);
			}
		}
		for (const auto* hook : removalHooks)
		{
			(*hook)(entityID);
		}

		m_serializedToRuntime.erase(m_runtimeToSerialized.at(entityID));
		m_runtimeToSerialized.erase(entityID);
		m_registry.erase(entityID);

		return true;
	}

	void EntityRegistry::clear()
	{
		while (!m_registry.empty())
			removeEntity(m_registry.begin()->first);
		m_runtimeToSerialized.clear();
		m_serializedToRuntime.clear();
		m_entitySSID = 1;
	}

	bool EntityRegistry::empty()
	{
		return m_registry.empty();
	}

	void EntityRegistry::serialize(nlohmann::json& j) const
	{
		j = nlohmann::json::array();

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
						prefabPath = componentAsJson.at("prefabPath").get<std::string>();
					}
					else
					{
						componentsJson[type] = componentAsJson;
					}
				}
				catch (const nlohmann::json::exception& e)
				{
					m_logger.warn("failed to serialize {}: {}", type, e.what());
					throw;
				}
			}

			nlohmann::json entityObj;
			entityObj["id"] = getEntitySSID(entityID);
			if (isPrefabEntity)
			{
				entityObj["prefab_ref"] = prefabPath;
				if (!componentsJson.empty())
				{
					entityObj["overrides"]["components"] = componentsJson;
				}
				j.push_back(entityObj);
			}
			else
			{
				entityObj["components"] = componentsJson;
				j.push_back(entityObj);
			}
		}
	}

	void EntityRegistry::deserialize(const nlohmann::json& j)
	{
		assert(j.is_array() && !j.is_null() && !j.is_discarded() && "Registry: failed to deserialize json");

		std::vector<uint32_t> runtimeIDs;
		runtimeIDs.reserve(j.size());
		for (const auto& entity : j)
		{
			runtimeIDs.push_back(entity.contains("id") ? _addEntity(entity["id"].get<uint64_t>()) : 0);
		}
		for (auto& entityID : runtimeIDs)
		{
			if (entityID == 0)
				entityID = addEntity();
		}

		// Resolve ids before components can reference other entities
		for (size_t index = 0; index < j.size(); ++index)
		{
			const auto& entity = j[index];
			const uint32_t entityID = runtimeIDs[index];
			auto componentsJson = entity.value("components", nlohmann::json::object());

			if (entity.contains("prefab_ref"))
			{
				addComponent<PrefabComponent>(entityID, entity["prefab_ref"]);
			}

			for (auto& [type, json] : componentsJson.items())
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

					typeResult.invoke(entityID, json);
				}
				catch (const nlohmann::json::exception& e)
				{
					m_logger.warn("failed to deserialize {}: {}", type, e.what());

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
