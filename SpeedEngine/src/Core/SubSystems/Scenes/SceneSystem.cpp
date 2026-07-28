#include "pch.h"
#include "SceneSystem.h"

#include "../Events/EventSystem.h"

namespace SE
{

	SceneSystem* SceneSystem::s_instance = nullptr;

	void SceneSystem::init()
	{
		SubSystem::init();
		s_instance = this;
	}

	void SceneSystem::update(double deltaTime)
	{
		SubSystem::update(deltaTime);

		if (!m_currentScene && !m_scenes.empty())
		{
			m_currentScene = m_scenes.front().get();
			m_currentScene->initializeScene();
		}

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

	void SceneSystem::addScene(std::unique_ptr<Scene> scene)
	{
		m_scenes.push_back(std::move(scene));
	}

	void SceneSystem::newScene(const std::string& name)
	{
		m_logger->info("Creating new scene: {}", name);
		auto newScene = std::make_unique<Scene>(name, *m_logger);
		m_scenes.push_back(std::move(newScene));
	}

	void SceneSystem::unloadScene(Scene* scene)
	{
		m_logger->info("Unloading scene: {}", scene->getName());
		if (scene == m_currentScene)
		{
			scene->shutdownScene();
			m_currentScene = nullptr;
		}

		m_scenes.erase(
			std::remove_if(m_scenes.begin(), m_scenes.end(),
				[scene](const std::unique_ptr<Scene>& s) {
					return s.get() == scene;
				}),
			m_scenes.end()
		);
	}

	void SceneSystem::setCurrentScene(const std::string& name)
	{
		m_logger->info("Setting current scene: {}", name);

		if (m_currentScene)
		{
			m_currentScene->shutdownScene();
		}

		auto it = std::find_if(m_scenes.begin(), m_scenes.end(),
			[&name](const std::unique_ptr<Scene>& scene) {
				return scene->getName() == name;
			});
		if (it != m_scenes.end())
		{
			m_currentScene = it->get();
			m_currentScene->initializeScene();

			EventSystem::Instance().publish(std::make_unique<SceneChanged>(name));
		}
		else
		{
			m_logger->warn("Scene '{}' not found. Current scene not changed.", name);
		}
	}
}