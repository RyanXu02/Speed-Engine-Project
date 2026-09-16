#include "pch.h"
#include "Scene.h"
#include "Engine.h"

#include "../SubSystems/Events/EventSystem.h"

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

		if (!m_entitiesToRemove.empty()) {
			for (uint32_t id : m_entitiesToRemove) {
				m_registry.erase(m_registry.begin() + id);
			}
			m_entitiesToRemove.clear();
		}

		for (auto& entity : m_registry) {
			if (!entity.getComponentNew<LifeComponent>()->isAlive) {
				removeEntityNew(entity.id());
			}
		}
		
		m_isUpdating = false;
	}

	void Scene::shutdownScene()
	{
		m_logger.info("Shutting down");
		m_entities.clear();
		m_entitiesToAdd.clear();

		m_isInitialized = false;
	}

	uint32_t Scene::addEntityNew(const std::string& name)
	{
		m_registry.push_back(Entity(m_entityID));
		Entity* entity = getEntityNew(m_entityID);

		entity->addComponentNew<Transform>();
		entity->addComponentNew<NameComponent>(name);
		entity->addComponentNew<LifeComponent>(true);
		NameComponent* component = entity->getComponentNew<NameComponent>();

		return m_assignID();
	}

	Entity* Scene::getEntityNew(uint32_t instanceID)
	{
		if (instanceID < 0 || instanceID >= m_registry.size()) {
			return nullptr;
		}

		return &m_registry[instanceID];
	}

	bool Scene::removeEntityNew(uint32_t instanceId)
	{
		if (instanceId < 0 || instanceId >= m_registry.size()) {
			return false;
		}

		Entity* entity = getEntityNew(m_entityID);
		if (entity) {
			entity->getComponentNew<LifeComponent>()->isAlive = false;
		}
		m_entitiesToRemove.push_back(instanceId);

		return true;
	}

	uint32_t Scene::m_assignID()
	{
		return m_entityID.fetch_add(1, std::memory_order_relaxed);;
	}

	std::unordered_map<uint32_t, std::string> Scene::getEntityList()
	{
		std::unordered_map<uint32_t, std::string> entityList;
		for (auto& entity : m_registry)
		{
			LifeComponent* life = entity.getComponentNew<LifeComponent>();
			if (life && life->isAlive)
			{	
				entityList[entity.id()] = entity.getComponentNew<NameComponent>()->name;
			}
		}
		return entityList;
	}
}