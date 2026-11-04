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

		uint32_t addEntity(const std::string& name);
		bool hasEntity(uint32_t id);
		Entity getEntity(uint32_t id);
		bool removeEntity(uint32_t id);
		bool isEntityPendingRemoval(uint32_t id) const { return std::ranges::find(m_entitiesToRemove, id) != m_entitiesToRemove.end(); }
		bool readScene(std::string_view path, nlohmann::json& sceneJson) const;
		bool saveScene(std::string_view path);
		bool loadScene(std::string_view path);

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

		// @brief For use with UI widget
		std::unordered_map<uint32_t, std::string> getEntityList();

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
