#include "pch.h"

#include "Entity.h"

#include "Component/ComponentStructs.h"
#include "Core/SubSystems/Events/EventSystem.h"
#include "Engine.h"
#include "ResourceManagers/ResourceManager.h"
#include <SubSystems/Scriptings/ScriptingSystem.h>

namespace SE
{
	using MT = ActiveSceneModified::ModifyType;


	Entity::Entity(const Entity& other) 
	{
		m_id = other.m_id;
		m_registry = other.m_registry;
	}

	Entity::Entity(uint32_t id, EntityRegistry* registry)
	    : m_id(id),
	      m_registry(registry)
	{}

	Entity::HookFunction Entity::_onNameComponentAdded()
	{
		HookFunction function = [](Entity entity)
		{
			auto* nameComp = entity.getComponent<NameComponent>();
			//printf("nameComponent added: %s\n", nameComp->name.c_str());
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

	Entity::HookFunction Entity::_onScriptComponentAdded()
	{
		HookFunction function = [](Entity entity)
		{
			auto* scriptComponent = entity.getComponent<ScriptComponent>();
			auto scriptingSystem = Engine::Instance().getSubSystem<ScriptingSystem>();
			scriptingSystem->loadScript(entity, scriptComponent->path);
		};
		return function;
	}

	Entity::HookFunction Entity::_onScriptComponentRemoved()
	{
		HookFunction function = [](Entity entity)
		{
			auto* scriptComponent = entity.getComponent<ScriptComponent>();
			auto scriptingSystem = Engine::Instance().getSubSystem<ScriptingSystem>();
			//scriptingSystem->removeScript(scriptComponent->path);
		};
		return function;
	}

}
