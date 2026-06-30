#include "pch.h"
#include "SubSystem.h"
#include "../Logger/Logger.h"

namespace SE
{
	SubSystem::SubSystem(const std::string& name) : m_name(name), m_logger(nullptr), isActive(true)
	{
	}

	SubSystem::~SubSystem()
	{
		m_logger->shutdown();
	}

	void SubSystem::init()
	{
		m_logger = std::make_unique<Logger>();
		m_logger->init(m_name + "Logger");
		assert(m_logger != nullptr);

		m_logger->info("{} initialized.", m_name);
	}

	void SubSystem::update(double deltaTime)
	{
		m_logger->verbose("{} updating. Delta time: {}", m_name, deltaTime);
		if (!isActive) return;
	}

	void SubSystem::shutdown()
	{
		m_logger->info("{} shutting down.", m_name);
	}
}