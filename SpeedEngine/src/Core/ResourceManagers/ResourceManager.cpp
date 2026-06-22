#include "pch.h"
#include "ResourceManager.h"
#include "../Logger/Logger.h"

namespace SE
{
	uint32_t ResourceManager::generateId()
	{
		return m_ids.fetch_add(1, std::memory_order_relaxed);
	}
}