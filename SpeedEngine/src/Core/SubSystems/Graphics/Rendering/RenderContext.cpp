#include "pch.h"
#include "RenderContext.h"

#include "glm/gtc/matrix_transform.hpp"

namespace SE
{
    RenderContext::RenderContext()
    {
        resetStats();
    }

	RenderContext::~RenderContext()
	{
	}

	void RenderContext::setCameraFrustum(const CameraFrustum& cameraFrustum)
	{
		m_cameraFrustum = cameraFrustum;
		m_cameraFrustum.viewMatrix = glm::lookAt(m_cameraFrustum.position, m_cameraFrustum.target, m_cameraFrustum.up);
		m_cameraFrustum.projectionMatrix = glm::perspective(glm::radians(m_cameraFrustum.fov), m_viewport.aspectRatio(), m_cameraFrustum.nearPlane, m_cameraFrustum.farPlane);
		m_cameraFrustum.viewProjectionMatrix = m_cameraFrustum.projectionMatrix * m_cameraFrustum.viewMatrix;
	}

	void RenderContext::resetStats()
	{
		m_stats.trianglesDrawn = 0;
		m_stats.drawCalls = 0;
		m_stats.batchesSubmitted = 0;
		m_stats.frameTimeMs = 0.0;
	}

	void RenderContext::recordDrawCall(uint32_t triangles)
	{
		m_stats.drawCalls++;
		m_stats.trianglesDrawn += triangles;
	}
	void RenderContext::recordBatchSubmission()
	{
		m_stats.batchesSubmitted++;
	}
}