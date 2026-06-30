#include "pch.h"
#include "Engine.h"
#include "SubSystems/SubSystem.h"

#include "ResourceManagers/ResourceManager.h"
#include "ResourceManagers/Shader/ShaderManager.h"

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