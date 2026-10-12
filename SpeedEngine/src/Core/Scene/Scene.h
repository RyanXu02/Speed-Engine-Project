#pragma once

#include "../Logger/LoggerProxy.h"
#include "Entity/Entity.h"
#include "Entity/EntityRegistry.h"

#include <Core/Features/Serializable.h>

namespace SE
{
	// @brief Represents a scene in the engine, which contains a collection of
	// entities
	class Scene : public Serializable
	{
	public:
		Scene(const std::string& name, Logger& logger)
		    : m_name(name),
		      m_logger(logger, fmt::format("Scene: {}", name)),
		      Serializable()
		{}

		void initializeScene();
		void updateScene(double deltaTime);
		void shutdownScene();
		void reloadScene();

		const std::string& getName() const { return m_name; }


		uint32_t addEntity(const std::string& name);
		bool hasEntity(uint32_t id);
		Entity getEntity(uint32_t id);
		bool removeEntity(uint32_t id);
		bool isEntityPendingRemoval(uint32_t id) const
		{
			return std::ranges::find(m_entitiesToRemove, id) != m_entitiesToRemove.end();
		}
		void saveScene(std::string_view path);
		void loadScene(std::string_view path);

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

		// @brief For use with UI widget
		std::unordered_map<uint32_t, std::string> getEntityList();

		LoggerProxy& getLogger() { return m_logger; }


		virtual void serialize(nlohmann::json& j) const override;
		virtual void deserialize(const nlohmann::json& j) override;

	private:
		LoggerProxy m_logger;
		std::string m_name;

		std::vector<uint32_t> m_entitiesToRemove{};

		EntityRegistry m_registry;

		bool m_isInitialized{ false };
		bool m_isUpdating{ false };
	};
} // namespace SE
