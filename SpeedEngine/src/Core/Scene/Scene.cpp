#include "pch.h"
#include "Scene.h"

#include "../SubSystems/Events/EventSystem.h"

namespace SE
{
	void Scene::initializeScene()
	{
		m_logger.info("Initializing Scene");
		
		for (auto& entity : m_entities)
		{
			entity->initializeEntity(this);
		}

		m_isInitialized = true;
	}

	void Scene::updateScene(double deltaTime)
	{
		m_isUpdating = true;

		// update all entities
		for (auto& entity : m_entities)
		{
			if (entity->isAlive())
			{
				entity->updateEntity(deltaTime);
			}
		}

		// add new entities
		if (!m_entitiesToAdd.empty())
		{
			for (auto& entity : m_entitiesToAdd)
			{
				m_logger.debug("adding entity: {}", entity->getName());
				entity->initializeEntity(this);
				// Event is published with "isAdding" set to true
				EventSystem::Instance().publish(std::make_unique<ActiveSceneModified>(true, entity->getInstanceId(), entity->getName()));
				m_entities.push_back(std::move(entity));
			}
			m_entitiesToAdd.clear();
		}

		// delete dead entities
		auto it = std::remove_if(m_entities.begin(), m_entities.end(),
			[this](const std::unique_ptr<Entity>& entity) 
			{
				if (!entity->isAlive())
				{
					m_logger.debug("removing entity: {}", entity->getName());
					EventSystem::Instance().publish(
						// Event is published with "isAdding" set to false
						std::make_unique<ActiveSceneModified>(false, entity->getInstanceId(), entity->getName())
					);
					return true;
				}
				return false;			
			});

		m_entities.erase(it, m_entities.end());
		
		m_isUpdating = false;
	}

	void Scene::shutdownScene()
	{
		m_logger.info("Shutting down");
		for (auto& entity : m_entities)
		{
			entity->shutdownEntity();
		}
		m_entities.clear();
		m_entitiesToAdd.clear();

		m_isInitialized = false;
	}

	void Scene::addEntity(std::unique_ptr<Entity> entity)
	{
		if (m_isUpdating)
		{
			m_logger.debug("deferring addition of entity: {}", entity->getName());
			m_entitiesToAdd.push_back(std::move(entity));
		}
		else
		{
			m_logger.debug("adding entity: {}", entity->getName());
			entity->initializeEntity(this);
			// Event is published with "isAdding" set to true
			EventSystem::Instance().publish(std::make_unique<ActiveSceneModified>(true, entity->getInstanceId(), entity->getName()));
			m_entities.push_back(std::move(entity));
		}
	}

	Entity* Scene::getEntity(uint32_t instanceId)
	{
		for (auto& entity : m_entities)
		{
			if (entity->getInstanceId() == instanceId && entity->isAlive())
			{
				return entity.get();
			}
		}
		return nullptr;
	}

	void Scene::removeEntity(uint32_t instanceId)
	{
		for (auto& entity : m_entities)
		{
			if (entity->getInstanceId() == instanceId)
			{
				m_logger.debug("deferring remove entity: {}", entity->getName());
				entity->killEntity();
				return;
			}
		}

		// event is published with "isAdding" set to false
		EventSystem::Instance().publish(std::make_unique<ActiveSceneModified>(false, instanceId, ""));
	}

	std::unordered_map<uint32_t, std::string> Scene::getEntityList()
	{
		std::unordered_map<uint32_t, std::string> entityList;
		for (const auto& entity : m_entities)
		{
			if (entity->isAlive())
			{
				entityList[entity->getInstanceId()] = entity->getName();
			}
		}
		return entityList;
	}
}