#include "pch.h"

#include "CameraController.h"
#include "CameraFrustum.h"
#include "Core/SubSystems/Input/InputSystem.h"

namespace SE
{
	void CameraController::update(CameraFrustum& camera, const InputSystem& input, double deltaTime)
	{
		const float speedMultiplier = input.isKeyDown(KeyCodes::KEY_LEFT_SHIFT) ? 2.0f : 1.0f;
		const float moveSpeed = m_cameraMoveSpeed * speedMultiplier * static_cast<float>(deltaTime);
		const float dollySpeed = m_cameraDollySpeed * speedMultiplier * static_cast<float>(deltaTime);
		glm::vec3 lookDir = glm::normalize(camera.getTarget() - camera.getPosition());
		glm::vec3 planarLookDir = lookDir;
		planarLookDir.y = 0.0f;
		glm::vec3 cameraRight = glm::normalize(glm::cross(planarLookDir, WORLD_UP));
		glm::vec3 moveDir(0.0f);

		if (input.isKeyDown(KeyCodes::KEY_W))
			moveDir += planarLookDir;
		if (input.isKeyDown(KeyCodes::KEY_S))
			moveDir -= planarLookDir;
		if (input.isKeyDown(KeyCodes::KEY_A))
			moveDir -= cameraRight;
		if (input.isKeyDown(KeyCodes::KEY_D))
			moveDir += cameraRight;
		if (glm::length(moveDir) > 0.0f)
			camera.move(glm::normalize(moveDir) * moveSpeed);

		if (input.getMouseScroll().yoffset > 0.0)
			camera.move(lookDir * dollySpeed);
		if (input.getMouseScroll().yoffset < 0.0)
			camera.move(-lookDir * dollySpeed);
		if (input.isKeyDown(KeyCodes::KEY_SPACE))
			camera.move(WORLD_UP * moveSpeed);
		if (input.isKeyDown(KeyCodes::KEY_LEFT_CONTROL))
			camera.move(-WORLD_UP * moveSpeed);

		if (input.isMouseDown(MouseCodes::MOUSE_BUTTON_3))
		{
			glm::vec2 mouseDelta{ input.getMouseDelta().x, input.getMouseDelta().y };
			glm::vec3 direction = glm::normalize(camera.getTarget() - camera.getPosition());
			cameraRight = glm::normalize(glm::cross(direction, WORLD_UP));
			glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, direction));

			float yawAngle = -mouseDelta.x * m_sensitivity;
			direction = glm::vec3(glm::rotate(glm::mat4(1.0f), yawAngle, cameraUp) * glm::vec4(direction, 0.0f));
			float pitchAngle = -mouseDelta.y * m_sensitivity;
			glm::vec3 newDirection = glm::vec3(glm::rotate(glm::mat4(1.0f), pitchAngle, cameraRight) * glm::vec4(direction, 0.0f));
			// Limit pitch to avoid gimbal lock.
			float upDot = glm::dot(newDirection, WORLD_UP);
			if (upDot < 0.999f && upDot > -0.999f)
				direction = newDirection;

			float distance = glm::length(camera.getTarget() - camera.getPosition());
			camera.setTarget(camera.getPosition() + direction * distance);
		}
	}
} // namespace SE
