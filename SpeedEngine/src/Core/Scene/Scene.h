#pragma once

#include "Logger/LoggerProxy.h"
#include "Entity/Entity.h"
#include "Entity/EntityRegistry.h"

#include "Core/Features/Serializable.h"

namespace SE
{
	// @brief Represents a scene in the engine, which contains a collection of entities
	class Scene : public Serializable
	{
	public:
		Scene(const std::string& name, Logger& logger)
			: Serializable(),
			  m_logger(logger, fmt::format("Scene: {}", name)),
			  m_name(name),
			  m_registry(m_logger)
		{}

		void initializeScene();
		void updateScene(double deltaTime);
		void shutdownScene();

		const std::string& getName() const { return m_name; }
		const std::string& getSourcePath() const { return m_path; }

		// @brief Adds a new entity to the scene
		// @param name The name of the entity
		// @returns The runtime ID of the newly added entity
		uint32_t addEntity(const std::string& name);

		// @brief Get a Scene Serialization ID for a given runtime entity ID
		// @param entityID The runtime entity ID
		// @returns The Scene Serialization ID for the entity, or 0 if the entity does not exist
		uint64_t getEntitySSID(uint32_t id) const
		{
			return isEntityPendingRemoval(id) ? 0 : m_registry.getEntitySSID(id);
		}
		// @brief Get a runtime entity ID for a given Scene Serialization ID
		// @param ssid The Scene Serialization ID
		// @returns The runtime entity ID for the entity, or 0 if the entity does not exist
		uint32_t getEntityID(uint64_t ssid) const
		{
			const uint32_t id = m_registry.getEntityID(ssid);
			return isEntityPendingRemoval(id) ? 0 : id;
		}

		// @brief Checks if an entity exists in the scene
		// @param id The runtime entity ID
		bool hasEntity(uint32_t id);
		// @brief Retrieves an entity from the scene
		// @param id The runtime entity ID
		Entity getEntity(uint32_t id);
		// @brief Removes an entity from the scene
		// @param id The runtime entity ID
		bool removeEntity(uint32_t id);

		// @brief Checks if an entity is pending removal from the scene
		// @param id The runtime entity ID
		bool isEntityPendingRemoval(uint32_t id) const { return std::ranges::find(m_entitiesToRemove, id) != m_entitiesToRemove.end(); }
		
		bool readScene(std::string_view path, nlohmann::json& sceneJson) const;
		bool saveScene(std::string_view path);
		bool loadScene(std::string_view path);

		// @brief Saves one entity as an independent component template
		bool savePrefab(uint32_t id, const std::string& path) const;
		// @brief Creates an ordinary entity from a component template with fresh IDs
		uint32_t instantiatePrefab(const std::string& path);

		template<typename... Ts>
		std::vector<Entity> getEntitiesWith()
		{
			std::vector<Entity> entities;

			for (auto& [id, components] : m_registry)
			{
				Entity entity = getEntity(id);
				if ((entity.getComponent<Ts>() && ...))
				{
					entities.push_back(entity);
				}
			};
			return entities;
		}

		template<typename... Ts>
		void forEntitiesWith(std::function<void(Entity)> callback)
		{
			for (auto& [id, components] : m_registry)
			{
				Entity entity = getEntity(id);
				if ((entity.getComponent<Ts>() && ...))
				{
					callback(entity);
				}
			};
		}

		LoggerProxy& getLogger() { return m_logger; }


		virtual void serialize(nlohmann::json& j) const override;
		virtual void deserialize(const nlohmann::json& j) override;

	private:
		bool validateSceneJson(const nlohmann::json& j) const;

		LoggerProxy m_logger;
		std::string m_name;
		std::string m_path;

		std::vector<uint32_t> m_entitiesToRemove{};

		EntityRegistry m_registry;

		bool m_isInitialized{ false };
	};
} // namespace SE
