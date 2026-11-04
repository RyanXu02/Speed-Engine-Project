#pragma once
#include "../Manager.h"
#include "MeshResource.h"

namespace SE
{
	class MeshResourceManager : public Manager
	{
		friend class ResourceManager;

	public:
		MeshResourceManager(const std::string& name = "MeshResourceManager")
		    : Manager(ResourceType::MeshResource, name)
		{}
		~MeshResourceManager() override = default;

		void init() override;
		void shutdown() override;

	private:
		// @brief adds a mesh resource to this manager
		// @param id The unique ID for the mesh resource
		// @param meshName The name of the mesh resource shown in the resource browser
		// @param meshPath The file path to the mesh file
		// @returns the ID of the newly added mesh resource, or 0 if the mesh resource
		// could not be added
		uint32_t addMeshResource(uint32_t id, std::string_view meshName, std::string_view meshPath);

		// @brief retrieves a mesh resource from the manager given an id
		// @param meshResourceId the id of mesh resource to retrieve
		// @returns the requested mesh resource if it exists, nullptr if not
		MeshResource* getMeshResource(uint32_t meshResourceId);

		// @brief removes a mesh resource from the manager given an id
		// @param meshResourceId the id of mesh resource to remove
		// @returns true if successfully removed, false if not
		bool removeMeshResource(uint32_t meshResourceId);

		std::unordered_map<uint32_t, std::unique_ptr<MeshResource>> m_meshresources; // map of mesh resource id to mesh resource object
		std::unordered_map<std::string, uint32_t> m_meshPaths; // map of mesh resource file paths to mesh resource id
	};

} // namespace SE
