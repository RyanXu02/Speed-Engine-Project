#include "pch.h"
#include "SceneSystem.h"

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

		for (auto& scene : m_scenes)
		{
			scene->shutdownScene();
		}

		m_scenes.clear();
		m_currentScene = nullptr;
	}
}