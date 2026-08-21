#include "pch.h"
#include "Engine.h"
#include "SubSystems/SubSystem.h"

#include "SubSystems/Graphics/Window.h"
#include "SubSystems/Input/InputSystem.h"

#include "SubSystems/Events/EventSystem.h"
#include "SubSystems/Events/EventSubscription.h"

#include "ResourceManagers/ResourceManager.h"

#include "SubSystems/Graphics/Rendering/RendererManager.h"

#include "SubSystems/Scenes/SceneSystem.h"

// temp
#include "ResourceManagers/Material/Material.h"
#include "Scene/Entity/Component/Transform.h"
#include "Scene/Entity/Component/Mesh.h"

namespace SE
{
	Engine& Engine::Instance()
	{
		static Engine instance;
		return instance;
	}

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
		m_subSystems.push_back(std::unique_ptr<EventSystem>(new EventSystem()));
		m_subSystems.push_back(std::unique_ptr<Window>(new Window(1280, 720, "Speed Engine"))); //has to be BEFORE ResourceManager
		m_subSystems.push_back(std::unique_ptr<InputSystem>(new InputSystem()));
		m_subSystems.push_back(std::unique_ptr<ResourceManager>(new ResourceManager()));
		m_subSystems.push_back(std::unique_ptr<RendererManager>(new RendererManager(*getSubSystem<Window>())));
		m_subSystems.push_back(std::unique_ptr<SceneSystem>(new SceneSystem()));
		
		// init all subsystems
		for (auto& subSystem : m_subSystems)
		{
			subSystem->init();
		}
		
		// subscribe to window close event
		m_windowCloseEvent = getSubSystem<EventSystem>()->subscribe(EventType::WindowClose, [this](const Event& event) {
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
		getSubSystem<SceneSystem>()->newScene("TestScene");
		getSubSystem<SceneSystem>()->setCurrentScene("TestScene");
		getSubSystem<SceneSystem>()->getCurrentScene()->addEntity(std::make_unique<Entity>("TestEntity"));
		getSubSystem<SceneSystem>()->getCurrentScene()->addEntity(std::make_unique<Entity>("bunny"));
		getSubSystem<SceneSystem>()->getCurrentScene()->addEntity(std::make_unique<Entity>("room"));
		getSubSystem<SceneSystem>()->getCurrentScene()->addEntity(std::make_unique<Entity>("TestEntity3"));


		uint32_t defaultShaderIid = getSubSystem<ResourceManager>()->addResource<ResourceType::Shader>("Assets/Shaders/default.vert", "Assets/Shaders/default.frag", "defaultShader");
		std::vector<std::pair<TextureType, std::string_view>> texlist = { {TextureType::Albedo,"Assets/Textures/cole-foxy.jpg"} };
		uint32_t matid = getSubSystem<ResourceManager>()->addResource<ResourceType::Material>(defaultShaderIid, texlist, "testMaterial");

		uint32_t bunnymeshId = getSubSystem<ResourceManager>()->addResource<ResourceType::MeshResource>("bunnymesh", "Assets/Meshes/bunny.obj");
		uint32_t roommeshId2 = getSubSystem<ResourceManager>()->addResource<ResourceType::MeshResource>("conferencemesh", "Assets/Meshes/conference.obj");
		
		auto entity1 = getSubSystem<SceneSystem>()->getCurrentScene()->getEntity(2);
		entity1->addComponent(std::make_unique<Mesh>());
		entity1->getComponent<Mesh>()->setMeshResourceId(bunnymeshId);
		entity1->getComponent<Transform>()->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));

		auto entity2 = getSubSystem<SceneSystem>()->getCurrentScene()->getEntity(3);
		entity2->addComponent(std::make_unique<Mesh>());
		entity2->getComponent<Mesh>()->setMeshResourceId(roommeshId2);
		entity2->getComponent<Transform>()->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		// end temp


		// get subsystems used in loop
		auto* window = getSubSystem<Window>();
		auto* rendererManager = getSubSystem<RendererManager>();
		
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