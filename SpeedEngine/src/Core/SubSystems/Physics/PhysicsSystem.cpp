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

using namespace JPH;
using namespace JPH::literals;

// Using boilerplate code from https://github.com/jrouwe/JoltPhysics/blob/master/HelloWorld/HelloWorld.cpp

namespace Layers
{
	static constexpr ObjectLayer NON_MOVING = 0;
	static constexpr ObjectLayer MOVING = 1;
	static constexpr ObjectLayer NUM_LAYERS = 2;
};

/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
{
public:
	virtual bool ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
	{
		switch (inObject1)
		{
		case Layers::NON_MOVING: return inObject2 == Layers::MOVING; // Non moving only collides with moving
		case Layers::MOVING: return true; // Moving collides with everything
		default: JPH_ASSERT(false); return false;
		}
	}
};

namespace BroadPhaseLayers
{
	static constexpr BroadPhaseLayer NON_MOVING(0);
	static constexpr BroadPhaseLayer MOVING(1);
	static constexpr uint NUM_LAYERS(2);
};

// BroadPhaseLayerInterface implementation
// This defines a mapping between object and broadphase layers.
class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
{
public:
	BPLayerInterfaceImpl()
	{
		// Create a mapping table from object to broad phase layer
		mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
		mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
	}

	virtual uint GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::NUM_LAYERS; }

	virtual BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer inLayer) const override
	{
		JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
		return mObjectToBroadPhase[inLayer];
	}

private:
	BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
};

/// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
{
public:
	virtual bool ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
	{
		switch (inLayer1)
		{
		case Layers::NON_MOVING: return inLayer2 == BroadPhaseLayers::MOVING;
		case Layers::MOVING: return true;
		default: JPH_ASSERT(false); return false;
		}
	}
};

namespace SE
{
	void PhysicsSystem::init() {
		SubSystem::init();

		// Allocation hook, can connect this to custom memory control if needed (see Jolt/Core/Memory.h)
		RegisterDefaultAllocator();

		Factory::sInstance = new Factory();
		RegisterTypes();
		m_temp_allocator = std::make_unique<TempAllocatorImpl>(10 * 1024 * 1024);
		m_job_system = std::make_unique<JobSystemThreadPool>(cMaxPhysicsJobs, cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

		BPLayerInterfaceImpl broad_phase_layer_interface;
		ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
		ObjectLayerPairFilterImpl obj_vs_object_layer_filter;

		m_physics_system = std::make_unique<JPH::PhysicsSystem>();
		m_physics_system->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, obj_vs_object_layer_filter);

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
		BodyInterface::AddState addState = getBodyInterface()->AddBodiesPrepare(ioBodies.data(), ioBodies.size());
		getBodyInterface()->AddBodiesFinalize(ioBodies.data(), ioBodies.size(), addState, EActivation::Activate);
		m_physics_system->OptimizeBroadPhase();
	}

	void PhysicsSystem::step(double deltaTime) {
		const int cCollisionSteps = 1; // If stepping physics less that 60hz, do more collision steps per simulation step

		// Step physics simulation
		m_physics_system->Update(deltaTime, cCollisionSteps, m_temp_allocator.get(), m_job_system.get());

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
				getBodyInterface()->AddBody(rigidBody->getBodyID(), EActivation::Activate);
			}
			else if (rigidBody->getMotionType() != EMotionType::Static)
			{
				/// Ideal
				///transform->setTransform(rigidBody->getTransform());
				transform->setPosition(rigidBody->getPosition());
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
		UnregisterTypes();
		delete Factory::sInstance;
		Factory::sInstance = nullptr;
	}
}
