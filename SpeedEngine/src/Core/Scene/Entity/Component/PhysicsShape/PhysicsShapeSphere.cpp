#include "pch.h"
#include "PhysicsShapeSphere.h"

#include <Jolt/Physics/Collision/Shape/SphereShape.h>

namespace SE
{
	float PhysicsShapeSphere::getRadius() const
	{
		return m_radius;
	}

	void PhysicsShapeSphere::setRadius(float radius)
	{
		m_radius = radius;
		updateShape();
	}

	void PhysicsShapeSphere::createUnscaledShape()
	{
		m_unscaledShape = new JPH::SphereShape(m_radius);
	}
}
