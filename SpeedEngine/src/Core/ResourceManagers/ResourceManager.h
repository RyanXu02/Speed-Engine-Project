#pragma once

#include "Core/SubSystems/SubSystem.h"
#include "Resource.h"
#include "MeshResource/MeshResource.h"
#include "Terrain/Terrain.h"
#include "Material/Material.h"
#include "Material/Texture.h"
#include "Font/Font.h"
#include "Shader/ShaderSource.h"

namespace SE
{
	// @brief The state of a resource:
	// @brief	Loading: Still being loaded on worker thread
	// @brief	Ready: CPU data has finished loading and has been published
	// @brief	Failed: The resource failed to load
	enum class ResourceState { Loading, Ready, Failed };

	constexpr const char* resourceStateName(ResourceState state)
	{
		switch (state)
		{
		case ResourceState::Loading: return "Loading";
		case ResourceState::Ready: return "Ready";
		case ResourceState::Failed: return "Failed";
		}
		return "Unknown";
	}

	struct ShaderLoadDesc{ std::string_view vertexShaderPath, fragmentShaderPath, name; };
	struct FileLoadDesc { std::string_view path, name; };
	struct FontLoadDesc { std::string_view path; uint32_t pixelHeight; std::string_view name; };
	struct MaterialDesc { ResourceID shaderId; std::span<const TextureBinding> textures; std::string_view name; };
	
	// @brief Metadata about a resource
	struct ResourceInfo
	{
		ResourceID id;
		ResourceType type;
		ResourceState state;
		std::string name;
		std::string path;
		std::string error;
	};

	// @brief Manages CPU data of resources
	// @brief This SubSystem doesn't know anything about graphics or physics
	// @brief Any published resources are immutable and live until shutdown
	class ResourceManager : public SubSystem
	{
		friend class Engine;
	public:
		void init() override;
		void shutdown() override;

		// @brief Publish completed loads on the main thread
		void processCompletedLoads();

		// @brief Request a mesh to be loaded from disk
		// @return A unique ID for the resource, or 0 if the request failed
		ResourceID requestMesh(FileLoadDesc desc);
		// @brief Request a terrain to be loaded from disk
		// @return A unique ID for the resource, or 0 if the request failed
		ResourceID requestTerrain(FileLoadDesc desc);
		// @brief Request a texture to be loaded from disk
		// @return A unique ID for the resource, or 0 if the request failed
		ResourceID requestTexture(FileLoadDesc desc);
		// @brief Request a font to be loaded from disk
		// @return A unique ID for the resource, or 0 if the request failed
		ResourceID requestFont(FontLoadDesc desc);
		// @brief Request a shader to be loaded from disk
		// @return A unique ID for the resource, or 0 if the request failed
		ResourceID requestShader(ShaderLoadDesc desc);
		// @brief Request a compute shader to be loaded from disk
		// @return A unique ID for the resource, or 0 if the request failed
		ResourceID requestComputeShader(FileLoadDesc desc);
		// @brief Create a material from a shader and texture bindings
		// @return A unique ID for the resource, or 0 if the request failed
		ResourceID createMaterial(const MaterialDesc& desc);

		// @brief Block the calling thread until the resource is ready or failed
		// @brief Use during your initialization if you need something to be ready
		// @returns true if the resource is ready, false if it failed or the ID is invalid
		bool waitForStartupResource(ResourceID id);

		// @brief Returns the state of a resource
		std::optional<ResourceState> getResourceState(ResourceID id) const;

		// @brief Returns a list of all resources and their states
		std::vector<ResourceInfo> inspectResources() const;

		// @brief Returns a pointer to the resource data if it is ready, or nullptr otherwise
		// @tparam T The type of resource to get
		// @param id The ID of the resource to get
		// @return A pointer to the resource data if it is ready, or nullptr otherwise
		template<typename T>
		const T* getResource(ResourceID id) const
		{
			std::lock_guard lock(m_mutex);

			const auto entry = m_entries.find(id);
			if (entry == m_entries.end())
				return nullptr;

			const auto* data = std::get_if<std::unique_ptr<const T>>(&entry->second.data);
			return data ? data->get() : nullptr;
		}

	private:
		ResourceManager() : SubSystem("ResourceManager") {}

		// @brief Key for uniquely identifying a resource to load
		struct AssetKey
		{
			ResourceType type;
			std::string path;
			std::string fragmentPath{};
			uint32_t pixelHeight = 0;
			auto operator<=>(const AssetKey&) const = default;
		};

		// Inheritance is the source of all evil
		// We know the types of resources already
		using Data = std::variant<
			std::monostate,
			std::unique_ptr<const MeshResource>,
			std::unique_ptr<const Terrain>,
			std::unique_ptr<const TextureData>,
			std::unique_ptr<const FontData>,
			std::unique_ptr<const ShaderSource>,
			std::unique_ptr<const Material>
		>;

		// @brief metadata, state, and the actual resource data
		struct Entry { ResourceInfo info; Data data; };
		// @brief A job to load a resource
		struct Job { ResourceID id; AssetKey key; };
		// @brief A completed load of a resource
		struct Completion { ResourceID id; Data data; std::string error; };

		// A request for a resource
		ResourceID request(AssetKey key, std::string_view name);
		// Worker thread function to load resources
		void runLoader(std::stop_token stop);

		mutable std::mutex m_mutex; // Don't want to un-const functions just to lock the mutex, so make it mutable

		std::condition_variable_any m_workAvailable;
		std::condition_variable m_resultAvailable;

		std::unordered_map<ResourceID, Entry> m_entries;
		std::map<AssetKey, ResourceID> m_assets; // To prevent duplicate loads

		std::deque<Job> m_jobs; // Pending background loading jobs
		std::deque<Completion> m_completed; // Results waiting for publication

		ResourceID m_nextId = 1;

		std::thread::id m_owner; // The thread that owns ResourceManager
		bool m_accepting = false;
		std::jthread m_worker; // resource loading thread
	};
}