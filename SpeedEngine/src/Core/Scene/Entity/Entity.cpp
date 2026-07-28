#include "pch.h"
#include "Entity.h"

#include "../../SubSystems/Scenes/SceneSystem.h"
namespace SE
{
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
}