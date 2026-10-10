#pragma once
#include "Core/SubSystems/SubSystem.h"

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

using namespace JPH;

namespace SE
{
	class PhysicsSystem : public SubSystem
	{
	public:
		PhysicsSystem() : SubSystem("PhysicsSystem") {};

		void init() override;

		// @brief steps physics world one tick
		// @param deltaTime fixed time step
		void step(double deltaTime);

		void shutdown() override;

		// @brief Getter for BodyInterface, which is needed to interact with bodies in the physics world 
		// @returns Reference to the BodyInterface
		JPH::BodyInterface& getBodyInterface() { return physics_system.GetBodyInterface(); }

	private:
		TempAllocatorImpl* temp_allocator = nullptr;
		JobSystemThreadPool* job_system = nullptr;
		const uint cMaxBodies = 65536;
		const uint cNumBodyMutexes = 0;
		const uint cMaxBodyPairs = 65536;
		const uint cMaxContactConstraints = 10240;
		JPH::PhysicsSystem physics_system;
	};
}
