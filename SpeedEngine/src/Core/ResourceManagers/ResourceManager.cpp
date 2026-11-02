#include "pch.h"

#include "ResourceManager.h"

#include "../Logger/Logger.h"
#include "Engine.h"
#include "Material/MaterialManager.h"
#include "MeshResource/MeshResourceManager.h"
#include "Shader/ShaderManager.h"

namespace SE
{
	void ResourceManager::init()
	{
		SubSystem::init();

		// init managers
		m_managers.emplace_back(std::make_unique<ShaderManager>());
		m_managers.emplace_back(std::make_unique<MaterialManager>());
		m_managers.emplace_back(std::make_unique<MeshResourceManager>());
		m_managers.emplace_back(std::make_unique<FontManager>());
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

	const std::string* ResourceManager::getString(std::string_view filePath)
	{
		std::string path(filePath);

		// find in cache
		auto it = m_stringCache.find(path);
		if (it != m_stringCache.end())
		{
			return &it->second;
		}
		// open file
		std::ifstream file(path, std::ios::in | std::ios::binary);
		if (!file.is_open())
		{
			m_logger->warn("Failed to open file: {}", path);
			return nullptr;
		}
		std::error_code e;
		const auto size = std::filesystem::file_size(path, e);
		if (e)
		{
			m_logger->warn("Failed to get file size: {}. Error: {}", path, e.message());
			return nullptr;
		}
		std::string contents(size, '\0');
		if (size > 0 && !file.read(contents.data(), size))
		{
			m_logger->warn("Failed to read file: {}", path);
			return nullptr;
		}
		file.close();

		auto [it2, _] = m_stringCache.emplace(std::move(path), std::move(contents));
		return &it2->second;
	}

	Resource* ResourceManager::getResource(uint32_t id)
	{
		auto it = m_resourceTypes.find(id);
		if (it != m_resourceTypes.end())
		{
			if (it->second == ResourceType::Shader)
			{
				return _getManager<ShaderManager>()->getShader(id);
			}
			else if (it->second == ResourceType::Material)
			{
				return _getManager<MaterialManager>()->getMaterial(id);
			}
			else if (it->second == ResourceType::MeshResource)
			{
				return _getManager<MeshResourceManager>()->getMeshResource(id);
			}
			else if (it->second == ResourceType::Font)
			{
				return _getManager<FontManager>()->getFont(id);
			}
			//.......
		}
		return nullptr;
	}

	bool ResourceManager::removeResource(uint32_t id)
	{
		bool removed = false;
		auto it = m_resourceTypes.find(id);
		if (it == m_resourceTypes.end())
			return false;
		if (it->second == ResourceType::Shader)
		{
			removed = _getManager<ShaderManager>()->removeShader(id);
#if defined(COMPILE_WITH_EDITOR)
			Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<ResourceChanged>(id, it->second, false));
#endif
			return removed;
		}
		else if (it->second == ResourceType::Material)
		{
			removed = _getManager<MaterialManager>()->removeMaterial(id);
#if defined(COMPILE_WITH_EDITOR)
			Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<ResourceChanged>(id, it->second, false));
#endif
			return removed;
		}
		else if (it->second == ResourceType::MeshResource)
		{
			removed = _getManager<MeshResourceManager>()->removeMeshResource(id);
#if defined(COMPILE_WITH_EDITOR)
			Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<ResourceChanged>(id, it->second, false));
#endif
			return removed;
		}
		else if (it->second == ResourceType::Font)
		{
			removed = _getManager<FontManager>()->removeFont(id);
			if (removed)
			{
				m_resourceTypes.erase(it);
#if defined(COMPILE_WITH_EDITOR)
				Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<ResourceChanged>(id, ResourceType::Font, false));
#endif
			}
			return removed;
		}
		//.....
		return false;
	}
} // namespace SE
