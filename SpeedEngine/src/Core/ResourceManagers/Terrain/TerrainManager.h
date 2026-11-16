#pragma once
#include "Core/ResourceManagers/Manager.h"
#include "Terrain.h"

namespace SE
{
	class TerrainManager : public Manager
	{
		friend class ResourceManager;

	public:
		TerrainManager(const std::string& name = "TerrainManager")
		    : Manager(ResourceType::Terrain, name)
		{}
		~TerrainManager() override = default;

		void init() override;
		void shutdown() override;

	private:
		// @brief adds a terrain resource to this manager
		// @param id The unique ID for the terrain
		// @param meshName The name of the terrain shown in the resource browser
		// @param meshPath The file path to the terrain file
		// @returns the ID of the newly added terrain, or 0 if the terrain
		// could not be added
		uint32_t addTerrain(uint32_t id, std::string_view terrainName, std::string_view terrainPath);

		// @brief retrieves a terrain from the manager given an id
		// @param terrainId the id of terrain to retrieve
		// @returns the requested terrain if it exists, nullptr if not
		Terrain* getTerrain(uint32_t terrainId);

		// @brief removes a terrain from the manager given an id
		// @param terrainId the id of terrain to remove
		// @returns true if successfully removed, false if not
		bool removeTerrain(uint32_t terrainId);

		std::unordered_map<uint32_t, std::unique_ptr<Terrain>> m_terrains; // map of terrain id to terrain resource object
		std::unordered_map<std::string, uint32_t> m_terrainPaths; // map of terrain file paths to terrain id
	};

} // namespace SE
