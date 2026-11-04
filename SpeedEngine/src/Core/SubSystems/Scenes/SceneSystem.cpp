#include "pch.h"

#include "SceneSystem.h"

#include "SubSystems/Events/EventSystem.h"
#include "Engine.h"

namespace SE
{
	void SceneSystem::init()
	{
		SubSystem::init();
	}

	void SceneSystem::update(double deltaTime)
	{
		SubSystem::update(deltaTime);

		if (m_currentScene)
		{
			m_currentScene->updateScene(deltaTime);
		}
	}

	void SceneSystem::shutdown()
	{
		SubSystem::shutdown();
		m_pendingTransition.reset();

		if (m_currentScene)
			unloadScene(m_currentScene.get());
	}

	void SceneSystem::requestNewScene(const std::string& name)
	{
		if (!m_pendingTransition)
			m_pendingTransition = TransitionRequest{ TransitionType::New, name };
	}

	void SceneSystem::requestOpenScene(const std::string& path)
	{
		if (!m_pendingTransition)
			m_pendingTransition = TransitionRequest{ TransitionType::Open, path };
	}

	void SceneSystem::requestReloadScene()
	{
		if (!m_pendingTransition)
			m_pendingTransition = TransitionRequest{ TransitionType::Reload, {} };
	}

	void SceneSystem::processPendingTransition()
	{
		if (!m_pendingTransition)
			return;

		const auto request = std::move(*m_pendingTransition);
		// Consume first so callbacks can queue a request for the next update
		m_pendingTransition.reset();

		switch (request.type)
		{
		case TransitionType::New:
			newScene(request.value);
			break;
		case TransitionType::Open:
			setCurrentScene(request.value);
			break;
		case TransitionType::Reload:
			if (m_currentScene && !m_currentScene->getSourcePath().empty())
				m_currentScene->loadScene(m_currentScene->getSourcePath());
			else
				m_logger->warn("Cannot reload a scene without a saved source path.");
			break;
		}
	}

	void SceneSystem::newScene(const std::string& name)
	{
		m_logger->info("Creating new scene: {}", name);
		if (m_currentScene)
			m_currentScene->shutdownScene();
		m_currentScene = std::make_unique<Scene>(name, *m_logger);
		m_currentScene->initializeScene();
		sceneContentsReplaced(m_currentScene.get());
	}

	void SceneSystem::unloadScene(Scene* scene)
	{
		if (!scene || scene != m_currentScene.get())
		{
			m_logger->warn("Attempted to unload a scene that is not active.");
			return;
		}

		m_logger->info("Unloading scene: {}", scene->getName());
		scene->shutdownScene();
		m_currentScene.reset();
		++m_sceneRevision;
	}

	void SceneSystem::setCurrentScene(const std::string& path)
	{
		if (!m_currentScene)
			newScene(std::filesystem::path(path).stem().string());
		m_currentScene->loadScene(path);
	}

	void SceneSystem::sceneContentsReplaced(const Scene* scene)
	{
		if (scene == m_currentScene.get())
		{
			++m_sceneRevision;
			Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<SceneChanged>(scene->getName()));
		}
	}
} // namespace SE
