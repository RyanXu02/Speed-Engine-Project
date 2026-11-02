#include "pch.h"

#include "ResourceManager.h"

#include "Core/Logger/Logger.h"
#include "Engine.h"

namespace SE
{
	void ResourceManager::init()
	{
		SubSystem::init();

		m_shaderManager.init();
		m_materialManager.init();
		m_meshManager.init();
		m_fontManager.init();
	}

	void ResourceManager::shutdown()
	{
		SubSystem::shutdown();

		m_shaderManager.shutdown();
		m_materialManager.shutdown();
		m_meshManager.shutdown();
		m_fontManager.shutdown();
	}

	uint32_t ResourceManager::loadShader(const ShaderLoadDesc& desc)
	{
		uint32_t id = m_shaderManager.addShader(generateId(), desc.vertexShaderPath, desc.fragmentShaderPath, desc.name);
		if (id != 0)
		{
			m_resourceTypes.insert({ id, ResourceType::Shader });
		}
#if defined(COMPILE_WITH_EDITOR)
		Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<ResourceChanged>(id, ResourceType::Shader, true));
#endif
		return id;
	}

	uint32_t ResourceManager::loadMesh(const MeshLoadDesc& desc)
	{
		uint32_t id = m_meshManager.addMeshResource(generateId(), desc.name, desc.path);
		if (id != 0)
		{
			m_resourceTypes.insert({ id, ResourceType::MeshResource });
		}
#if defined(COMPILE_WITH_EDITOR)
		Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<ResourceChanged>(id, ResourceType::MeshResource, true));
#endif
		return id;
	}

	uint32_t ResourceManager::loadFont(const FontLoadDesc& desc)
	{
		uint32_t id = m_fontManager.addFont(generateId(), desc.path, desc.pixelHeight, desc.name);
		if (id != 0)
		{
			m_resourceTypes.insert({ id, ResourceType::Font });
#if defined(COMPILE_WITH_EDITOR)
			Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<ResourceChanged>(id, ResourceType::Font, true));
#endif
		}
		return id;
	}

	uint32_t ResourceManager::createMaterial(const MaterialDesc& desc)
	{
		uint32_t id = m_materialManager.addMaterial(generateId(), desc.shaderId, desc.textures, desc.name);
		if (id != 0)
		{
			m_resourceTypes.insert({ id, ResourceType::Material });
		}
#if defined(COMPILE_WITH_EDITOR)
		Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<ResourceChanged>(id, ResourceType::Material, true));
#endif
		return id;
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
				return m_shaderManager.getShader(id);
			}
			else if (it->second == ResourceType::Material)
			{
				return m_materialManager.getMaterial(id);
			}
			else if (it->second == ResourceType::MeshResource)
			{
				return m_meshManager.getMeshResource(id);
			}
			else if (it->second == ResourceType::Font)
			{
				return m_fontManager.getFont(id);
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
			removed = m_shaderManager.removeShader(id);
#if defined(COMPILE_WITH_EDITOR)
			Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<ResourceChanged>(id, it->second, false));
#endif
			return removed;
		}
		else if (it->second == ResourceType::Material)
		{
			removed = m_materialManager.removeMaterial(id);
#if defined(COMPILE_WITH_EDITOR)
			Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<ResourceChanged>(id, it->second, false));
#endif
			return removed;
		}
		else if (it->second == ResourceType::MeshResource)
		{
			removed = m_meshManager.removeMeshResource(id);
#if defined(COMPILE_WITH_EDITOR)
			Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<ResourceChanged>(id, it->second, false));
#endif
			return removed;
		}
		else if (it->second == ResourceType::Font)
		{
			removed = m_fontManager.removeFont(id);
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
