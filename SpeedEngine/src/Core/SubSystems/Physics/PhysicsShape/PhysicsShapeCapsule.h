#pragma once
#include "PhysicsShape.h"

namespace SE
{
	class PhysicsShapeCapsule : public PhysicsShape
	{
	public:
		PhysicsShapeCapsule() {};
		PhysicsShapeCapsule(float halfHeight, float radius)
		    : m_halfHeight(halfHeight),
		      m_radius(radius) {};
		~PhysicsShapeCapsule() = default;

		float getHalfHeight() const;
		void setHalfHeight(float halfHeight);
		float getRadius() const;
		void setRadius(float radius);

	private:
		void createShape() override;

		float m_halfHeight{ 1.0f };
		float m_radius{ 1.0f };
	};
}
