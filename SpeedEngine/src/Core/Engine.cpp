#include "pch.h"

#include "Engine.h"

#include "Features/MemoryTrackingSystem.h"
#include "ResourceManagers/ResourceManager.h"
#include "SubSystems/Events/EventSubscription.h"
#include "SubSystems/Events/EventSystem.h"
#include "SubSystems/Graphics/Rendering/DebugRenderer/DebugRenderer.h"
#include "SubSystems/Graphics/Rendering/RendererManager.h"
#include "SubSystems/Graphics/Rendering/SceneRenderer/SceneRenderer.h"
#include "SubSystems/Graphics/Window.h"
#include "SubSystems/Input/InputSystem.h"
#include "SubSystems/Scenes/SceneSystem.h"
#include "SubSystems/Physics/PhysicsSystem.h"
#include "SubSystems/Audio/AudioSystem.h"
#include "SubSystems/SubSystem.h"

// temp
#include <SubSystems/Scriptings/ScriptingSystem.h>

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
		m_subSystems.push_back(std::unique_ptr<Window>(new Window(1280, 720, "Speed Engine"))); // has to be BEFORE ResourceManager
		m_subSystems.push_back(std::unique_ptr<InputSystem>(new InputSystem()));
		m_subSystems.push_back(std::unique_ptr<ResourceManager>(new ResourceManager()));
		m_subSystems.push_back(std::unique_ptr<RendererManager>(new RendererManager(*getSubSystem<Window>())));
		m_subSystems.push_back(std::unique_ptr<PhysicsSystem>(new PhysicsSystem()));
		m_subSystems.push_back(std::unique_ptr<SceneSystem>(new SceneSystem()));
		m_subSystems.push_back(std::unique_ptr<ScriptingSystem>(new ScriptingSystem()));
		m_subSystems.push_back(std::unique_ptr<AudioSystem>(new AudioSystem()));

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
		SceneSystem* sceneSystem = getSubSystem<SceneSystem>();
		sceneSystem->newScene("TestScene");
		sceneSystem->setCurrentScene("TestScene");
		Scene* currentScene = sceneSystem->getCurrentScene();
		currentScene->loadScene("Assets/Data/default-scene.json");

		auto* sceneRenderer = getSubSystem<RendererManager>()->getRenderer<SceneRenderer>();
		
		ResourceManager* resourceManager = getSubSystem<ResourceManager>();
		uint32_t defaultShaderId = resourceManager->addResource<ResourceType::Shader>("Assets/Shaders/default.vert", "Assets/Shaders/default.frag", "defaultShader");
		sceneRenderer->setDefaultShaderId(defaultShaderId);
		
		const std::pair<TextureType, std::string_view> defaultTextures[] = {
			{ TextureType::Albedo, "Assets/Textures/default-texture-128.png" }
		};
		uint32_t defaultMaterialId = resourceManager->addResource<ResourceType::Material>(defaultShaderId, defaultTextures, "defaultMaterial");
		sceneRenderer->setDefaultMaterialId(defaultMaterialId);
	}
	void Engine::run()
	{
		

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

		auto* debugRenderer = rendererManager->getRenderer<DebugRenderer>(); // temp

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

			// temp
			debugRenderer->drawLine(glm::vec3(-2.0f, 1.0f, 0.0f), glm::vec3(2.0f, 1.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

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
