#pragma once

#include "Math.h"
#include "CameraState.h"

namespace SE
{
	struct CameraFrustum
	{
	private:
		glm::vec3 position = glm::vec3(0.0f, 2.0f, 5.0f);
		glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);

		float fov = 90.0f;
		float nearPlane = 0.1f;
		float farPlane = 1000.0f;


	public:
		// @brief Move the camera by a given offset
		// @param delta The offset to move the camera by
		void move(const glm::vec3& delta)
		{
			position += delta;
			target += delta;
			isDirty = true;
		}

		const glm::vec3& getPosition() const { return position; }
		void setPosition(const glm::vec3& pos)
		{
			position = pos;
			isDirty = true;
		}
		const glm::vec3& getTarget() const { return target; }
		void setTarget(const glm::vec3& tgt)
		{
			target = tgt;
			isDirty = true;
		}

		const float getFOV() const { return fov; }
		void setFOV(float f)
		{
			fov = f;
			isDirty = true;
		}
		const float getNearPlane() const { return nearPlane; }
		void setNearPlane(float n)
		{
			nearPlane = n;
			isDirty = true;
		}
		const float getFarPlane() const { return farPlane; }
		void setFarPlane(float f)
		{
			farPlane = f;
			isDirty = true;
		}

		CameraState getState() const { return { position, target, fov, nearPlane, farPlane }; }

		// TODO: make these private, add getter for them
		bool isDirty = true;
		glm::mat4 viewMatrix = glm::mat4(1.0f);
		glm::mat4 projectionMatrix = glm::mat4(1.0f);
		glm::mat4 viewProjectionMatrix = glm::mat4(1.0f);
	};

}