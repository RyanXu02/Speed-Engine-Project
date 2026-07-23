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
		m_subSystems.push_back(std::make_unique<Window>(1280, 720, "Speed Engine")); //has to be BEFORE ResourceManager
		m_subSystems.push_back(std::make_unique<ResourceManager>());

		//create RendererManager
		std::unique_ptr<RendererManager> rendermgr = std::make_unique<RendererManager>(*_getSubSystem<Window>());
		RendererManager::InitInstance(rendermgr.get());
		//m_subSystems.push_back(std::make_unique<RendererManager>(*_getSubSystem<Window>())); // has to be after Window
		m_subSystems.push_back(std::move(rendermgr));

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
		uint32_t id2 = ResourceManager::Instance().addResource<ResourceType::Shader>("Assets/Shaders/default.vert", "Assets/Shaders/default.frag", "defaultShader");
		uint32_t id3 = ResourceManager::Instance().addResource<ResourceType::Shader>("Assets/Shaders/default.vert", "Assets/Shaders/default.frag", "defaultShader3");
		uint32_t id4 = ResourceManager::Instance().addResource<ResourceType::Shader>("Assets/Shaders/default.vert", "Assets/Shaders/default.frag", "defaultShader4");
		uint32_t id5 = ResourceManager::Instance().addResource<ResourceType::Shader>("Assets/Shaders/default.vert", "Assets/Shaders/default.frag", "defaultShader5");
		uint32_t id6 = ResourceManager::Instance().addResource<ResourceType::Shader>("Assets/Shaders/default.vert", "Assets/Shaders/default.frag", "defaultShader6");
		std::vector<std::pair<TextureType, std::string_view>> texlist = { {TextureType::Albedo,"Assets/Textures/cole-foxy.jpg"} };
		uint32_t matid = ResourceManager::Instance().addResource<ResourceType::Material>(id, texlist, "testMaterial");
		//ResourceManager::Instance().removeResource(matid);
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