#include "pch.h"
#include "Engine.h"
#include "SubSystems/SubSystem.h"

#include "SubSystems/Graphics/Window.h"

#include "ResourceManagers/ResourceManager.h"
#include "ResourceManagers/Shader/ShaderManager.h"

//temp include to test glfw window close
#include "GLFW/glfw3.h"
#include "SubSystems/Graphics/Window.h"

namespace SE
{
	Engine::Engine()
		: m_isRunning(false), m_deltaTime(0.0)
	{
	}
	Engine::~Engine()
	{
	}
	void Engine::init()
	{
		m_subSystems.push_back(std::make_unique<Window>(1280, 720, "Speed Engine"));
		m_subSystems.push_back(std::make_unique<ResourceManager>());

		for (auto& subSystem : m_subSystems)
		{
			subSystem->init();
		}
	}
	void Engine::start()
	{
		m_isRunning = true;
	}
	void Engine::run()
	{
		ResourceManager::Instance().addResource(ResourceType::Shader, "Assets/Shaders/default.vert", "Assets/Shaders/default.frag", "defaultShader");
		while (m_isRunning)
		{
			for (auto& subSystem : m_subSystems)
			{
				subSystem->update(m_deltaTime);
			}

			// temp test window close code
			Window* window = dynamic_cast<Window*>(m_subSystems[0].get());
			if (window && glfwWindowShouldClose(window->getWindow())) {
				m_isRunning = false;
			}
		}
	}
	void Engine::stop()
	{
		m_isRunning = false;
		for (auto& subSystem : m_subSystems)
		{
			subSystem->shutdown();
		}
	}
}