#include "pch.h"
#include "Engine.h"
#include "SubSystems/SubSystem.h"

#include "SubSystems/Graphics/Window.h"

#include "SubSystems/Events/EventSystem.h"
#include "SubSystems/Events/EventSubscription.h"

#include "ResourceManagers/ResourceManager.h"

#include "SubSystems/Graphics/Rendering/RendererManager.h"

// temp
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
		// push back subsystems in order of initialization
		m_subSystems.push_back(std::make_unique<EventSystem>());
		m_subSystems.push_back(std::make_unique<ResourceManager>());
		m_subSystems.push_back(std::make_unique<Window>(1280, 720, "Speed Engine")); //has to be after ResourceManager
		m_subSystems.push_back(std::make_unique<RendererManager>(*_getSubSystem<Window>())); // has to be after Window
		// init all subsystems
		for (auto& subSystem : m_subSystems)
		{
			subSystem->init();
		}
		
		// subscribe to window close event
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
		uint32_t id = ResourceManager::Instance().addResource<ResourceType::Shader>("Assets/Shaders/default.vert", "Assets/Shaders/default.frag", "defaultShader");
		std::vector<std::pair<TextureType, std::string_view>> texlist = { {TextureType::Albedo,"Assets/Textures/cole-foxy.jpg"} };
		ResourceManager::Instance().addResource<ResourceType::Material>(id, texlist, "testMaterial");
		while (m_isRunning)
		{
			for (auto& subSystem : m_subSystems)
			{
				subSystem->update(m_deltaTime);
			}

			// render
			auto* rendererManager = _getSubSystem<RendererManager>();
			rendererManager->render();
		}
	}
	void Engine::stop()
	{
		for (auto& subSystem : m_subSystems | std::views::reverse)
		{
			subSystem->shutdown();
		}
	}
}