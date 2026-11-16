#include "pch.h"

#include "Entity.h"

#include "Component/ComponentStructs.h"
#include "Core/SubSystems/Events/EventSystem.h"
#include "Core/SubSystems/Physics/PhysicsSystem.h"
#include "Engine.h"
#include "ResourceManagers/ResourceManager.h"
#include "SubSystems/Scriptings/ScriptingSystem.h"

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

	Entity::HookFunction Entity::_onMeshComponentAdded()
	{
		HookFunction function = [](Entity entity)
		{
			auto* meshComponent = entity.getComponent<MeshComponent>();
			if (meshComponent->meshResourceID == 0 && !meshComponent->path.empty() && meshComponent->path != "None")
				meshComponent->setPath(meshComponent->path);
		};
		return function;
	}

	Entity::HookFunction Entity::_onTerrainComponentAdded()
	{
		HookFunction function = [](Entity entity)
		{
			auto* terrainComponent = entity.getComponent<TerrainComponent>();
			if (terrainComponent->terrainResourceID == 0 && !terrainComponent->path.empty() && terrainComponent->path != "None")
				terrainComponent->setPath(terrainComponent->path);
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
			auto scriptingSystem = Engine::Instance().getSubSystem<ScriptingSystem>();
			scriptingSystem->removeScript(entity);
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

	Entity::HookFunction Entity::_onCameraComponentRemoved()
	{
		return [](Entity entity)
		{
			EngineStates::releaseCamera(&entity.getComponent<CameraComponent>()->frustum);
		};
	}

	Entity::HookFunction Entity::_onRigidBodyComponentAdded()
	{
		return [](Entity entity)
		{
			auto* physicsSystem = Engine::Instance().getSubSystem<PhysicsSystem>();
			entity.getComponent<RigidBody>()->initialize(physicsSystem->getBodyInterface());
		};
	}

	Entity::HookFunction Entity::_onSFXComponentAdded()
	{
		HookFunction function = [](Entity entity)
		{
			auto* sfxComponent = entity.getComponent<AudioSFXComponent>();
			if (!sfxComponent->getFilepath().empty() && sfxComponent->getFilepath() != "None")
				sfxComponent->createSound(sfxComponent->getFilepath(), sfxComponent->isGameSFX());
		};
		return function;
	}

	Entity::HookFunction Entity::_onMusicComponentAdded()
	{
		HookFunction function = [](Entity entity)
		{
			auto* musicComponent = entity.getComponent<AudioMusicComponent>();
			if (!musicComponent->getFilepath().empty() && musicComponent->getFilepath() != "None")
				musicComponent->createTrack(musicComponent->getFilepath());
		};
		return function;
	}

	Entity::HookFunction Entity::_onSFXComponentRemoved()
	{
		return [](Entity entity)
		{
			entity.getComponent<AudioSFXComponent>()->StopMySound();
		};
	}


	Entity::HookFunction Entity::_onPhysicsShapeMeshComponentAdded() 
	{
		HookFunction function = [](Entity entity)
		{
			auto* physicsShapeMeshComponent = entity.getComponent<PhysicsShapeMesh>();
			const auto path = physicsShapeMeshComponent->getPath();
			if (physicsShapeMeshComponent->getMeshResourceID() == 0 && !path.empty() && path != "None")
			{
				auto meshResourceManager = Engine::Instance().getSubSystem<ResourceManager>();
				physicsShapeMeshComponent->setMeshResourceID(meshResourceManager->loadMesh({ .path = path, .name = path }));
			}
		};
		return function;
	}

	Entity::HookFunction Entity::_onPhysicsShapeTerrainComponentAdded()
	{
		HookFunction function = [](Entity entity)
		{
			auto* physicsShapeTerrainComponent = entity.getComponent<PhysicsShapeTerrain>();
			const auto path = physicsShapeTerrainComponent->getPath();
			if (physicsShapeTerrainComponent->getTerrainID() == 0 && !path.empty() && path != "None")
			{
				auto terrainManager = Engine::Instance().getSubSystem<ResourceManager>();
				physicsShapeTerrainComponent->setTerrainID(terrainManager->loadTerrain({ .path = path, .name = path }));
			}
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
