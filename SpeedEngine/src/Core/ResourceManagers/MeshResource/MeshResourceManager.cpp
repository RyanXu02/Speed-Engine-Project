#include "pch.h"
#include "MeshResourceManager.h"
#include "../../Logger/Logger.h"

namespace SE
{
	void MeshResourceManager::init() {
		Manager::init();

		m_meshresources.reserve(16);
	}

	void MeshResourceManager::shutdown() {
		Manager::shutdown();

		for (auto& [k, meshresource] : m_meshresources)
		{
			if (meshresource)
			{
				meshresource->destroy();
			}
		}
		m_meshresources.clear();

		m_logger->info("MeshResourceManager cleared all mesh resources.");
	}

	uint32_t MeshResourceManager::addMeshResource(uint32_t id, std::string_view meshName, std::string_view objpath, std::string_view mtlpath) { //asdfhasfhlsadfsakjf
		auto newMesh = std::make_unique<MeshResource>(id, meshName, *m_logger);
		if (!newMesh->init(objpath, mtlpath)) return 0;
		return id;
	}

	MeshResource* MeshResourceManager::getMeshResource(uint32_t meshResourceId) {
		auto it = m_meshresources.find(meshResourceId);
		if (it == m_meshresources.end()) return nullptr;

		return it->second.get();
	}

	bool MeshResourceManager::removeMeshResource(uint32_t meshResourceId) {
		auto it = m_meshresources.find(meshResourceId);
		if (it == m_meshresources.end()) return false;

		m_meshresources.erase(meshResourceId);
		return true;
	}
}