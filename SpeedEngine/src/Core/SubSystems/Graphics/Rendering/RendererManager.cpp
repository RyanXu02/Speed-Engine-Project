#include "pch.h"
#include "RendererManager.h"
#include "RenderContext.h"

#include "SceneRenderer/SceneRenderer.h"

namespace SE
{
	void RendererManager::addRenderer(std::unique_ptr<Renderer> renderer,  std::unique_ptr<RenderContext> context)
	{
		// if the type of renderer is already in the list, do not add it again
		auto type = std::type_index(typeid(*renderer));
		if (m_rendererMap.find(type) != m_rendererMap.end())
		{
			m_logger->info("Renderer {} already exists in RendererManager. Not adding again.", type.name());
			return;
		}

		if (!renderer->init())
		{
			m_logger->warn("Renderer {} failed to initialize. Not adding to RendererManager.", type.name());
			return;
		}
		m_renderers.push_back(std::move(renderer));
		m_rendererMap[type] = std::move(context);
	}

	void RendererManager::init()
	{
		SubSystem::init();

		auto sceneRenderContext = std::make_unique<RenderContext>();
		auto sceneRenderer = std::make_unique<SceneRenderer>(*sceneRenderContext, *m_logger);
		addRenderer(std::move(sceneRenderer), std::move(sceneRenderContext));
	}

	void RendererManager::update(double deltaTime)
	{
		SubSystem::update(deltaTime);
		for (auto& renderer : m_renderers)
		{
			renderer->update(deltaTime);
		}

		// potentially moved in the future
		for (auto& renderer : m_renderers)
		{
			renderer->beginFrame();

			auto type = std::type_index(typeid(*renderer));
			auto contextIt = m_rendererMap.find(type);
			if (contextIt != m_rendererMap.end())
			{
				renderer->render();
			}

			renderer->endFrame();
		}
	}

	void RendererManager::shutdown()
	{
		SubSystem::shutdown();
		for (auto& renderer : m_renderers)
		{
			renderer->shutdown();
		}
		m_renderers.clear();
		m_rendererMap.clear();
	}
}