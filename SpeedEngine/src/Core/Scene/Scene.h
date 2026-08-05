#pragma once

#include "Entity/Entity.h"
#include "../Logger/LoggerProxy.h"


namespace SE
{
	class Scene
	{
	public:
		Scene(const std::string& name, Logger& logger) : m_name(name), m_logger(logger, fmt::format("Scene: {}", name)) {}
		
		void initializeScene();
		void updateScene(double deltaTime);
		void shutdownScene();
		
		const std::string& getName() const { return m_name; }


		//for use with ui widget
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

