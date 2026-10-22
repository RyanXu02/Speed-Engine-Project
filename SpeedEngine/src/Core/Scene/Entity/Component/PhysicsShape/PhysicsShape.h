#pragma once

#include "Core/features/Serializable.h"

#include <glm/glm.hpp>

#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/PhysicsSettings.h>

namespace SE
{
	//@brief abstract class representing jolt shapes
	class PhysicsShape
	{
	public:
		PhysicsShape() = default;
		virtual ~PhysicsShape() = default;

		virtual JPH::Ref<JPH::Shape> getShape()
		{
			if (!m_shape)
				createShape();

			return m_shape;
		};

		virtual void updateShape()
		{
			m_shape = nullptr;
			createShape();
		};

	protected:
		JPH::Ref<JPH::Shape> m_shape{ nullptr };

	private:
		virtual void createShape() = 0;
	};
}
