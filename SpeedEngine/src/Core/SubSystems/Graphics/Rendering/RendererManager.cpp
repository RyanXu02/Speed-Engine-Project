#include "pch.h"

#include "RendererManager.h"

#include "SceneRenderer/SceneRenderer.h"

#include <glad/glad.h>

namespace SE
{
	void RendererManager::init()
	{
		SubSystem::init();

		// init all renderers
		auto sceneViewport = std::make_unique<Viewport>(m_window->getWidth(), m_window->getHeight());
		m_sceneRenderer = std::unique_ptr<SceneRenderer>(new SceneRenderer(*m_logger, std::move(sceneViewport)));
		m_sceneRenderer->init();
#if defined(COMPILE_WITH_EDITOR)
		// imgui renderer doesn't need a viewport, it makes its own
		m_imguiRenderer = std::unique_ptr<ImGuiRenderer>(new ImGuiRenderer(*m_logger, *m_window));
		m_imguiRenderer->init();
#endif
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
		if (m_sceneRenderer)
			m_sceneRenderer->update(deltaTime);
#if defined(COMPILE_WITH_EDITOR)
		if (m_imguiRenderer)
			m_imguiRenderer->update(deltaTime);
#endif
		// ...
	}

	void RendererManager::render() const
	{
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_sceneRenderer->render();
		//....
#if defined(COMPILE_WITH_EDITOR)
		m_imguiRenderer->render();

		m_imguiRenderer->finalizeFrame();
#endif

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
#if defined(COMPILE_WITH_EDITOR)
		if (m_imguiRenderer)
		{
			m_imguiRenderer->shutdown();
			m_imguiRenderer.reset();
		}
#endif
		// ...
	}

} // namespace SE
