#include "pch.h"
#include "Transform.h"
#define GLM_ENABLE_EXPERIMENTAL

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"
#include "imgui.h"

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

	bool Transform::renderImGuiInterface()
	{
		if (ImGui::CollapsingHeader("Transform"))
		{
			ImGui::DragFloat3("Position", &m_position[0], 0.1f);
			glm::vec3 rotationDegrees = glm::degrees(m_rotation);
			ImGui::DragFloat3("Rotation", &rotationDegrees[0], 0.1f);
			m_rotation = glm::radians(rotationDegrees);

			// find ratio to change the corresponding scale component when linked
			auto linkScale = [&](int index, float oldValue)
				{
					if (!m_scaleLink)
						return;

					if (oldValue != 0.0f)
					{
						const float ratio = m_scale[index] / oldValue;
						for (int i = 0; i < 3; ++i)
						{
							if (i != index)
								m_scale[i] *= ratio;
						}
					}
					else
					{
						for (int i = 0; i < 3; ++i)
						{
							if (i != index)
								m_scale[i] = m_scale[index];
						}
					}
				};

			float oldx = m_scale[0];
			if (ImGui::DragFloat("x", &m_scale[0], 0.1f, 0.001f)) 
				linkScale(0, oldx);

			float oldy = m_scale[1];
			if (ImGui::DragFloat("y", &m_scale[1], 0.1f, 0.001f)) 
				linkScale(1, oldy);
			
			float oldz = m_scale[2];
			if (ImGui::DragFloat("z", &m_scale[2], 0.1f, 0.001f)) 
				linkScale(2, oldz);
			

			ImGui::Checkbox("Link Scale", &m_scaleLink);
			m_isDirty = true;
		}
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