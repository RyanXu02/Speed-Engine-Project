#pragma once

#include <glm/glm.hpp>

namespace SE
{
	struct CameraState
	{
		glm::vec3 position{ 0.0f, 2.0f, 5.0f };
		glm::vec3 target{ 0.0f };
		float fov = 90.0f;
		float nearPlane = 0.1f;
		float farPlane = 1000.0f;
	};
} // namespace SE
