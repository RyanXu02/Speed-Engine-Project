#pragma once
#include "SubSystems/Events/EventSubscription.h"

namespace SE
{
	class SubSystem;

	class Engine
	{
	public:
		Engine();
		~Engine();

		void init();
		void start();
		void run();
		void stop();

	private:
		bool m_isRunning;
		double m_deltaTime;

		template <typename T> requires std::derived_from<T, SubSystem>
		T* _getSubSystem()
		{
			auto it = std::ranges::find_if(m_subSystems, [&](const std::unique_ptr<SubSystem>& subSystem) {
				return dynamic_cast<T*>(subSystem.get()) != nullptr;
				});
			if (it != m_subSystems.end())
				return static_cast<T*>(it->get());
			return nullptr;
		}

		std::vector<std::unique_ptr<SubSystem>> m_subSystems;

		EventSubscription m_windowCloseEvent;
	};
}