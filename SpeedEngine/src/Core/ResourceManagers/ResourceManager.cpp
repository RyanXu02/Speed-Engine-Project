#include "pch.h"
#include "ResourceManager.h"
#include "Core/Logger/Logger.h"

namespace SE
{
	void ResourceManager::init()
	{
		SubSystem::init();
		m_owner = std::this_thread::get_id();
		m_accepting = true;
		m_worker = std::jthread([this](std::stop_token stop) { runLoader(stop); });
	}

	void ResourceManager::shutdown()
	{
		assert(std::this_thread::get_id() == m_owner);
		
		{
			std::lock_guard lock(m_mutex);
			m_accepting = false;
			m_jobs.clear();
		}

		m_worker.request_stop();
		m_worker.join();
		m_completed.clear();
		m_entries.clear();
		m_assets.clear();
		SubSystem::shutdown();
	}

	ResourceID ResourceManager::request(AssetKey key, std::string_view name)
	{
		std::lock_guard lock(m_mutex);
		if (!m_accepting)
			return 0;

		if (const auto found = m_assets.find(key); found != m_assets.end())
			return found->second;

		const auto id = m_nextId++;
		m_entries.emplace(id, Entry{ { id, key.type, ResourceState::Loading, std::string(name), key.path, {} }, {} });
		m_assets.emplace(key, id);
		m_jobs.push_back({ id, std::move(key) });
		m_workAvailable.notify_one();

		return id;
	}

	ResourceID ResourceManager::requestMesh(FileLoadDesc desc)
	{
		return request({ ResourceType::MeshResource, normalizeResourcePath(desc.path) }, desc.name);
	}
	ResourceID ResourceManager::requestTerrain(FileLoadDesc desc)
	{
		return request({ ResourceType::Terrain, normalizeResourcePath(desc.path) }, desc.name);
	}
	ResourceID ResourceManager::requestTexture(FileLoadDesc desc)
	{
		return request({ ResourceType::Texture, normalizeResourcePath(desc.path) }, desc.name);
	}
	ResourceID ResourceManager::requestFont(FontLoadDesc desc)
	{
		return request({ ResourceType::Font, normalizeResourcePath(desc.path), {}, desc.pixelHeight }, desc.name);
	}
	ResourceID ResourceManager::requestShader(ShaderLoadDesc desc)
	{
		if (desc.vertexShaderPath.ends_with(".frag") && desc.fragmentShaderPath.ends_with(".vert"))
			std::swap(desc.vertexShaderPath, desc.fragmentShaderPath);
		return request({ ResourceType::Shader, normalizeResourcePath(desc.vertexShaderPath),
			normalizeResourcePath(desc.fragmentShaderPath) }, desc.name);
	}
	ResourceID ResourceManager::requestComputeShader(FileLoadDesc desc)
	{
		return request({ ResourceType::Shader, normalizeResourcePath(desc.path) }, desc.name);
	}
	ResourceID ResourceManager::createMaterial(const MaterialDesc& desc)
	{
		for (const auto& binding : desc.textures)
			if (binding.usage < TextureType::Albedo || binding.usage > TextureType::Metallic)
				return 0;

		auto material = std::make_unique<const Material>(Material{ desc.shaderId, { desc.textures.begin(), desc.textures.end() } });
		
		std::lock_guard lock(m_mutex);

		if (!m_accepting)
			return 0;

		const auto id = m_nextId++;
		m_entries.emplace(id, Entry{ { id, ResourceType::Material, ResourceState::Ready, std::string(desc.name), {}, {} }, std::move(material) });
		
		return id;
	}

	void ResourceManager::runLoader(std::stop_token stop)
	{
		while (!stop.stop_requested())
		{
			Job job;
			{
				std::unique_lock lock(m_mutex);
				m_workAvailable.wait(lock, stop, [this] { return !m_jobs.empty(); });
				if (stop.stop_requested())
					return;
				job = std::move(m_jobs.front());
				m_jobs.pop_front();
			}

			Completion result{ job.id, {}, {} };
			const auto& key = job.key;
			try
			{
				if (!key.path.empty())
				{
					switch (key.type)
					{
					case ResourceType::MeshResource: result.data = loadMeshData(key.path, *m_logger); break;
					case ResourceType::Terrain: result.data = loadTerrainData(key.path, *m_logger); break;
					case ResourceType::Texture: result.data = loadTextureData(key.path); break;
					case ResourceType::Font: result.data = loadFontData(key.path, key.pixelHeight, *m_logger); break;
					case ResourceType::Shader: result.data = loadShaderSource(key.path, key.fragmentPath); break;
					case ResourceType::Material: break;
					}
				}

				if (!std::visit([](const auto& data) {
					if constexpr (requires { data.get(); }) return static_cast<bool>(data);
					else return false;
				}, result.data))
					result.error = key.path.empty() ? "Empty or invalid asset path" : fmt::format("Could not prepare '{}'", key.path);
			}
			catch (const std::exception& error)
			{
				result.error = fmt::format("Could not prepare '{}': {}", key.path, error.what());
			}

			{
				std::lock_guard lock(m_mutex);
				m_completed.push_back(std::move(result));
			}
			m_resultAvailable.notify_one();
		}
	}

	void ResourceManager::processCompletedLoads()
	{
		assert(std::this_thread::get_id() == m_owner);

		std::deque<Completion> completed;
		{
			std::lock_guard lock(m_mutex);
			completed.swap(m_completed);
			for (auto& result : completed)
			{
				auto& entry = m_entries.at(result.id);
				if (result.error.empty())
				{
					entry.data = std::move(result.data);
					entry.info.state = ResourceState::Ready;
				}
				else
				{
					entry.info.error = result.error;
					entry.info.state = ResourceState::Failed;
				}
			}
		}

		for (const auto& result : completed)
			if (!result.error.empty())
				m_logger->warn("Resource {} failed: {}", result.id, result.error);
	}

	bool ResourceManager::waitForStartupResource(ResourceID id)
	{
		assert(std::this_thread::get_id() == m_owner);
		while (true)
		{
			const auto state = getResourceState(id);
			if (state != ResourceState::Loading)
				return state == ResourceState::Ready;

			{
				std::unique_lock lock(m_mutex);
				m_resultAvailable.wait(lock, [this] { return !m_completed.empty(); });
			}

			processCompletedLoads();
		}
	}

	std::optional<ResourceState> ResourceManager::getResourceState(ResourceID id) const
	{
		std::lock_guard lock(m_mutex);
		const auto entry = m_entries.find(id);
		return entry == m_entries.end() ? std::nullopt : std::optional(entry->second.info.state);
	}

	std::vector<ResourceInfo> ResourceManager::inspectResources() const
	{
		std::lock_guard lock(m_mutex);
		std::vector<ResourceInfo> result;
		result.reserve(m_entries.size());
		for (const auto& [id, entry] : m_entries)
			result.push_back(entry.info);
		return result;
	}
}