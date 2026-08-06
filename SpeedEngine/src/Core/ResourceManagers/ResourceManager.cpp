#include "pch.h"
#include "ResourceManager.h"
#include "../Logger/Logger.h"

#include "Shader/ShaderManager.h"
#include "Material/MaterialManager.h"
#include "MeshResource/MeshResourceManager.h"

namespace SE
{

	ResourceManager* ResourceManager::s_instance = nullptr;

	void ResourceManager::init()
	{
		SubSystem::init();

		s_instance = this;

		// init managers
		m_managers.emplace_back(std::make_unique<ShaderManager>());
		m_managers.emplace_back(std::make_unique<MaterialManager>());
		m_managers.emplace_back(std::make_unique<MeshResourceManager>());
		//...
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

	Resource* ResourceManager::getResource(uint32_t id) {
		auto it = m_resourceTypes.find(id);
		if (it != m_resourceTypes.end()) {
			if (it->second == ResourceType::Shader) {
				return _getManager<ShaderManager>()->getShader(id);
			}
			else if (it->second == ResourceType::Material) {
				return _getManager<MaterialManager>()->getMaterial(id);
			}
			else if (it->second == ResourceType::MeshResource) {
				return _getManager<MeshResourceManager>()->getMeshResource(id);
			}
			//.......
		}
		return nullptr;
	}

	bool ResourceManager::removeResource(uint32_t id) {
		bool removed = false;
		auto it = m_resourceTypes.find(id);
		if (it == m_resourceTypes.end()) return false;
		if (it->second == ResourceType::Shader) {
			removed = _getManager<ShaderManager>()->removeShader(id);
			EventSystem::Instance().publish(std::make_unique<ResourceChanged>(id, it->second, false));
			return removed;
		}
		else if (it->second == ResourceType::Material) {
			removed = _getManager<MaterialManager>()->removeMaterial(id);
			EventSystem::Instance().publish(std::make_unique<ResourceChanged>(id, it->second, false));
			return removed;
		}
		else if (it->second == ResourceType::MeshResource) {
			removed = _getManager<MeshResourceManager>()->removeMeshResource(id);
			EventSystem::Instance().publish(std::make_unique<ResourceChanged>(id, it->second, false));
			return removed;
		}
		//.....
		return false;
	}
}