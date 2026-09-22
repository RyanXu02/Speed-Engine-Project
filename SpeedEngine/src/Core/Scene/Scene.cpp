#include "pch.h"

#include "Scene.h"

#include "../SubSystems/Events/EventSystem.h"
#include "Engine.h"

namespace SE
{
	using MT = ActiveSceneModified::ModifyType;

	void Scene::initializeScene()
	{
		m_logger.info("Initializing Scene");


		m_isInitialized = true;
	}

	void Scene::updateScene(double deltaTime)
	{
		m_isUpdating = true;

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
			if (!entity.getComponent<LifeComponent>()->isAlive)
			{
				removeEntity(entity.id());
			}
		}

		m_isUpdating = false;
	}

	void Scene::shutdownScene()
	{
		m_logger.info("Shutting down");
		m_registry.clear();

		m_isInitialized = false;
	}

	uint32_t Scene::addEntity(const std::string& name)
	{
		uint32_t entityID = m_registry.addEntity();
		Entity entity = getEntity(entityID);

		entity.addComponent<Transform>();
		entity.addComponent<NameComponent>(name);
		entity.addComponent<LifeComponent>(true);
		NameComponent* component = entity.getComponent<NameComponent>();

		return entityID;
	}

	bool Scene::hasEntity(uint32_t id)
	{ 
		if (!m_registry.hasEntity(id))
		{
			m_logger.warn("there is no entity with given name");
			return false;
		}

		return m_registry.hasEntity(id);
	}

	Entity Scene::getEntity(uint32_t id)
	{
		assert(hasEntity(id));

		return Entity(id, &m_registry);
	}

	bool Scene::removeEntity(uint32_t id)
	{
		m_registry.hasEntity(id);
		m_entitiesToRemove.push_back(id);

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
} // namespace SE
