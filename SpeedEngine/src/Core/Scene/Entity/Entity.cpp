#include "pch.h"
#include "Entity.h"
#include "Engine.h"

#include "../../SubSystems/Scenes/SceneSystem.h"

#include "../../SubSystems/Events/EventSystem.h"

#include "Component/Transform.h"
namespace SE
{
	using MT = ActiveSceneModified::ModifyType;


	Entity::Entity(uint32_t id) : m_id(id)
	{
	}
}