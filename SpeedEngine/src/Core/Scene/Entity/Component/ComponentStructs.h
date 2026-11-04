#pragma once

#include "Transform.h"
#include "RigidBody.h"
#include "PhysicsShape/PhysicsShapeBox.h"
#include "PhysicsShape/PhysicsShapeCapsule.h"
#include "PhysicsShape/PhysicsShapeSphere.h"
#include "PhysicsShape/PhysicsShapeMesh.h"
#include "Core/features/Serializable.h"
#include "Core/features/CameraFrustum.h"
#include "Core/EngineStates.h"
#include "AudioComponents.h"
#include "Particles/ParticleEmitter.h"


namespace SE
{
	struct NameComponent
	{
		NameComponent() = default;
		NameComponent(std::string_view n)
		    : name(n){};

		std::string name = "Unknown";

		DEFINE_TYPE(NameComponent, member("name", &NameComponent::name));
	};

	struct MeshComponent
	{
		MeshComponent() = default;
		MeshComponent(std::string p)
		    : path(p){};
		MeshComponent(uint32_t meshId)
		    : meshResourceID(meshId)
		{}

		uint32_t meshResourceID{ 0 };
		std::string path = "None";

		DEFINE_TYPE(MeshComponent, member("path", &MeshComponent::path));
	};

	struct LifeComponent
	{
		LifeComponent() = default;
		LifeComponent(bool alive)
		    : isAlive(alive)
		{}

		bool isAlive{ true };

		DEFINE_TYPE(LifeComponent, member("isAlive", &LifeComponent::isAlive));
	};

	struct RelationshipComponent
	{
		RelationshipComponent() = default;

		uint32_t parent{ 0 };
		std::vector<uint32_t> children;
	};

	struct PrefabComponent
	{
		PrefabComponent() = default;

		std::string prefabPath = "None";

		DEFINE_TYPE(PrefabComponent, member("prefabPath", &PrefabComponent::prefabPath));
	};

	struct ScriptComponent
	{
		ScriptComponent() = default;
		ScriptComponent(std::string scriptPath)
		    : path(scriptPath) {};

		std::string path{ "None" };
		std::function<void()> onInit = []() {};
		std::function<void(double)> onUpdate = [](double) {};
		std::function<void()> onDestroy = []() {};
		bool initilized{ false };

		DEFINE_TYPE(ScriptComponent, member("path", &ScriptComponent::path));
	};

	
	struct CameraComponent
	{
		CameraComponent() = default;
		~CameraComponent() { EngineStates::releaseCamera(&frustum); }

		CameraFrustum frustum;

		float fov{ 90.0f };
		float nearPlane{ 0.1f };
		float farPlane{ 1000.0f };
		bool isPrimary{ false };

		DEFINE_TYPE(
			CameraComponent,
			member("fov", &CameraComponent::fov),
			member("nearPlane", &CameraComponent::nearPlane),
			member("farPlane", &CameraComponent::farPlane),
			member("isPrimary", &CameraComponent::isPrimary)
		);
	};

	struct LightComponent
	{
		LightComponent() = default;

		glm::vec3 color{ 1.0f };
		float intensity{ 300.0f };

		DEFINE_TYPE(
			LightComponent,
			member("color", &LightComponent::color),
			member("intensity", &LightComponent::intensity)
		);
	};
}
