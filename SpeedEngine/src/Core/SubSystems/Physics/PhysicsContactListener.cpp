#include "pch.h"
#include "PhysicsContactListener.h"

#include "Engine.h"

#include <glm/glm.hpp>

using namespace JPH;

inline glm::vec3 toGLM(const JPH::Vec3& vec)
{
	return glm::vec3(vec.GetX(), vec.GetY(), vec.GetZ());
}

namespace SE
{
	void PhysicsContactListener::OnContactAdded(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings) 
	{
		const std::lock_guard lock(m_mutex);
		Engine::Instance().getSubSystem<EventSystem>()->publish(
			std::make_unique<CollisionAddedEvent>(inBody1.GetID(), inBody2.GetID(), toGLM(inManifold.mWorldSpaceNormal), toGLM(inBody1.GetLinearVelocity()), toGLM(inBody2.GetLinearVelocity())));
	}

	void PhysicsContactListener::OnContactPersisted(const Body& inBody1, const Body& inBody2, const ContactManifold& inManifold, ContactSettings& ioSettings)
	{
		const std::lock_guard lock(m_mutex);
		Engine::Instance().getSubSystem<EventSystem>()->publish(
		    std::make_unique<CollisionPersistedEvent>(inBody1.GetID(), inBody2.GetID(), toGLM(inManifold.mWorldSpaceNormal)));
	}

	void PhysicsContactListener::OnContactRemoved(const SubShapeIDPair& inSubShapePair)
	{
		const std::lock_guard lock(m_mutex);
		BodyID inBody1Id = inSubShapePair.GetBody1ID();
		BodyID inBody2Id = inSubShapePair.GetBody2ID();
		Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<CollisionRemovedEvent>(inBody1Id, inBody2Id));
	}
}
