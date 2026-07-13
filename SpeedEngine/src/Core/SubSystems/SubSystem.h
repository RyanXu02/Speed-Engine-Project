#pragma once

namespace SE
{
	class Logger;

	// baseclass
	class SubSystem 
	{
	public:
		SubSystem(const std::string& name);
		virtual ~SubSystem();
		
		const std::string& getName() const 
		{
			return m_name;
		};

		virtual void init();
		virtual void update(double deltaTime);
		virtual void shutdown();

		void enable() { isActive = true; }
		void disable() { isActive = false; }
		bool getActive() const { return isActive; }

	protected:
		std::string m_name;
		std::unique_ptr<Logger> m_logger;

		bool isActive;
	};
}

