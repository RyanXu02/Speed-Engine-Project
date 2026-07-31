#include "pch.h"
#include "Entity.h"

#include "../../SubSystems/Scenes/SceneSystem.h"

#include "../../SubSystems/Events/EventSystem.h"
namespace SE
{
	using MT = ActiveSceneModified::ModifyType;


	uint32_t Entity::s_nextId = 1;

	Entity::Entity(std::string name) :
		m_instanceId(s_nextId++),
		m_name(name)
	{
	}

	void Entity::initializeEntity(Scene* parentScene)
	{
		m_parentScene = parentScene;
	}

	void Entity::setName(std::string_view name)
	{
		m_name = std::string(name);
		EventSystem::Instance().publish(
			std::make_unique<ActiveSceneModified>(MT::Rename, m_instanceId, m_name)
		);
	}
}