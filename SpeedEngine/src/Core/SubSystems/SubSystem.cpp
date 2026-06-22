#include "pch.h"
#include "SubSystem.h"
#include "../Logger/Logger.h"

namespace SE
{
	SubSystem::SubSystem(const std::string& name) : m_name(name), m_logger(nullptr), isActive(true)
	{
	}

	SubSystem::~SubSystem() = default;

	void SubSystem::init()
	{
		m_logger = std::make_unique<Logger>();
		m_logger->init(m_name + "Logger");
		assert(m_logger != nullptr);
	}

	void SubSystem::update(double deltaTime)
	{
		if (!isActive)
			return;
	}

	void SubSystem::shutdown()
	{
		m_logger->shutdown();
	}
}