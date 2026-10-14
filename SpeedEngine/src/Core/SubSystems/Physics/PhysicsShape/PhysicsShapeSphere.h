#pragma once

#include "PhysicsShape.h"

namespace SE
{
	class PhysicsShapeSphere : public PhysicsShape
	{
	public:
		PhysicsShapeSphere() {};
		PhysicsShapeSphere(float radius)
		    : m_radius(radius) {};
		~PhysicsShapeSphere() = default;

		float getRadius() const;
		void setRadius(float radius);

	private:
		void createShape() override;

		float m_radius{ 1.0f };
	};
}
