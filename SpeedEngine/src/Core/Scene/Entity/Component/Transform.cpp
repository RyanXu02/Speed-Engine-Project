#include "pch.h"
#include "Transform.h"
#define GLM_ENABLE_EXPERIMENTAL

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"


namespace SE
{
	Transform::Transform(const Transform* other)
		: Component(DISALLOW_MULTIPLE_COMPONENTS),
		m_position(other->m_position), m_rotation(other->m_rotation), m_scale(other->m_scale),
		m_isDirty(other->m_isDirty), m_transform(other->m_transform)
	{
	}

	bool Transform::initComponent()
	{
		return true;
	}
	bool Transform::updateComponent()
	{
		return true;
	}
	bool Transform::renderComponent()
	{
		return true;
	}
	bool Transform::shutdownComponent()
	{
		return true;
	}

	const glm::mat4& Transform::getTransform() const
	{
		if (m_isDirty)
		{
			glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), m_position);
			glm::mat4 rotationMatrix = glm::yawPitchRoll(m_rotation.y, m_rotation.x, m_rotation.z);
			glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), m_scale);
			m_transform = translationMatrix * rotationMatrix * scaleMatrix;
			m_isDirty = false;
		}
		return m_transform;
	}
}