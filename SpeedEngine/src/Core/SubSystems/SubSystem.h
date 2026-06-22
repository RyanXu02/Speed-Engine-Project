#pragma once

namespace SE::SubSystems
{
	class SubSystem 
	{
	public:
		const std::string& getName() const 
		{
			return m_name;
		};

		virtual void init();
		virtual void update(double deltaTime);
		virtual void shutdown();

		virtual ~SubSystem();

	protected:
		SubSystem(const std::string& name) : m_name(name) {};
		std::string m_name;
	};
}

