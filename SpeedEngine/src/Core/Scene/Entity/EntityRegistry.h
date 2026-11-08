#pragma once

#include "Core/Features/Serializable.h"

namespace SE
{
	class Entity;

	class EntityRegistry : public Serializable
	{
	public:
		//Note: ideally want to use sparse set and dense array for performance but until that becomes an issue, do not touch
		// each entity id is associated with a map of components
		typedef std::unordered_map<std::type_index, std::any> ComponentRegistry;
		typedef std::unordered_map<uint32_t, ComponentRegistry> Registry;

		explicit EntityRegistry(const LoggerProxy& logger);
		~EntityRegistry();

		// @brief Adds a new entity to the registry and returns its runtime ID
		// @brief PREFER TO USE Scene::addEntity() INSTEAD OF THIS ONE
		uint32_t addEntity();

		// @brief Get a Scene Serialization ID for a given runtime entity ID
		// @param entityID The runtime entity ID
		// @returns The Scene Serialization ID for the entity, or 0 if the entity does not exist
		uint64_t getEntitySSID(uint32_t entityID) const;
		// @brief Get a runtime entity ID for a given Scene Serialization ID
		// @param entitySSID The Scene Serialization ID
		// @returns The runtime entity ID for the entity, or 0 if the entity does not exist
		uint32_t getEntityID(uint64_t entitySSID) const;

		// @brief Get the component registry for a given entity ID
		// @param entityID The runtime entity ID
		// @returns A pointer to the component registry, or nullptr if the entity does not exist
		const ComponentRegistry* getComponentRegistry(uint32_t entityID) const;

		// @brief Checks if an entity exists in the registry
		// @param entityID The runtime entity ID
		// @returns True if the entity exists, false otherwise
		bool hasEntity(uint32_t entityID);
		// @brief Removes an entity from the registry
		// @param entityID The runtime entity ID
		// @returns True if the entity was removed, false if it did not exist
		bool removeEntity(uint32_t entityID);

		void clear();
		bool empty();

		virtual void serialize(nlohmann::json& j) const override;
		virtual void deserialize(const nlohmann::json& j) override;

		template<typename T, typename... Args>
		T* addComponent(uint32_t entityID, Args&&... args)
		{
			auto it = m_registry.find(entityID);
			if (it == m_registry.end())
			{
				return nullptr;
			}
			ComponentRegistry& components = it->second;
			auto [componentIt, status] = components.try_emplace(typeid(T), T(std::forward<Args>(args)...));

			return std::any_cast<T>(&componentIt->second);
		}

		template<typename T>
		T* getComponent(uint32_t entityID)
		{
			auto it = m_registry.find(entityID);
			if (it == m_registry.end())
			{
				return nullptr;
			}

			ComponentRegistry& components = it->second;

			auto componentIt = components.find(typeid(T));
			if (componentIt == components.end())
			{
				return nullptr;
			}
			return std::any_cast<T>(&componentIt->second);
		}

		template<typename T>
		bool removeComponent(uint32_t entityID)
		{
			auto it = m_registry.find(entityID);
			if (it == m_registry.end())
			{
				return false;
			}

			ComponentRegistry& components = it->second;
			auto componentIt = components.find(typeid(T));

			if (componentIt == components.end())
			{
				return false;
			}

			components.erase(componentIt);
			return true;
		}


		const Registry::const_iterator begin() const;
		const Registry::const_iterator end() const;


	private:
		const LoggerProxy& m_logger;
		Registry m_registry;

		// reserve 0 for invalid entity
		std::atomic<uint32_t> m_entityID{ 1 }; // The next available runtime entity ID
		uint64_t m_entitySSID{ 1 }; // The next available entity Scene Serialization ID

		std::unordered_map<uint32_t, uint64_t> m_runtimeToSerialized;
		std::unordered_map<uint64_t, uint32_t> m_serializedToRuntime;

		typedef std::function<void(uint32_t, const nlohmann::json&)> OnComponentAdded;
		typedef std::function<void(uint32_t, const nlohmann::json&)> OnComponentRemoved;

		// @brief Adds an entity with a Scene Serialization ID
		// @param entitySSID The Scene Serialization ID
		// @returns The runtime entity ID for the newly added entity
		uint32_t _addEntity(uint64_t entitySSID);

		// @brief Assigns a new unique runtime ID for an entity
		// @returns The newly assigned runtime entity ID
		uint32_t _assignID();
	};
}
