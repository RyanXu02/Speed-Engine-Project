#pragma once

namespace SE
{
	class Logger;

	class SubSystem 
	{
	public:
		SubSystem(const std::string& name);
		
		const std::string& getName() const 
		{
			return m_name;
		};

		virtual void init();
		virtual void update(double deltaTime);
		virtual void shutdown();

		virtual ~SubSystem();

	protected:
		std::string m_name;
		std::unique_ptr<Logger> m_logger;

		bool isActive;
	};
}

