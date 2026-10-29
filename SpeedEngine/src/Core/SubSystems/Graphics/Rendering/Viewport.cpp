#include "pch.h"

#include "Viewport.h"

#include <glm/gtc/matrix_transform.hpp>

namespace SE
{
	Viewport::Viewport(uint32_t width, uint32_t height)
		: m_width(width),
		  m_height(height),
	      m_isActive(true)
	{

	}

	void Viewport::setCamera(const CameraState& camera)
	{
		m_cameraFrustum.setPosition(camera.position);
		m_cameraFrustum.setTarget(camera.target);
		m_cameraFrustum.setFOV(camera.fov);
		m_cameraFrustum.setNearPlane(camera.nearPlane);
		m_cameraFrustum.setFarPlane(camera.farPlane);
	}

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
		{
			return;
		}

		m_cameraFrustum.viewMatrix = glm::lookAt(m_cameraFrustum.getPosition(), m_cameraFrustum.getTarget(), WORLD_UP);
		float aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);
		m_cameraFrustum.projectionMatrix = glm::perspective(glm::radians(m_cameraFrustum.getFOV()), aspectRatio, m_cameraFrustum.getNearPlane(), m_cameraFrustum.getFarPlane());
		m_cameraFrustum.viewProjectionMatrix = m_cameraFrustum.projectionMatrix * m_cameraFrustum.viewMatrix;
		m_cameraFrustum.isDirty = false;
	}
} // namespace SE
