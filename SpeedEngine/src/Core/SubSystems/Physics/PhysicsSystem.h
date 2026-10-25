#pragma once
#include "Core/SubSystems/SubSystem.h"

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

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
	class PhysicsSystem : public SubSystem
	{
		friend class Engine;

	public:
		void init() override;

		// @brief initializes the RigidBody components, creating them as Jolt objects and adding them to the physics world.
		//  Also sets the initial transform of the RigidBody to that of the connected Transform component.
		void createAndAddComponents();

		// @brief steps physics world one tick
		// @param deltaTime fixed time step
		void step(double deltaTime);

		void shutdown() override;

		void pause() { m_running = false; }
		void resume() { m_running = true; }

		// @brief Getter for BodyInterface, which is needed to interact with bodies in the physics world 
		// @returns Reference to the BodyInterface
		JPH::BodyInterface* getBodyInterface() { return &m_physics_system->GetBodyInterface(); }

	private:
		PhysicsSystem()
		    : SubSystem("PhysicsSystem") {};

		std::unique_ptr<TempAllocatorImpl> m_temp_allocator;
		std::unique_ptr<JobSystemThreadPool> m_job_system;
		std::unique_ptr<JPH::PhysicsSystem> m_physics_system;
		BPLayerInterfaceImpl m_broad_phase_layer_interface;
		ObjectVsBroadPhaseLayerFilterImpl m_object_vs_broadphase_layer_filter;
		ObjectLayerPairFilterImpl m_obj_vs_object_layer_filter;
		bool m_running = false;
		const uint cMaxBodies = 65536;
		const uint cNumBodyMutexes = 0;
		const uint cMaxBodyPairs = 65536;
		const uint cMaxContactConstraints = 10240;
	};
}
