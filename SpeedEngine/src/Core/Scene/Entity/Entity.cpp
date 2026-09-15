#include "pch.h"
#include "Entity.h"
#include "Engine.h"

#include "../../SubSystems/Scenes/SceneSystem.h"

#include "../../SubSystems/Events/EventSystem.h"

#include "Component/Transform.h"
namespace SE
{
	using MT = ActiveSceneModified::ModifyType;


	uint32_t Entity::s_nextId = 1;

	Entity::Entity(std::string name) :
		m_instanceId(s_nextId++),
		m_name(name)
	{
		m_components.push_back(std::make_unique<Transform>());
	}

	Entity::Entity(const Entity& other) :
		m_instanceId(s_nextId++),
		m_name(other.m_name),
		m_isAlive(other.m_isAlive)
	{
	}

	void Entity::initializeEntity(Scene* parentScene)
	{
		m_parentScene = parentScene;
	}

	void Entity::setName(std::string_view name)
	{
		m_name = std::string(name);
		Engine::Instance().getSubSystem<EventSystem>()->publish(
			std::make_unique<ActiveSceneModified>(MT::Rename, m_instanceId, m_name)
		);
	}

	void Entity::addComponent(std::unique_ptr<Component> component)
	{
		if (!component)
		{
			m_parentScene->getLogger().critical("Attempted to add a null component to entity '{}'", m_name);
			assert(false && "Attempted to add a null component to entity");

			return;
		}

		m_components.push_back(std::move(component));
	}
}