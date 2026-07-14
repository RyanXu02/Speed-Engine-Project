#include "pch.h"
#include "SceneRenderer.h"
#include <glad/gl.h>

#include "../RendererManager.h"



namespace SE
{
	bool SceneRenderer::init()
	{
		m_logger.info("Initializing");
		return true;
	}
	void SceneRenderer::shutdown()
	{
		m_logger.info("Shutting down");
	}
	void SceneRenderer::update(double deltaTime)
	{
		// Update scene logic here
	}
	void SceneRenderer::beginFrame()
	{
		m_logger.verbose("Beginning frame");

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glViewport(m_context->getViewport().x, m_context->getViewport().y, m_context->getViewport().width, m_context->getViewport().height);
	}
	void SceneRenderer::endFrame()
	{
		m_logger.verbose("Ending frame");
	}
	void SceneRenderer::render() const
	{
		// Perform rendering using the provided context
		// For example, set up camera matrices, draw objects, etc.
	}
}
