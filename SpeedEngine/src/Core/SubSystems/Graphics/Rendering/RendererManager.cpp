#include "pch.h"
#include "RendererManager.h"

#include <glad/gl.h>

#include "SceneRenderer/SceneRenderer.h"

namespace SE
{
	void RendererManager::init()
	{
		SubSystem::init();

		// init all renderers
		auto sceneViewport = std::make_unique<Viewport>(m_window->getWidth(), m_window->getHeight());
		m_sceneRenderer = std::make_unique<SceneRenderer>(*m_logger, std::move(sceneViewport));
		m_sceneRenderer->init();

		// imgui renderer doesn't need a viewport, it makes its own
		m_imguiRenderer = std::make_unique<ImGuiRenderer>(*m_logger, *m_window);
		m_imguiRenderer->init();
		//...

		// Initialize OpenGL states
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
	}

	void RendererManager::update(double deltaTime)
	{
		SubSystem::update(deltaTime);
		if (m_sceneRenderer) m_sceneRenderer->update(deltaTime);
		if (m_imguiRenderer) m_imguiRenderer->update(deltaTime);
		// ...
	}

	void RendererManager::render() const
	{
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_sceneRenderer->render();
		m_imguiRenderer->render();
			// ...
		
		m_imguiRenderer->finalizeFrame();

		m_window->swapBuffers();
	}

	void RendererManager::shutdown()
	{
		SubSystem::shutdown();
		if (m_sceneRenderer)
		{
			m_sceneRenderer->shutdown();
			m_sceneRenderer.reset();
		}
		if (m_imguiRenderer)
		{
			m_imguiRenderer->shutdown();
			m_imguiRenderer.reset();
		}
		// ...
	}

}