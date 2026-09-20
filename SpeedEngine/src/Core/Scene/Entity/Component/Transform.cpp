#include "pch.h"

#include "Transform.h"
#define GLM_ENABLE_EXPERIMENTAL

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "imgui.h"

namespace SE
{


	//		glm::vec3 rotationDegrees = glm::degrees(m_rotation);
	//		m_isDirty = m_isDirty || ImGui::DragFloat3("Rotation", &rotationDegrees[0], 0.1f);
	//		m_rotation = glm::radians(rotationDegrees);


	//		ImGui::Checkbox("Link Scale", &m_scaleLink);
	//	}
	//	return true;
	//}

	const glm::mat4& Transform::getTransform()
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
} // namespace SE
