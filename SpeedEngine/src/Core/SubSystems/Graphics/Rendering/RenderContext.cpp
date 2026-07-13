#include "pch.h"
#include "RenderContext.h"

#include "glm/gtc/matrix_transform.hpp"

namespace SE
{
    RenderContext::RenderContext(const Window* window)
        : m_window(window)
    {
        resetStats();
    }

	RenderContext::~RenderContext()
	{
	}

	void RenderContext::setCamera(const Camera& camera)
	{
		m_camera = camera;
		m_camera.viewMatrix = glm::lookAt(m_camera.position, m_camera.target, m_camera.up);
		m_camera.projectionMatrix = glm::perspective(glm::radians(m_camera.fov), m_viewport.aspectRatio(), m_camera.nearPlane, m_camera.farPlane);
		m_camera.viewProjectionMatrix = m_camera.projectionMatrix * m_camera.viewMatrix;
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