#pragma once

#include <glm/glm.hpp>

namespace SE
{
	// @brief Represents a RigidBody physics component that can be attached to an entity
	class RigidBody
	{
	public:
		RigidBody() = default;

	private:
		glm::vec3 m_offset{ 0.0f, 0.0f, 0.0f };

	};
}
