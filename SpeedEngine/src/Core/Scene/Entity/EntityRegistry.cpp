#include "pch.h"

#include "EntityRegistry.h"

namespace SE
{
	EntityRegistry::EntityRegistry() {}

	EntityRegistry::~EntityRegistry() {}

	uint32_t EntityRegistry::addEntity()
	{
		m_registry.try_emplace(m_entityID, ComponentRegistry());

		return _assignID();
	}

	const EntityRegistry::ComponentRegistry* EntityRegistry::getComponentRegistry(uint32_t entityID) const
	{
		auto it = m_registry.find(entityID);
		if (it == m_registry.end())
		{
			return nullptr;
		}
		return &it->second;
	}

	bool EntityRegistry::hasEntity(uint32_t entityID)
	{
		return m_registry.find(entityID) != m_registry.end();
	}

	bool EntityRegistry::removeEntity(uint32_t entityID)
	{
		auto it = m_registry.find(entityID);
		if (it == m_registry.end())
		{
			return false;
		}

		m_registry.erase(entityID);

		return true;
	}

	void EntityRegistry::clear()
	{
		m_registry.clear();
	}

	const EntityRegistry::Registry::const_iterator EntityRegistry::begin() const
	{
		return m_registry.begin();
	}

	const EntityRegistry::Registry::const_iterator EntityRegistry::end() const
	{
		return m_registry.end();
	}


	uint32_t EntityRegistry::_assignID()
	{
		return m_entityID.fetch_add(1, std::memory_order_relaxed);
	}

}
