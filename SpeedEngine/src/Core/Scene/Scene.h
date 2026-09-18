#pragma once

#include "../Logger/LoggerProxy.h"
#include "Entity/Entity.h"

namespace SE
{
	// @brief Represents a scene in the engine, which contains a collection of
	// entities
	class Scene
	{
	public:
		Scene(const std::string& name, Logger& logger)
		    : m_name(name),
		      m_logger(logger, fmt::format("Scene: {}", name))
		{}

		void initializeScene();
		void updateScene(double deltaTime);
		void shutdownScene();

		const std::string& getName() const { return m_name; }


		uint32_t addEntity(const std::string& name);
		Entity* getEntity(uint32_t instanceId);
		bool removeEntity(uint32_t instanceId);

		template<typename... Ts>
		std::vector<Entity*> getEntitiesWith()
		{
			std::vector<Entity*> entities;

			for (auto& entity : m_registry)
			{
				if ((entity.getComponent<Ts>() && ...))
				{
					entities.push_back(&entity);
				}
			};
			return entities;
		}


		// @brief For use with UI widget
		std::unordered_map<uint32_t, std::string> getEntityList();

		LoggerProxy& getLogger() { return m_logger; }

	private:
		LoggerProxy m_logger;
		std::string m_name;

		std::atomic<uint32_t> m_entityID{ 0 };
		std::vector<Entity> m_registry{};
		std::vector<uint32_t> m_entitiesToRemove{};

		bool m_isInitialized{ false };
		bool m_isUpdating{ false };

		uint32_t m_assignID();
	};
} // namespace SE
