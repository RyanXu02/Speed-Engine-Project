#include "pch.h"

#include "MeshResourceManager.h"

#include "../../Logger/Logger.h"

namespace SE
{
	void MeshResourceManager::init()
	{
		Manager::init();

		m_meshresources.reserve(16);
	}

	void MeshResourceManager::shutdown()
	{
		Manager::shutdown();

		for (auto& [k, meshresource] : m_meshresources)
		{
			if (meshresource)
			{
				meshresource->destroy();
			}
		}
		m_meshresources.clear();
		m_meshPaths.clear();

		m_logger->info("MeshResourceManager cleared all mesh resources.");
	}

	uint32_t MeshResourceManager::addMeshResource(uint32_t id, std::string_view meshName, std::string_view meshPath)
	{
		const auto path = normalizeResourcePath(meshPath);
		if (auto it = m_meshPaths.find(path); it != m_meshPaths.end())
		{
			assert(m_meshresources.contains(it->second));
			return it->second;
		}

		auto newMesh = std::make_unique<MeshResource>(id, meshName, *m_logger);
		if (path.empty() || !newMesh->init(path))
		{
			m_logger->warn("Failed to initialize MeshResource {} from {}", meshName, meshPath);
			return 0;
		}

		// Actually add the mesh to the map!
		m_meshresources[id] = std::move(newMesh);
		m_meshPaths.emplace(path, id);
		assert(m_meshPaths.size() == m_meshresources.size());

		m_logger->info("Added MeshResource {} with id {}", meshName, id);

		return id;
	}

	MeshResource* MeshResourceManager::getMeshResource(uint32_t meshResourceId)
	{
		auto it = m_meshresources.find(meshResourceId);
		if (it == m_meshresources.end())
			return nullptr;

		return it->second.get();
	}

	bool MeshResourceManager::removeMeshResource(uint32_t meshResourceId)
	{
		auto it = m_meshresources.find(meshResourceId);
		if (it == m_meshresources.end())
			return false;

		m_meshresources.erase(meshResourceId);
		std::erase_if(m_meshPaths, [meshResourceId](const auto& entry) { return entry.second == meshResourceId; });
		assert(m_meshPaths.size() == m_meshresources.size());

		return true;
	}
} // namespace SE
