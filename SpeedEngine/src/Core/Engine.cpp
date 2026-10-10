#include "pch.h"

#include "Engine.h"

#include "Features/MemoryTrackingSystem.h"
#include "ResourceManagers/ResourceManager.h"
#include "SubSystems/Events/EventSubscription.h"
#include "SubSystems/Events/EventSystem.h"
#include "SubSystems/Graphics/Rendering/RendererManager.h"
#include "SubSystems/Graphics/Window.h"
#include "SubSystems/Input/InputSystem.h"
#include "SubSystems/Scenes/SceneSystem.h"
#include "SubSystems/Physics/PhysicsSystem.h"
#include "SubSystems/SubSystem.h"

// temp
#include "ResourceManagers/Material/Material.h"
#include "Scene/Entity/Component/ComponentStructs.h"
#include "Scene/Entity/Component/Transform.h"

namespace SE
{
	Engine& Engine::Instance()
	{
		static Engine instance;
		return instance;
	}

	Engine::Engine()
	    : m_isRunning(false),
	      m_deltaTime(0.0),
	      m_currentTime(0.0),
	      m_lastTime(0.0),
	      m_windowCloseEvent()
	{}
	Engine::~Engine() {}
	void Engine::init()
	{
		// push back subsystems in order of initialization
		m_subSystems.push_back(std::unique_ptr<MemoryTrackingSystem>(new MemoryTrackingSystem()));
		m_subSystems.push_back(std::unique_ptr<EventSystem>(new EventSystem()));
		m_subSystems.push_back(std::unique_ptr<Window>(new Window(1280, 720,
		    "Speed Engine"))); // has to be BEFORE ResourceManager
		m_subSystems.push_back(std::unique_ptr<InputSystem>(new InputSystem()));
		m_subSystems.push_back(std::unique_ptr<ResourceManager>(new ResourceManager()));
		m_subSystems.push_back(std::unique_ptr<RendererManager>(new RendererManager(*getSubSystem<Window>())));
		m_subSystems.push_back(std::unique_ptr<SceneSystem>(new SceneSystem()));
		m_subSystems.push_back(std::unique_ptr<PhysicsSystem>(new PhysicsSystem()));

		// init all subsystems
		for (auto& subSystem : m_subSystems)
		{
			subSystem->init();
		}

		// subscribe to window close event
		m_windowCloseEvent = getSubSystem<EventSystem>()->subscribe(EventType::WindowClose,
		    [this](const Event& event)
		    {
			    m_isRunning = false;
		    });
	}
	void Engine::start()
	{
		m_isRunning = true;
	}
	void Engine::run()
	{
		// temp
		SceneSystem* sceneSystem = getSubSystem<SceneSystem>();
		sceneSystem->newScene("TestScene");
		sceneSystem->setCurrentScene("TestScene");
		Scene* currentScene = sceneSystem->getCurrentScene();

		uint32_t defaultShaderIid = getSubSystem<ResourceManager>()->addResource<ResourceType::Shader>("Assets/Shaders/default.vert", "Assets/Shaders/default.frag", "defaultShader");
		std::vector<std::pair<TextureType, std::string_view>> texlist = { { TextureType::Albedo, "Assets/Textures/cole-foxy.jpg" } };
		uint32_t matid = getSubSystem<ResourceManager>()->addResource<ResourceType::Material>(defaultShaderIid, texlist, "testMaterial");

		//uint32_t bunnymeshId = getSubSystem<ResourceManager>()->addResource<ResourceType::MeshResource>("bunnymesh", "Assets/Meshes/bunny.obj");
		//uint32_t entityID = currentScene->addEntity("bunny");
		//Entity entity = currentScene->getEntity(entityID);
		//entity.addComponent<MeshComponent>(bunnymeshId);

		//uint32_t roommeshId2 = getSubSystem<ResourceManager>()->addResource<ResourceType::MeshResource>("conferencemesh", "Assets/Meshes/conference.obj");
		//uint32_t entityID2 = currentScene->addEntity("conference");
		//Entity entity2 = currentScene->getEntity(entityID2);
		//entity2.addComponent<MeshComponent>(roommeshId2);
		//entity2.getComponent<Transform>()->setScale(glm::vec3(0.01f));

		//uint32_t suzanemeshId3 = getSubSystem<ResourceManager>()->addResource<ResourceType::MeshResource>("suzanemesh", "Assets/Meshes/Suzanne.gltf");
		//uint32_t entityID3 = currentScene->addEntity("suzane");
		//Entity entity3 = currentScene->getEntity(entityID3);
		//entity3.addComponent<MeshComponent>(suzanemeshId3);
		// end temp


		// get subsystems used in loop
		auto* window = getSubSystem<Window>();
		auto* rendererManager = getSubSystem<RendererManager>();
		auto* physicsSystem = getSubSystem<PhysicsSystem>();

		// init time
		double lastTime = window->getCurrentTime();
		int frameCounter = 0;
		double fpsTimer = 0.0;
		double currentFPS = 0.0;
		const double targetFrameTime = 1.0 / window->getFPS();

		double accumulator = 0.0;
		const double physicsFrameTime = 1.0 / 60.0; // fixed 60 fps physics
		while (m_isRunning)
		{
			m_currentTime = window->getCurrentTime();
			m_deltaTime = m_currentTime - m_lastTime;
			// Cap dt
			if (m_deltaTime > 0.2)
				m_deltaTime = 0.2;
			m_lastTime = m_currentTime;

			for (auto& subSystem : m_subSystems)
			{
				subSystem->update(m_deltaTime);
			}

			accumulator += m_deltaTime;
			while (accumulator >= physicsFrameTime)
			{
				//physics step
				physicsSystem->step(physicsFrameTime);
				accumulator -= physicsFrameTime;
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
} // namespace SE
