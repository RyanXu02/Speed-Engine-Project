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
		m_subSystems.push_back(std::unique_ptr<SceneSystem>(new SceneSystem()));
		m_subSystems.push_back(std::unique_ptr<ScriptingSystem>(new ScriptingSystem()));

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
		// get subsystems used in loop
		auto* window = getSubSystem<Window>();
		auto* rendererManager = getSubSystem<RendererManager>();

		auto* debugRenderer = rendererManager->getRenderer<DebugRenderer>(); // temp

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
