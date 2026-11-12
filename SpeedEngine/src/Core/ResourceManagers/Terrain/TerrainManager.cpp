#include "pch.h"

#include "TerrainManager.h"

#include "Core/Logger/Logger.h"

namespace SE
{
	void TerrainManager::init()
	{
		Manager::init();

		m_terrains.reserve(16);
	}

	void TerrainManager::shutdown()
	{
		Manager::shutdown();

		for (auto& [k, terrain] : m_terrains)
		{
			if (terrain)
			{
				terrain->destroy();
			}
		}
		m_terrains.clear();
		m_terrainPaths.clear();

		m_logger->info("TerrainManager cleared all terrain resources.");
	}

	uint32_t TerrainManager::addTerrain(uint32_t id, std::string_view terrainName, std::string_view terrainPath)
	{
		const auto path = normalizeResourcePath(terrainPath);
		if (auto it = m_terrainPaths.find(path); it != m_terrainPaths.end())
		{
			assert(m_terrains.contains(it->second));
			return it->second;
		}

		auto newTerrain = std::make_unique<Terrain>(id, terrainName, *m_logger);
		if (path.empty() || !newTerrain->init(path))
		{
			m_logger->warn("Failed to initialize Terrain {} from {}", terrainName, terrainPath);
			return 0;
		}

		// Actually add the terrain to the map
		m_terrains[id] = std::move(newTerrain);
		m_terrainPaths.emplace(path, id);
		assert(m_terrainPaths.size() == m_terrains.size());

		m_logger->info("Added Terrain {} with id {}", terrainName, id);

		return id;
	}

	Terrain* TerrainManager::getTerrain(uint32_t terrainId)
	{
		auto it = m_terrains.find(terrainId);
		if (it == m_terrains.end())
			return nullptr;

		return it->second.get();
	}

	bool TerrainManager::removeTerrain(uint32_t terrainId)
	{
		auto it = m_terrains.find(terrainId);
		if (it == m_terrains.end())
			return false;

		m_terrains.erase(terrainId);
		std::erase_if(m_terrainPaths, [terrainId](const auto& entry) { return entry.second == terrainId; });
		assert(m_terrainPaths.size() == m_terrains.size());

		return true;
	}
} // namespace SE
