#pragma once
#include "SubSystems/Events/EventSubscription.h"
#include "Features/CameraController.h"
#include "SubSystems/Graphics/Rendering/RenderFrameBuilder.h"
#include "Threading/FrameExchange.h"

namespace SE
{
	class SubSystem;

	class Engine
	{
	public:
		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;
		Engine(Engine&&) = delete;
		Engine& operator=(Engine&&) = delete;

		~Engine();
		// @brief Get the singleton instance of the Engine
		// @brief THIS SHOULD BE THE ONLY INTERFACE TO ACCESS ANY SUBSYSTEMS WITHIN
		// THE ENGINE
		static Engine& Instance();

		void init();
		void start();
		void run();
		void stop();
		CameraController& getCameraController() { return m_cameraController; }

		// @brief Get the requested SubSystem, use this for all subsystems in the
		// engine
		// @tparam T The type of the SubSystem to get
		// @return A pointer to the requested SubSystem type if it exists, otherwise
		// returns nullptr
		template<typename T>
		requires std::derived_from<T, SubSystem> T* getSubSystem()
		{
			auto it = std::ranges::find_if(m_subSystems,
			    [&](const std::unique_ptr<SubSystem>& subSystem)
			    {
				    return dynamic_cast<T*>(subSystem.get()) != nullptr;
			    });
			if (it != m_subSystems.end())
				return static_cast<T*>(it->get());
			throw std::runtime_error("SubSystem " + std::string(typeid(T).name()) + " not found or not initialized!");
		}

	private:
		Engine() = default;

		void runSingleThreaded();
		void runThreaded();

		// @brief Runs platform updates and rendering on the main thread
		void runMainLoop();
		// @brief Runs the simulation loop of the engine, which updates the simulation state and produces render frames
		void runSimulation();

		// @brief Updates the simulation state of the engine
		// @param deltaTime The time elapsed since the last update
		// @param physicsSteps The number of physics steps to perform
		void updateSimulation(double deltaTime, int physicsSteps);

		// @brief Produces a render frame for the current simulation state
		void produceRenderFrame();

		// @brief Stops the simulation thread and waits for it to finish
		void stopSimulation();

		std::atomic<bool> m_shutdownRequested{ false };

		std::vector<std::unique_ptr<SubSystem>> m_subSystems;

		CameraController m_cameraController;

		RenderFrameBuilder m_frameBuilder;
		FrameExchange m_frameExchange;

		std::jthread m_simulationThread;

		EventSubscription m_windowCloseEvent;
		EventSubscription m_cameraSwitchEvent;
	};
} // namespace SE
