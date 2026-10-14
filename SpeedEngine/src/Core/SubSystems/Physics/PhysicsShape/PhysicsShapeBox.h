#pragma once

#include "PhysicsShape.h"

namespace SE
{
	class PhysicsShapeBox : public PhysicsShape
	{
	public:
		PhysicsShapeBox() {};
		PhysicsShapeBox(glm::vec3 halfExtents)
		    : m_halfExtents(halfExtents) {};
		~PhysicsShapeBox() = default;

		glm::vec3 getHalfExtents() const;
		void setHalfExtents(const glm::vec3& halfExtents);

	private:
		void createShape() override;

		glm::vec3 m_halfExtents{ 1.0f, 1.0f, 1.0f };
	};
}
