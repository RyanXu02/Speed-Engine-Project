#pragma once

#include "Core/SubSystems/Physics/PhysicsShape/PhysicsShape.h"

#include <glm/glm.hpp>

namespace SE
{
	// @brief Represents a RigidBody physics component that can be attached to an entity
	class RigidBody
	{
	public:
		RigidBody() = default;
		~RigidBody() = default;

		uint32_t getBodyID() const { return m_bodyID; }
		PhysicsShape* getShape() const { return m_shape; }
		void setShape(PhysicsShape* shape);

	private:
		glm::vec3 m_offset{ 0.0f, 0.0f, 0.0f };
		uint32_t m_bodyID{ 0 };
		PhysicsShape* m_shape{ nullptr };
	};
}
