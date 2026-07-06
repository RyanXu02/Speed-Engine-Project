#include "pch.h"
#include "ResourceManager.h"
#include "../Logger/Logger.h"

#include "Shader/ShaderManager.h"

namespace SE
{

	ResourceManager* ResourceManager::s_instance = nullptr;

	void ResourceManager::init()
	{
		SubSystem::init();

		s_instance = this;

		// init managers
		m_managers.emplace_back(std::make_unique<ShaderManager>());
		//m_managers.emplace_back(std::make_unique<Material>());
		for (auto& manager : m_managers)
		{
			manager->init();
		}
	}

	void ResourceManager::shutdown()
	{
		SubSystem::shutdown();

		for (auto& manager : m_managers)
		{
			manager->shutdown();
		}
		m_managers.clear();

	}

	uint32_t ResourceManager::generateId()
	{
		return m_ids.fetch_add(1, std::memory_order_relaxed);
	}

	std::string ResourceManager::getString(std::string_view filePath)
	{
		// find in cache
		auto it = m_stringCache.find(filePath.data());
		if (it != m_stringCache.end())
		{
			return it->second;
		}
		// open file
		std::ifstream file(filePath.data());
		if (!file.is_open())
		{
			m_logger->warn("Failed to open file: {}", filePath);
			return "";
		}
		std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();

		m_stringCache[filePath.data()] = contents;
		return contents;
	}
}