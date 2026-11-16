#pragma once

#include "Core/features/Serializable.h"
#include "Core/Features/MathToJSONBindings.h"

#include <glm/glm.hpp>

#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/PhysicsSettings.h>

inline JPH::Vec3 toJPH(const glm::vec3& vec)
{
	return JPH::Vec3(vec.x, vec.y, vec.z);
}

namespace SE
{
	//@brief abstract class representing jolt shapes
	class PhysicsShape
	{
	public:
		PhysicsShape() = default;
		virtual ~PhysicsShape() = default;

		virtual void createShape() 
		{
			if (!m_unscaledShape)
				createUnscaledShape();

			JPH::ScaledShapeSettings shapeSettings(m_unscaledShape, toJPH(m_scale));
			JPH::Shape::ShapeResult result; // can be used for error checking
			m_shape = new JPH::ScaledShape(shapeSettings, result);
		}

		virtual JPH::Ref<JPH::Shape> getShape()
		{
			if (!m_shape)
				createShape();

			return m_shape;
		};

		virtual void updateShape()
		{
			m_shape = nullptr;
			m_unscaledShape = nullptr;
			createShape();
		};

		virtual void setScale(glm::vec3 scale)
		{
			if (scale == m_scale)
				return;
			m_scale = scale;
			m_shape = nullptr;
			createShape();
		}

		virtual glm::vec3 getScale() const { return m_scale; }

	protected:
		JPH::Ref<JPH::Shape> m_shape{ nullptr };
		JPH::Ref<JPH::Shape> m_unscaledShape{ nullptr };
		glm::vec3 m_scale{ 1.0f };

	private:
		virtual void createUnscaledShape() = 0;
	};
}
