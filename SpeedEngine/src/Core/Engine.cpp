#include "pch.h"
#include "Engine.h"
#include "SubSystems/SubSystem.h"

#include "SubSystems/Graphics/Window.h"

#include "SubSystems/Events/EventSystem.h"
#include "SubSystems/Events/EventSubscription.h"

#include "ResourceManagers/ResourceManager.h"
#include "ResourceManagers/Shader/ShaderManager.h"

#include "ResourceManagers/Material/Material.h"

namespace SE
{
	Engine::Engine()
		: m_isRunning(false), m_deltaTime(0.0), m_windowCloseEvent()
	{
	}
	Engine::~Engine()
	{
	}
	void Engine::init()
	{
		m_subSystems.push_back(std::make_unique<Window>(1280, 720, "Speed Engine"));
		m_subSystems.push_back(std::make_unique<EventSystem>());
		m_subSystems.push_back(std::make_unique<ResourceManager>());

		for (auto& subSystem : m_subSystems)
		{
			subSystem->init();
		}

		m_windowCloseEvent = EventSystem::Instance().subscribe(EventType::WindowClose, [this](const Event& event) {
			m_isRunning = false;
			});
	}
	void Engine::start()
	{
		m_isRunning = true;
	}
	void Engine::run()
	{
		ResourceManager::Instance().addResource<ResourceType::Shader>("Assets/Shaders/default.vert", "Assets/Shaders/default.frag", "defaultShader");
		while (m_isRunning)
		{
			for (auto& subSystem : m_subSystems)
			{
				subSystem->update(m_deltaTime);
			}
		}
	}
	void Engine::stop()
	{
		for (auto& subSystem : m_subSystems)
		{
			subSystem->shutdown();
		}
	}
}