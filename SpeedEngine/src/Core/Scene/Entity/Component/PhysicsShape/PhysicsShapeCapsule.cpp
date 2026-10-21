#include "pch.h"
#include "PhysicsShapeCapsule.h"

#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>

namespace SE
{
	float PhysicsShapeCapsule::getHalfHeight() const {
		return m_halfHeight;
	}

	void PhysicsShapeCapsule::setHalfHeight(float halfHeight) {
		m_halfHeight = halfHeight;
		updateShape();
	}

	float PhysicsShapeCapsule::getRadius() const {
		return m_radius;
	}

	void PhysicsShapeCapsule::setRadius(float radius) {
		m_radius = radius;
		updateShape();
	}

	void PhysicsShapeCapsule::createShape() {
		m_shape = new JPH::CapsuleShape(m_halfHeight, m_radius);
	}
}
