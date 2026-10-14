#include "pch.h"
#include "RigidBody.h"

namespace SE
{
	void RigidBody::setShape(PhysicsShape* shape) {
		m_shape = shape;
	}
}
