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
		m_sceneRenderer = std::make_unique<SceneRenderer>(*m_logger);
		m_sceneRenderer->init();
		m_imguiRenderer = std::make_unique<ImGuiRenderer>(*m_logger, *m_window);
		m_imguiRenderer->init();
		//...

		// Initialize OpenGL states
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);

		createViewport(m_window->getWidth(), m_window->getHeight());
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


		for (const auto& [id, viewport] : m_viewports)
		{
			if (!viewport) continue;
			
			m_sceneRenderer->render(*viewport);
			m_imguiRenderer->render(*viewport);
			// ...
		}
		

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

		for (auto& [id, viewport] : m_viewports)
		{
			viewport.reset();
		}
		m_viewports.clear();
	}

	uint32_t RendererManager::createViewport(uint32_t width, uint32_t height)
	{
		static uint32_t newID = 1;
		uint32_t id = newID++;
		m_viewports[id] = std::make_unique<Viewport>(width, height);
		m_logger->info("Created viewport with ID {} ({}x{})", id, width, height);
		return id;
	}
	void RendererManager::destroyViewport(uint32_t viewportId)
	{
		auto it = m_viewports.find(viewportId);
		if (it != m_viewports.end())
		{
			it->second.reset();
			m_viewports.erase(it);
		}
		else
		{
			m_logger->warn("Viewport with ID {} not found for destruction.", viewportId);
		}
	}
	Viewport* RendererManager::getViewport(uint32_t viewportId)
	{
		auto it = m_viewports.find(viewportId);
		if (it != m_viewports.end())
		{
			return it->second.get();
		}
		else
		{
			m_logger->warn("Viewport with ID {} not found.", viewportId);
			return nullptr;
		}
	}
}