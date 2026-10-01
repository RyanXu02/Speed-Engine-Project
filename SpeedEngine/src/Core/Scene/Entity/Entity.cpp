#include "pch.h"

#include "Entity.h"

#include "Component/ComponentStructs.h"
#include "Core/SubSystems/Events/EventSystem.h"
#include "Core/SubSystems/Scenes/SceneSystem.h"
#include "Engine.h"
#include "ResourceManagers/ResourceManager.h"

namespace SE
{
	using MT = ActiveSceneModified::ModifyType;


	Entity::Entity(uint32_t id, EntityRegistry* registry)
	    : m_id(id),
	      m_registry(registry)
	{}

	Entity::HookFunction Entity::_onNameComponentAdded()
	{
		HookFunction function = [](Entity entity)
		{
			auto* nameComp = entity.getComponent<NameComponent>();
			printf("nameComponent added: %s", nameComp->name.c_str());
		};
		return function;
	}

	Entity::HookFunction Entity::_onNameComponentRemoved()
	{
		HookFunction function = [](Entity entity)
		{
			auto* nameComp = entity.getComponent<NameComponent>();
			printf("nameComponent removed: %s", nameComp->name.c_str());
		};
		return function;
	}

	Entity::HookFunction Entity::_onMeshComponentAdded()
	{
		HookFunction function = [](Entity entity)
		{
			auto* meshComponent = entity.getComponent<MeshComponent>();

			auto meshResourceManager = Engine::Instance().getSubSystem<ResourceManager>();
			meshComponent->meshResourceID = meshResourceManager->addResource<ResourceType::MeshResource>(meshComponent->path, meshComponent->path);
		};
		return function;
	}

	Entity::HookFunction Entity::_onMeshComponentRemoved()
	{
		HookFunction function = [](Entity entity)
		{
			auto* meshComponent = entity.getComponent<MeshComponent>();
			auto meshResourceManager = Engine::Instance().getSubSystem<ResourceManager>();
			//meshResourceManager->removeResource(meshComponent->meshResourceID);
		};
		return function;
	}

}
