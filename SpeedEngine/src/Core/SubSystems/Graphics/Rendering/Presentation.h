#pragma once

#include "Core/Features/CameraState.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace SE
{
	struct TransformState
	{
		glm::vec3 position{ 0.0f };
		glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
		glm::vec3 scale{ 1.0f };
	};

	inline glm::mat4 interpolateTransform(const TransformState& previous, const TransformState& current, float t)
	{
		return glm::translate(glm::mat4(1.0f), glm::mix(previous.position, current.position, t))
			* glm::mat4_cast(glm::slerp(previous.rotation, current.rotation, t))
			* glm::scale(glm::mat4(1.0f), glm::mix(previous.scale, current.scale, t));
	}

	inline CameraState interpolateCamera(const CameraState& previous, const CameraState& current, float t)
	{
		CameraState camera;
		camera.position = glm::mix(previous.position, current.position, t);
		camera.target = glm::mix(previous.target, current.target, t);
		camera.fov = glm::mix(previous.fov, current.fov, t);
		camera.nearPlane = glm::mix(previous.nearPlane, current.nearPlane, t);
		camera.farPlane = glm::mix(previous.farPlane, current.farPlane, t);
		return camera;
	}
}
