#include "pch.h"

#include "Transform.h"
#define GLM_ENABLE_EXPERIMENTAL

#include "glm/gtc/matrix_transform.hpp"
#include "imgui.h"

namespace SE
{
	const glm::mat4& Transform::getTransform()
	{
		if (m_isDirty)
		{
			glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translate);
			glm::mat4 rotationMatrix = glm::mat4_cast(rotate);
			glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
			m_transform = translationMatrix * rotationMatrix * scaleMatrix;
			m_isDirty = false;
		}
		return m_transform;
	}
}
