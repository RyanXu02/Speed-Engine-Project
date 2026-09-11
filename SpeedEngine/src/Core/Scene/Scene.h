#pragma once

#include "Entity/Entity.h"
#include "../Logger/LoggerProxy.h"


namespace SE
{
	// @brief Represents a scene in the engine, which contains a collection of entities
	class Scene
	{
	public:
		Scene(const std::string& name, Logger& logger) : m_name(name), m_logger(logger, fmt::format("Scene: {}", name)) {}
		
		void initializeScene();
		void updateScene(double deltaTime);
		void shutdownScene();
		
		const std::string& getName() const { return m_name; }


		// @brief For use with UI widget
		std::unordered_map<uint32_t, std::string> getEntityList();

		void addEntity(std::unique_ptr<Entity> entity);

		Entity* getEntity(uint32_t instanceId);
		void removeEntity(uint32_t instanceId);

		LoggerProxy& getLogger() { return m_logger; }

		const std::vector<std::unique_ptr<Entity>>& getEntities() const { return m_entities; }
	
	private:
		LoggerProxy m_logger;
		std::string m_name;

		std::vector<std::unique_ptr<Entity>> m_entities;
		std::vector<std::unique_ptr<Entity>> m_entitiesToAdd;

		bool m_isInitialized{ false };
		bool m_isUpdating{ false };
	};
}

