#pragma once

namespace SE
{
	class Logger;

	// @brief Base class for all subsystems in the engine
	class SubSystem 
	{
	public:
		SubSystem(const std::string& name);
		virtual ~SubSystem();

		SubSystem(const SubSystem&) = delete;
		SubSystem& operator=(const SubSystem&) = delete;
		SubSystem(SubSystem&&) = delete;
		SubSystem& operator=(SubSystem&&) = delete;
		
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

