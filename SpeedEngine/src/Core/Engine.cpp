#include "pch.h"
#include "Engine.h"
#include "SubSystems/SubSystem.h"

#include "SubSystems/Graphics/Window.h"

#include "SubSystems/Events/EventSystem.h"
#include "SubSystems/Events/EventSubscription.h"

#include "ResourceManagers/ResourceManager.h"

#include "SubSystems/Graphics/Rendering/RendererManager.h"

#include "SubSystems/Scenes/SceneSystem.h"

// temp
#include "ResourceManagers/Shader/ShaderManager.h"
#include "ResourceManagers/Material/Material.h"

namespace SE
{
	Engine::Engine()
		: m_isRunning(false), m_deltaTime(0.0), m_currentTime(0.0), m_lastTime(0.0), m_windowCloseEvent()
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
		m_subSystems.push_back(std::move(rendermgr));
		m_subSystems.push_back(std::make_unique<SceneSystem>());
		
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
		SceneSystem::Instance().newScene("TestScene");
		SceneSystem::Instance().setCurrentScene("TestScene");
		SceneSystem::Instance().getCurrentScene()->addEntity(std::make_unique<Entity>("TestEntity"));
		SceneSystem::Instance().getCurrentScene()->addEntity(std::make_unique<Entity>("TestEntity1"));
		SceneSystem::Instance().getCurrentScene()->addEntity(std::make_unique<Entity>("TestEntity2"));
		SceneSystem::Instance().getCurrentScene()->addEntity(std::make_unique<Entity>("TestEntity3"));
		auto entityList = SceneSystem::Instance().getCurrentScene()->getEntityList();
		SceneSystem::Instance().getCurrentScene()->removeEntity(entityList.begin()->first);

		uint32_t id = ResourceManager::Instance().addResource<ResourceType::Shader>("Assets/Shaders/default.vert", "Assets/Shaders/default.frag", "defaultShader");
		uint32_t id2 = ResourceManager::Instance().addResource<ResourceType::Shader>("Assets/Shaders/default.vert", "Assets/Shaders/default.frag", "defaultShader");
		uint32_t id3 = ResourceManager::Instance().addResource<ResourceType::Shader>("Assets/Shaders/default.vert", "Assets/Shaders/default.frag", "defaultShader3");
		uint32_t id4 = ResourceManager::Instance().addResource<ResourceType::Shader>("Assets/Shaders/default.vert", "Assets/Shaders/default.frag", "defaultShader4");
		uint32_t id5 = ResourceManager::Instance().addResource<ResourceType::Shader>("Assets/Shaders/default.vert", "Assets/Shaders/default.frag", "defaultShader5");
		uint32_t id6 = ResourceManager::Instance().addResource<ResourceType::Shader>("Assets/Shaders/default.vert", "Assets/Shaders/default.frag", "defaultShader6");
		std::vector<std::pair<TextureType, std::string_view>> texlist = { {TextureType::Albedo,"Assets/Textures/cole-foxy.jpg"} };
		uint32_t matid = ResourceManager::Instance().addResource<ResourceType::Material>(id, texlist, "testMaterial");
		//ResourceManager::Instance().removeResource(matid);
		
		// get subsystems used in loop
		auto* window = _getSubSystem<Window>();
		auto* rendererManager = _getSubSystem<RendererManager>();
		
		// init time
		double lastTime = window->getCurrentTime();
		int frameCounter = 0;
		double fpsTimer = 0.0;
		double currentFPS = 0.0;
		const double targetFrameTime = 1.0 / window->getFPS();
		while (m_isRunning)
		{
			m_currentTime = window->getCurrentTime();
			m_deltaTime = m_currentTime - m_lastTime;
			m_lastTime = m_currentTime;

			for (auto& subSystem : m_subSystems)
			{
				subSystem->update(m_deltaTime);
			}

			// render
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