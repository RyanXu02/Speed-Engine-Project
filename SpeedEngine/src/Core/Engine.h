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
		
		std::vector<std::unique_ptr<SubSystem>> m_subSystems;

		EventSubscription m_windowCloseEvent;
	};
}