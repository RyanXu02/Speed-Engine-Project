#pragma once
#include "SubSystems/Events/EventSubscription.h"

#include <cassert>

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
		// @brief THIS SHOULD BE THE ONLY INTERFACE TO ACCESS ANY SUBSYSTEMS WITHIN THE ENGINE
		static Engine& Instance();

		void init();
		void start();
		void run();
		void stop();

		// @brief Get the requested SubSystem, use this for all subsystems in the engine
		// @tparam T The type of the SubSystem to get
		// @return A pointer to the requested SubSystem type if it exists, otherwise returns nullptr
		template <typename T> requires std::derived_from<T, SubSystem>
		T* getSubSystem()
		{
			auto it = std::ranges::find_if(m_subSystems, [&](const std::unique_ptr<SubSystem>& subSystem) {
				return dynamic_cast<T*>(subSystem.get()) != nullptr;
				});
			if (it != m_subSystems.end())
				return static_cast<T*>(it->get());
			throw std::runtime_error("SubSystem " + std::string(typeid(T).name()) + " not found or not initialized!");
		}
	private:
		Engine();
		
		bool m_isRunning;

		double m_deltaTime;
		double m_currentTime;
		double m_lastTime;


		std::vector<std::unique_ptr<SubSystem>> m_subSystems;

		EventSubscription m_windowCloseEvent;
	};
}