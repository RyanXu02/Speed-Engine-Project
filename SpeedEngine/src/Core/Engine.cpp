#include "pch.h"

#include "Engine.h"

#include "Features/MemoryTrackingSystem.h"
#include "ResourceManagers/ResourceManager.h"
#include "SubSystems/Events/EventSubscription.h"
#include "SubSystems/Events/EventSystem.h"
#include "SubSystems/Graphics/Rendering/RendererManager.h"
#include "SubSystems/Graphics/Rendering/RenderFrameBuilder.h"
#include "SubSystems/Graphics/Rendering/SceneRenderer/SceneRenderer.h"
#include "SubSystems/Graphics/Window.h"
#include "SubSystems/Input/InputSystem.h"
#include "SubSystems/Scenes/SceneSystem.h"
#include "SubSystems/Physics/PhysicsSystem.h"
#include "SubSystems/Audio/AudioSystem.h"
#include "SubSystems/UI/UISystem.h"
#include "SubSystems/SubSystem.h"
#include "EngineStates.h"
#include "Threading/FrameExchange.h"

// temp
#include <SubSystems/Scriptings/ScriptingSystem.h>

#include <Jolt/Core/Memory.h>

namespace SE
{
	inline constexpr double simulationDeltaTime = 1.0 / 60.0;

	Engine& Engine::Instance()
	{
		static Engine instance;
		return instance;
	}

	Engine::~Engine() { stopSimulation(); }

	void Engine::init()
	{
		// Allocation hook for Jolt, can connect this to custom memory control if needed (see Jolt/Core/Memory.h)
		JPH::RegisterDefaultAllocator();

		// push back subsystems in order of initialization
		m_subSystems.push_back(std::unique_ptr<MemoryTrackingSystem>(new MemoryTrackingSystem()));
		m_subSystems.push_back(std::unique_ptr<EventSystem>(new EventSystem()));
		m_subSystems.push_back(std::unique_ptr<Window>(new Window(1280, 720, "Speed Engine"))); // has to be BEFORE ResourceManager
		m_subSystems.push_back(std::unique_ptr<InputSystem>(new InputSystem(getSubSystem<Window>()->inputEvents())));
		m_subSystems.push_back(std::unique_ptr<ResourceManager>(new ResourceManager()));
		m_subSystems.push_back(std::unique_ptr<RendererManager>(new RendererManager(*getSubSystem<Window>())));
		m_subSystems.push_back(std::unique_ptr<PhysicsSystem>(new PhysicsSystem()));
		m_subSystems.push_back(std::unique_ptr<SceneSystem>(new SceneSystem()));
		m_subSystems.push_back(std::unique_ptr<ScriptingSystem>(new ScriptingSystem()));
		m_subSystems.push_back(std::unique_ptr<AudioSystem>(new AudioSystem()));
		m_subSystems.push_back(std::unique_ptr<UISystem>(new UISystem()));


		// init all subsystems
		for (auto& subSystem : m_subSystems)
		{
			subSystem->init();
		}

		// subscribe to window close event
		m_windowCloseEvent = getSubSystem<EventSystem>()->subscribe(EventType::WindowClose,
		    [this](const Event& event)
		    {
				m_shutdownRequested.store(true);
		    });
		m_cameraSwitchEvent = getSubSystem<EventSystem>()->subscribe(EventType::CameraSwitched,
		    [this](const Event& event)
		    {
			    const SwitchCameraEvent& cameraEvent = static_cast<const SwitchCameraEvent&>(event);
			    EngineStates::setCamera(cameraEvent.camera);
		    });
	}
	void Engine::start()
	{
		m_shutdownRequested.store(false);

		SceneSystem* sceneSystem = getSubSystem<SceneSystem>();
		sceneSystem->setCurrentScene("Assets/Data/default-scene.json");

		ResourceManager* resourceManager = getSubSystem<ResourceManager>();
		uint32_t fontid = resourceManager->loadFont({ 
			.path = "fonts/Quicksand-Regular.ttf", 
			.pixelHeight = 32, 
			.name = "QSfont" });
		QSfont = resourceManager->getResource<Font>(fontid)->getHandle();
	}

	void Engine::run()
	{
#if defined(COMPILE_WITH_EDITOR)
		runSingleThreaded();
#else
		runThreaded();
#endif
	}

	void Engine::runSingleThreaded()
	{
		using Clock = std::chrono::steady_clock;
		auto lastTime = Clock::now();
		double physicsAccumulator = 0.0;
		auto* window = getSubSystem<Window>();
		auto* renderer = getSubSystem<RendererManager>();
		while (!m_shutdownRequested.load() && !window->shouldClose())
		{
			window->update(0.0);
			const auto now = Clock::now();
			const double deltaTime = std::chrono::duration<double>(now - lastTime).count();
			lastTime = now;

			physicsAccumulator += deltaTime;
			const int physicsSteps = static_cast<int>(physicsAccumulator / simulationDeltaTime);
			physicsAccumulator -= physicsSteps * simulationDeltaTime;
			updateSimulation(deltaTime, physicsSteps);
			m_frameBuilder.captureSimulation(*getSubSystem<SceneSystem>()->getCurrentScene(), now);
			produceRenderFrame();

			auto read = m_frameExchange.tryAcquireReady();
			renderer->render(read.frame(), 1.0f);
		}
	}

	void Engine::runThreaded()
	{
		m_simulationThread = std::jthread(
		    [this](std::stop_token stopToken)
		    {
			    runSimulation(stopToken);
		    });
		runMainLoop();
		stopSimulation();
	}


	void Engine::updateSimulation(double deltaTime, int physicsSteps)
	{
		m_frameBuilder.beginSimulation();
		getSubSystem<SceneSystem>()->processPendingTransition();
		getSubSystem<InputSystem>()->update(deltaTime);
		getSubSystem<EventSystem>()->update(deltaTime);
		getSubSystem<SceneSystem>()->update(deltaTime);
		getSubSystem<ScriptingSystem>()->update(deltaTime);
		for (int step = 0; step < physicsSteps; ++step)
			getSubSystem<PhysicsSystem>()->step(simulationDeltaTime);
		getSubSystem<AudioSystem>()->update(deltaTime);
		getSubSystem<UISystem>()->update(deltaTime);
		if (EngineStates::isEditorCameraActive() && !getSubSystem<UISystem>()->isAcceptingCommands())
			m_cameraController.update(EngineStates::getEditorCamera(), *getSubSystem<InputSystem>(), deltaTime);
		
		// temp test text
		submitText(QSfont, "Hello text blah blah blah\nnew line: !@#$%^&*()_+-=,./;'[]<>?:{}|", { 20.0f, 20.0f });
	}

	void Engine::produceRenderFrame()
	{
		if (auto write = m_frameExchange.tryBeginWrite())
		{
			RenderFrame& frame = write.frame();
			m_frameBuilder.build(frame, *getSubSystem<PhysicsSystem>(), *getSubSystem<SceneSystem>()->getCurrentScene());
			write.publish();
		}
	}

	void Engine::runSimulation(std::stop_token stopToken)
	{
		using Clock = std::chrono::steady_clock;
		const auto interval = std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(simulationDeltaTime));
		auto nextTick = Clock::now();

		while (!stopToken.stop_requested())
		{
			std::this_thread::sleep_until(nextTick);
			if (stopToken.stop_requested())
				break;
			updateSimulation(simulationDeltaTime, 1);
			m_frameBuilder.captureSimulation(*getSubSystem<SceneSystem>()->getCurrentScene(), nextTick);
			produceRenderFrame();
			nextTick += interval;
		}
		getSubSystem<InputSystem>()->update(0.0);
		getSubSystem<EventSystem>()->update(0.0);
	}

	void Engine::runMainLoop()
	{
		auto* window = getSubSystem<Window>();
		auto* renderer = getSubSystem<RendererManager>();
		std::optional<FrameExchange::ReadHandle> displayed; // The currently displayed frame
		while (!m_shutdownRequested.load() && !window->shouldClose())
		{
			window->update(0.0);
			if (auto next = m_frameExchange.tryAcquireReady())
				displayed.emplace(std::move(next));
			if (displayed)
			{
				const RenderFrame& frame = displayed->frame();
				const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - frame.tickTime).count();
				const float t = static_cast<float>(elapsed / simulationDeltaTime);
				renderer->render(frame, t);
			}
			// no frame ready, don't busy wait
			else
				window->waitForEvents(0.002);
		}
	}


	void Engine::stopSimulation()
	{
		m_shutdownRequested.store(true);
		if (m_simulationThread.joinable())
		{
			m_simulationThread.request_stop();
			m_simulationThread.join();
		}
	}

	void Engine::stop()
	{
		stopSimulation();
		for (auto& subSystem : m_subSystems | std::views::reverse)
		{
			subSystem->shutdown();
		}
	}
} // namespace SE
