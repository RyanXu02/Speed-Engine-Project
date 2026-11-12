#include "pch.h"

#include "ResourceManager.h"

#include "Core/Logger/Logger.h"
#include "Core/SubSystems/Events/EventSystem.h"
#include "Engine.h"

namespace SE
{
	void ResourceManager::init()
	{
		SubSystem::init();

		m_shaderManager.init();
		m_materialManager.init();
		m_meshManager.init();
		m_terrainManager.init();
		m_fontManager.init();
	}

	void ResourceManager::shutdown()
	{
		SubSystem::shutdown();

		m_materialManager.shutdown();
		m_shaderManager.shutdown();
		m_textures.clear();
		m_texturePaths.clear();
		m_meshManager.shutdown();
		m_terrainManager.shutdown();
		m_fontManager.shutdown();
		m_resourceTypes.clear();
		m_stringCache.clear();
	}

	uint32_t ResourceManager::loadShader(const ShaderLoadDesc& desc)
	{
		uint32_t id = m_shaderManager.addShader(generateId(), desc.vertexShaderPath, desc.fragmentShaderPath, desc.name);
		if (id != 0 && m_resourceTypes.emplace(id, ResourceType::Shader).second)
		{
#if defined(COMPILE_WITH_EDITOR)
			Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<ResourceChanged>(id, ResourceType::Shader, true));
#endif
		}
		return id;
	}

	uint32_t ResourceManager::loadMesh(const MeshLoadDesc& desc)
	{
		uint32_t id = m_meshManager.addMeshResource(generateId(), desc.name, desc.path);
		if (id != 0 && m_resourceTypes.emplace(id, ResourceType::MeshResource).second)
		{
#if defined(COMPILE_WITH_EDITOR)
			Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<ResourceChanged>(id, ResourceType::MeshResource, true));
#endif
		}
		return id;
	}

	uint32_t ResourceManager::loadTerrain(const TerrainLoadDesc& desc)
	{
		uint32_t id = m_terrainManager.addTerrain(generateId(), desc.name, desc.path);
		if (id != 0 && m_resourceTypes.emplace(id, ResourceType::Terrain).second)
		{
#if defined(COMPILE_WITH_EDITOR)
			Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<ResourceChanged>(id, ResourceType::Terrain, true));
#endif
		}
		return id;
	}

	uint32_t ResourceManager::loadFont(const FontLoadDesc& desc)
	{
		uint32_t id = m_fontManager.addFont(generateId(), desc.path, desc.pixelHeight, desc.name);
		if (id != 0 && m_resourceTypes.emplace(id, ResourceType::Font).second)
		{
#if defined(COMPILE_WITH_EDITOR)
			Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<ResourceChanged>(id, ResourceType::Font, true));
#endif
		}
		return id;
	}

	uint32_t ResourceManager::loadTexture(const TextureLoadDesc& desc)
	{
		const auto path = normalizeResourcePath(desc.path);
		if (auto it = m_texturePaths.find(path); it != m_texturePaths.end())
		{
			assert(m_textures.contains(it->second));
			return it->second;
		}

		const uint32_t id = generateId();
		auto texture = std::make_unique<Texture>(id, desc.name);
		const auto error = texture->init(path);
		if (!error.empty())
		{
			m_logger->warn("{}", error);
			return 0;
		}
		m_textures.emplace(id, std::move(texture));
		m_texturePaths.emplace(path, id);
		m_resourceTypes.emplace(id, ResourceType::Texture);
#if defined(COMPILE_WITH_EDITOR)
		Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<ResourceChanged>(id, ResourceType::Texture, true));
#endif
		return id;
	}

	uint32_t ResourceManager::createMaterial(const MaterialDesc& desc)
	{
		uint32_t id = m_materialManager.addMaterial(generateId(), desc.shaderId, desc.textures, desc.name);
		if (id != 0 && m_resourceTypes.emplace(id, ResourceType::Material).second)
		{
#if defined(COMPILE_WITH_EDITOR)
			Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<ResourceChanged>(id, ResourceType::Material, true));
#endif
		}
		return id;
	}


	uint32_t ResourceManager::generateId()
	{
		return m_ids.fetch_add(1, std::memory_order_relaxed);
	}

	const std::string* ResourceManager::getString(std::string_view filePath)
	{
		const auto path = normalizeResourcePath(filePath);

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

		auto [it2, _] = m_stringCache.emplace(path, std::move(contents));
		return &it2->second;
	}

	void ResourceManager::forgetShaderSources(const std::pair<std::string, std::string>& paths)
	{
		m_stringCache.erase(paths.first);
		m_stringCache.erase(paths.second);
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
			else if (it->second == ResourceType::Terrain)
			{
				return m_terrainManager.getTerrain(id);
			}
			else if (it->second == ResourceType::Font)
			{
				return m_fontManager.getFont(id);
			}
			else if (it->second == ResourceType::Texture)
			{
				auto texture = m_textures.find(id);
				return texture == m_textures.end() ? nullptr : texture->second.get();
			}
			//.......
		}
		return nullptr;
	}

	bool ResourceManager::removeResource(uint32_t id)
	{
		auto it = m_resourceTypes.find(id);
		if (it == m_resourceTypes.end())
			return false;

		const auto type = it->second;
		bool removed = false;
		switch (type)
		{
		case ResourceType::Shader: removed = m_shaderManager.removeShader(id); break;
		case ResourceType::Material: removed = m_materialManager.removeMaterial(id); break;
		case ResourceType::MeshResource: removed = m_meshManager.removeMeshResource(id); break;
		case ResourceType::Terrain: removed = m_terrainManager.removeTerrain(id); break;
		case ResourceType::Font: removed = m_fontManager.removeFont(id); break;
		case ResourceType::Texture: return false; // Cached textures cannot be removed individually, they live until shutdown
		}

		if (removed)
		{
			m_resourceTypes.erase(it);
#if defined(COMPILE_WITH_EDITOR)
			Engine::Instance().getSubSystem<EventSystem>()->publish(std::make_unique<ResourceChanged>(id, type, false));
#endif
		}
		return removed;
	}
} // namespace SE
