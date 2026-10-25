#include "pch.h"
#include "PhysicsSystem.h"

#include "Engine.h"
#include "Core/Scene/Scene.h"
#include "Core/SubSystems/Scenes/SceneSystem.h"
#include "Core/Scene/Entity/Component/Transform.h"
#include "Core/Scene/Entity/Component/RigidBody.h"
#include "Core/Scene/Entity/Component/PhysicsShape/PhysicsShape.h"
#include "Core/Scene/Entity/Component/PhysicsShape/PhysicsShapeBox.h"
#include "Core/Scene/Entity/Component/PhysicsShape/PhysicsShapeCapsule.h"
#include "Core/Scene/Entity/Component/PhysicsShape/PhysicsShapeSphere.h"
#include "Core/Scene/Entity/Component/PhysicsShape/PhysicsShapeMesh.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <Jolt/Physics/Body/BodyManager.h>
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

		BodyInterface& body_interface = m_physics_system->GetBodyInterface();
	}

	void PhysicsSystem::createAndAddComponents() {
		// Initialize all uninitialized RigidBody components at once and add them to the physics world
		// Update initial rigidbody transforms to match entities
		Scene* scene = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene();
		auto entities = scene->getEntitiesWith<Transform, RigidBody>();
		std::vector<BodyID> ioBodies;
		for (auto& entity : entities)
		{
			RigidBody* rigidBody = entity.getComponent<RigidBody>();
			Transform* transform = entity.getComponent<Transform>();
			PhysicsShape* shape = nullptr;
			shape = entity.getComponent<PhysicsShapeBox>();
			if (!shape) shape = entity.getComponent<PhysicsShapeCapsule>();
			if (!shape) shape = entity.getComponent<PhysicsShapeSphere>();
			///if (!shape) shape = entity.getComponent<PhysicsShapeMesh>();


			// Only add if not already created/added and if it has a collider shape component
			if (!rigidBody->isCreated() && shape)
			{
				rigidBody->setShape(shape->getShape());
				rigidBody->createBody();
				rigidBody->setPosition(transform->getTranslation() + rigidBody->getOffset());
				rigidBody->setRotation(glm::quat(transform->getRotation()));
				ioBodies.push_back(rigidBody->getBodyID());
			}
		}
		// Batch add all bodies for better performance
		BodyInterface::AddState addState = getBodyInterface()->AddBodiesPrepare(ioBodies.data(), static_cast<int>(ioBodies.size()));
		getBodyInterface()->AddBodiesFinalize(ioBodies.data(), static_cast<int>(ioBodies.size()), addState, EActivation::Activate);
		m_physics_system->OptimizeBroadPhase();
	}

	void PhysicsSystem::step(double deltaTime) {
		const int cCollisionSteps = 1; // If stepping physics less that 60hz, do more collision steps per simulation step

		// Step physics simulation
		if (m_running)
			m_physics_system->Update(static_cast<float>(deltaTime), cCollisionSteps, m_temp_allocator.get(), m_job_system.get());

		// Update entity transforms
		Scene* scene = Engine::Instance().getSubSystem<SceneSystem>()->getCurrentScene();
		auto entities = scene->getEntitiesWith<Transform, RigidBody>();
		for (auto& entity : entities)
		{
			RigidBody* rigidBody = entity.getComponent<RigidBody>();
			Transform* transform = entity.getComponent<Transform>();
			PhysicsShape* shape = nullptr;
			// Check for each type of physics shape component
			shape = entity.getComponent<PhysicsShapeBox>();
			if (!shape) shape = entity.getComponent<PhysicsShapeCapsule>();
			if (!shape) shape = entity.getComponent<PhysicsShapeSphere>();
			///if (!shape) shape = entity.getComponent<PhysicsShapeMesh>();
			if (shape)
			{
				rigidBody->setShape(shape->getShape());
			}

			if (!rigidBody->isCreated())
			{
				// Any new bodies will be added to the system
				// Note: this is inefficient if many new entities are being created at once,
				// In that case use createAndAddComponents to do it in a batch
				rigidBody->createBody();
				rigidBody->setPosition(transform->getTranslation() + rigidBody->getOffset());
				rigidBody->setRotation(glm::quat(transform->getRotation()));
				getBodyInterface()->AddBody(rigidBody->getBodyID(), rigidBody->isActive() ? EActivation::Activate : EActivation::DontActivate);
			}
			else if (rigidBody->getMotionType() != EMotionType::Static)
			{
				///transform->setTransform(rigidBody->getTransform()); IDEAL
				transform->setPosition(rigidBody->getPosition() - rigidBody->getOffset());
				glm::quat rotationQ = rigidBody->getRotation();
				float rotationPitch = glm::pitch(rotationQ);
				float rotationYaw = glm::yaw(rotationQ);
				float rotationRoll = glm::roll(rotationQ);
				transform->setRotation(glm::vec3(rotationPitch, rotationYaw, rotationRoll));
			}
		}
	}

	void PhysicsSystem::shutdown() {
		SubSystem::shutdown();
		// Shut down physics system
		delete m_physicsDebugRenderer;
		UnregisterTypes();
		delete Factory::sInstance;
		Factory::sInstance = nullptr;
	}

	void PhysicsSystem::setDebugRendererImpl(SE::DebugRenderer& renderer) {
		if (!m_physicsDebugRenderer)
			m_physicsDebugRenderer = new PhysicsDebugRenderer(renderer);
	}

	void PhysicsSystem::debugDrawBodies() {
		BodyManager::DrawSettings drawSettings;
		drawSettings.mDrawShape = true;
		drawSettings.mDrawShapeWireframe = true;
		m_physics_system->DrawBodies(drawSettings, JPH::DebugRenderer::sInstance);
	}

	void PhysicsSystem::debugUpdateCameraPos(const glm::vec3& pos) {
		static_cast<PhysicsDebugRenderer*>(JPH::DebugRenderer::sInstance)->SetCameraPos(JPH::Vec3(pos.x, pos.y, pos.z));
	}
}
