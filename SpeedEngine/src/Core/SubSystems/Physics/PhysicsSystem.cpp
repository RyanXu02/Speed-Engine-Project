#include "pch.h"
#include "PhysicsSystem.h"

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
		// Allocation hook, can connect this to custom memory control if needed (see Jolt/Core/Memory.h)
		RegisterDefaultAllocator();

		Factory::sInstance = new Factory();
		RegisterTypes();
		m_temp_allocator = std::make_unique<TempAllocatorImpl>(10 * 1024 * 1024);
		m_job_system = std::make_unique<JobSystemThreadPool>(cMaxPhysicsJobs, cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

		BPLayerInterfaceImpl broad_phase_layer_interface;
		ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
		ObjectLayerPairFilterImpl obj_vs_object_layer_filter;

		m_physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, obj_vs_object_layer_filter);

		BodyInterface& body_interface = m_physics_system.GetBodyInterface();

	}

	void PhysicsSystem::step(double deltaTime) {
		const int cCollisionSteps = 1; // If stepping physics less that 60hz, do more collision steps per simulation step

		// Step physics simulation
		m_physics_system.Update(deltaTime, cCollisionSteps, m_temp_allocator.get(), m_job_system.get());
	}

	void PhysicsSystem::shutdown() {
		// Remove and Destroy all bodies
		// ...
		
		// Shut down physics system
		UnregisterTypes();
		delete Factory::sInstance;
		Factory::sInstance = nullptr;
		delete m_temp_allocator.get();
		delete m_job_system.get();
	}
}
