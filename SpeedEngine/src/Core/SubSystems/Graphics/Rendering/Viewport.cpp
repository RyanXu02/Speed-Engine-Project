#include "pch.h"

#include "Viewport.h"

#include <glm/gtc/matrix_transform.hpp>

namespace SE
{
	void Viewport::resize(uint32_t width, uint32_t height)
	{
		if (width != m_width || height != m_height)
		{
			m_width = width;
			m_height = height;
			m_cameraFrustum.isDirty = true;
		}
	}

	void Viewport::updateMatrices()
	{
		if (!m_cameraFrustum.isDirty)
			return;

		auto& camera = m_cameraFrustum;
		camera.viewMatrix = glm::lookAt(camera.getPosition(), camera.getTarget(), WORLD_UP);
		float aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
		camera.projectionMatrix = glm::perspective(glm::radians(camera.getFOV()), aspectRatio, camera.getNearPlane(), camera.getFarPlane());
		camera.viewProjectionMatrix = camera.projectionMatrix * camera.viewMatrix;
		camera.isDirty = false;
	}
} // namespace SE
