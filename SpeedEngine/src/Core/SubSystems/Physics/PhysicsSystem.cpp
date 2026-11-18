#include "pch.h"
#include "PhysicsSystem.h"

#include "Engine.h"
#include "Core/ResourceManagers/ResourceManager.h"
#include "Core/Scene/Scene.h"
#include "Core/SubSystems/Scenes/SceneSystem.h"
#include "Core/Scene/Entity/Component/ComponentStructs.h"
#include "Core/Scene/Entity/Component/RigidBody.h"
#include "Core/Scene/Entity/Component/PhysicsShape/PhysicsShape.h"
#include "Core/Scene/Entity/Component/PhysicsShape/PhysicsShapeBox.h"
#include "Core/Scene/Entity/Component/PhysicsShape/PhysicsShapeCapsule.h"
#include "Core/Scene/Entity/Component/PhysicsShape/PhysicsShapeSphere.h"
#include "Core/Scene/Entity/Component/PhysicsShape/PhysicsShapeMesh.h"
#include "Core/Scene/Entity/Component/PhysicsShape/PhysicsShapeTerrain.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <Jolt/Physics/Body/BodyManager.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Renderer/DebugRenderer.h>

using namespace JPH;
using namespace JPH::literals;

namespace SE
{
	void PhysicsSystem::init() {
		SubSystem::init();

		Factory::sInstance = new Factory();
		RegisterTypes();
		m_temp_allocator = std::make_unique<TempAllocatorImpl>(10 * 1024 * 1024);
		m_job_system = std::make_unique<JobSystemThreadPool>(cMaxPhysicsJobs, cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

		m_physics_system = std::make_unique<JPH::PhysicsSystem>();
		m_physics_system->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, m_broad_phase_layer_interface, m_object_vs_broadphase_layer_filter, m_obj_vs_object_layer_filter);

		m_contact_listener = std::make_unique<PhysicsContactListener>();
		m_physics_system->SetContactListener(m_contact_listener.get());

		// BodyInterface& body_interface = m_physics_system->GetBodyInterface();
	}

	JPH::Ref<JPH::Shape> PhysicsSystem::assetShape(ResourceID id, bool terrain)
	{
		if (!id)
			return nullptr;

		if (const auto cached = m_assetShapes.find(id); cached != m_assetShapes.end())
			return cached->second;

		const auto& assets = *Engine::Instance().getSubSystem<ResourceManager>();
		JPH::Shape::ShapeResult result;
		if (terrain)
		{
			const auto* data = assets.getResource<Terrain>(id);
			if (!data)
				return nullptr;

			if (data->heightSamples.empty())
				return m_assetShapes.emplace(id, nullptr).first->second;

			const float cell = 2.0f / data->heightSampleCount;
			JPH::HeightFieldShapeSettings settings(data->heightSamples.data(), { -1.0f, 0.0f, -1.0f },
				{ cell, data->getHeightScale(), cell }, data->heightSampleCount);
			result = settings.Create();
		}
		else
		{
			const auto* data = assets.getResource<MeshResource>(id);
			if (!data)
				return nullptr;

			const auto& vertices = data->getVertices();
			const auto& indices = data->getIndices();
			JPH::TriangleList triangles;
			triangles.reserve(indices.size() / 3);
			for (const auto& sub : data->getSubMeshes())
				for (uint32_t i = sub.indexOffset; i < sub.indexOffset + sub.indexCount; i += 3)
					triangles.emplace_back(toJPH(vertices[sub.vertexOffset + indices[i]].position),
						toJPH(vertices[sub.vertexOffset + indices[i + 1]].position),
						toJPH(vertices[sub.vertexOffset + indices[i + 2]].position));
			JPH::MeshShapeSettings settings(triangles);
			result = settings.Create();
		}

		if (result.HasError())
			m_logger->warn("Collision shape for resource {} failed: {}", id, result.GetError().c_str());

		return m_assetShapes.emplace(id, result.IsValid() ? result.Get() : JPH::Ref<JPH::Shape>{}).first->second;
	}

	bool PhysicsSystem::syncBodyShape(Entity& entity, RigidBody& body, const Transform& transform)
	{
		PhysicsShape* primitive = entity.getComponent<PhysicsShapeBox>();
		if (!primitive) primitive = entity.getComponent<PhysicsShapeCapsule>();
		if (!primitive) primitive = entity.getComponent<PhysicsShapeSphere>();
		if (primitive)
		{
			primitive->setScale(transform.scale);
			body.setShape(primitive->getShape(), false);
			return body.getShape() != nullptr;
		}

		JPH::Ref<JPH::Shape> base;
		if (const auto* mesh = entity.getComponent<PhysicsShapeMesh>())
			base = assetShape(mesh->getMeshResourceID(), false);
		else if (const auto* terrain = entity.getComponent<PhysicsShapeTerrain>())
			base = assetShape(terrain->getTerrainID(), true);

		if (!base)
		{
			body.setShape(nullptr, false);
			return false;
		}

		const auto existing = body.getShape();
		const auto scale = toJPH(transform.scale);
		if (existing && existing->GetSubType() == JPH::EShapeSubType::Scaled)
		{
			const auto* scaled = static_cast<const JPH::ScaledShape*>(existing.GetPtr());
			if (scaled->GetInnerShape() == base.GetPtr() && scaled->GetScale() == scale)
				return true;
		}

		JPH::ScaledShapeSettings settings(base.GetPtr(), scale);
		const auto result = settings.Create();
		body.setShape(result.IsValid() ? result.Get() : JPH::Ref<JPH::Shape>{}, true);
		return result.IsValid();
	}

	void PhysicsSystem::createAndAddComponents() {
		// Initialize all uninitialized RigidBody components at once and add them to the physics world
		// Update initial rigidbody transforms to match entities
		Scene* scene = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene();
		if (!scene)
			return;
		auto entities = scene->getEntitiesWith<Transform, RigidBody>();
		std::vector<BodyID> ioBodies;
		for (auto& entity : entities)
		{
			RigidBody* rigidBody = entity.getComponent<RigidBody>();
			Transform* transform = entity.getComponent<Transform>();
			if (!rigidBody->isCreated() && syncBodyShape(entity, *rigidBody, *transform))
			{
				rigidBody->createBody();
				if (!rigidBody->isCreated())
					continue;
				rigidBody->setPosition(transform->translate + (rigidBody->getOffset() * glm::conjugate(transform->rotate)));
				rigidBody->setRotation(transform->rotate);
				ioBodies.push_back(rigidBody->getBodyID());
			}
		}
		// Batch add all bodies for better performance
		if (ioBodies.empty())
			return;
		BodyInterface::AddState addState = getBodyInterface()->AddBodiesPrepare(ioBodies.data(), static_cast<int>(ioBodies.size()));
		getBodyInterface()->AddBodiesFinalize(ioBodies.data(), static_cast<int>(ioBodies.size()), addState, EActivation::Activate);
		m_physics_system->OptimizeBroadPhase();
	}

	void PhysicsSystem::step(double deltaTime) {
		const int cCollisionSteps = 1; // If stepping physics less that 60hz, do more collision steps per simulation step

		// Install newly published shapes before the world steps
		Scene* scene = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene();
		if (!scene)
			return;
		auto entities = scene->getEntitiesWith<Transform, RigidBody>();
		for (auto& entity : entities)
		{
			RigidBody* rigidBody = entity.getComponent<RigidBody>();
			Transform* transform = entity.getComponent<Transform>();
			if (!syncBodyShape(entity, *rigidBody, *transform))
				continue;

			if (!rigidBody->isCreated())
			{
				// Any new bodies will be added to the system
				// Note: this is inefficient if many new entities are being created at once,
				// In that case use createAndAddComponents to do it in a batch
				rigidBody->createBody();
				if (!rigidBody->isCreated())
					continue;

				rigidBody->setPosition(transform->translate + (rigidBody->getOffset() * glm::conjugate(transform->rotate)));
				rigidBody->setRotation(transform->rotate);
				getBodyInterface()->AddBody(rigidBody->getBodyID(), rigidBody->isActive() ? EActivation::Activate : EActivation::DontActivate);
			}
			else if (rigidBody->getMotionType() == EMotionType::Static || !m_running)
			{
				// Allows for transform to be changed while physics is paused (or for static objects)
				rigidBody->setPosition(transform->translate + (rigidBody->getOffset() * glm::conjugate(transform->rotate)));
				rigidBody->setRotation(transform->rotate);
			}
		}

		if (!m_running)
			return;

		m_physics_system->Update(static_cast<float>(deltaTime), cCollisionSteps, m_temp_allocator.get(), m_job_system.get());
		
		for (auto& entity : entities)
		{
			const auto* body = entity.getComponent<RigidBody>();
			if (!body->isCreated() || body->getMotionType() == EMotionType::Static)
				continue;
			auto* transform = entity.getComponent<Transform>();
			transform->rotate = body->getRotation();
			transform->translate = body->getPosition() - body->getOffset() * glm::conjugate(transform->rotate);
		}
	}

	void PhysicsSystem::shutdown() {
		SubSystem::shutdown();
		// Shut down physics system
		m_physics_system.reset();
		m_assetShapes.clear();
		m_physicsDebugRenderer.reset();
		m_job_system.reset();
		m_temp_allocator.reset();
		m_contact_listener.reset();
		UnregisterTypes();
		delete Factory::sInstance;
		Factory::sInstance = nullptr;
	}

	void PhysicsSystem::appendDebugGeometry(RenderFrame& frame, const glm::vec3& cameraPosition) {
		if (!m_physics_system)
			return;
		if (!m_physicsDebugRenderer)
			m_physicsDebugRenderer = std::make_unique<PhysicsDebugRenderer>();
		m_physicsDebugRenderer->appendGeometry(frame, *m_physics_system, cameraPosition);
	}
}
