#pragma once

#include <glm/glm.hpp>

namespace SE
{
	// @brief Represents a transform component that can be attached to an entity
    class Transform
    {
    public:
        Transform() = default;

		void setPosition(const glm::vec3& position) { m_position = position; m_isDirty = true; }
		void setRotation(const glm::vec3& rotation) { m_rotation = rotation; m_isDirty = true; }
		void setScale(const glm::vec3& scale) { m_scale = scale; m_isDirty = true; }

		const glm::vec3& getPosition() const { return m_position; }
		const glm::vec3& getRotation() const { return m_rotation; }
		const glm::vec3& getScale() const { return m_scale; }

		const glm::mat4& getTransform();
    private:
		bool m_scaleLink{ true };

		bool m_isDirty{ true };
		glm::mat4 m_transform{ 1.0f };
		glm::vec3 m_position{ 0.0f, 0.0f, 0.0f };
		glm::vec3 m_rotation{ 0.0f, 0.0f, 0.0f };
		glm::vec3 m_scale{ 1.0f, 1.0f, 1.0f };
    };
}

