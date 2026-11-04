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
			meshComponent->meshResourceID = meshResourceManager->loadMesh({ .path = meshComponent->path, .name = meshComponent->path });
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
			scriptingSystem->removeScript(scriptComponent->path);
		};
		return function;
	}

	Entity::HookFunction Entity::_onCameraComponentAdded()
	{
		HookFunction function = [](Entity entity)
		{
			auto* cameraComponent = entity.getComponent<CameraComponent>();
			cameraComponent->frustum.setFOV(cameraComponent->fov);
			cameraComponent->frustum.setNearPlane(cameraComponent->nearPlane);
			cameraComponent->frustum.setFarPlane(cameraComponent->farPlane);
		};
		return function;
	}
	Entity::HookFunction Entity::_onSFXComponentAdded()
	{
		HookFunction function = [](Entity entity)
		{
			auto* sfxComponent = entity.getComponent<AudioSFXComponent>();
			sfxComponent->createSound(sfxComponent->getFilepath(), sfxComponent->isGameSFX());
		};
		return function;
	}

	Entity::HookFunction Entity::_onMusicComponentAdded()
	{
		HookFunction function = [](Entity entity)
		{
			auto* sfxComponent = entity.getComponent<AudioMusicComponent>();
			sfxComponent->createTrack(sfxComponent->getFilepath());
		};
		return function;
	}


	Entity::HookFunction Entity::_onPhysicsShapeMeshComponentAdded() 
	{
		HookFunction function = [](Entity entity)
		{
			auto* physicsShapeMeshComponent = entity.getComponent<PhysicsShapeMesh>();

			auto meshResourceManager = Engine::Instance().getSubSystem<ResourceManager>();
			physicsShapeMeshComponent->setMeshResourceID(meshResourceManager->loadMesh({ .path = physicsShapeMeshComponent->getPath(), .name = physicsShapeMeshComponent->getPath() }));
		};
		return function;
	}

	Entity::HookFunction Entity::_onParticleEmitterAdded()
	{
		HookFunction function = [](Entity entity)
		{
			auto* pe = entity.getComponent<ParticleEmitter>();
			pe->finalize();
		};
		return function;
	}
}
