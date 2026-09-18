#include "pch.h"

#include "Entity.h"

#include "../../SubSystems/Events/EventSystem.h"
#include "../../SubSystems/Scenes/SceneSystem.h"
#include "Component/Transform.h"
#include "Engine.h"
namespace SE
{
	using MT = ActiveSceneModified::ModifyType;


	Entity::Entity(uint32_t id)
	    : m_id(id)
	{}
} // namespace SE
