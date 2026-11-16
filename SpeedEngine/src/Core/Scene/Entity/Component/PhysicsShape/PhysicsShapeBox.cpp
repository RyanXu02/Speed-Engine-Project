#include "pch.h"
#include "PhysicsShapeBox.h"

#include <Jolt/Physics/Collision/Shape/BoxShape.h>

namespace SE
{
	glm::vec3 PhysicsShapeBox::getHalfExtents() const {
		return m_halfExtents;
	}

	void PhysicsShapeBox::setHalfExtents(const glm::vec3& halfExtents) {
		m_halfExtents = halfExtents;
		updateShape();
	}

	void PhysicsShapeBox::createUnscaledShape() {
		m_unscaledShape = new JPH::BoxShape(JPH::Vec3(m_halfExtents.x, m_halfExtents.y, m_halfExtents.z), JPH::cDefaultConvexRadius);
	}
}
