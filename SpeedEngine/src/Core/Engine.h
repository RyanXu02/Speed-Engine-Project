#pragma once

namespace SE
{
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
		
		std::vector<std::unique_ptr<SubSystems::SubSystem>> m_subSystems;
	};
}