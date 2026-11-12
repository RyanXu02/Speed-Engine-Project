#pragma once
#include "Core/Logger/LoggerProxy.h"
#include "Core/ResourceManagers/Resource.h"
#include "Core/ResourceManagers/MeshResource/MeshResource.h"

#include <nlohmann/json.hpp>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>

struct aiScene;

namespace SE
{
	struct TerrainMaterial
	{
		std::string name;

		std::string heightMapPath;
		std::string normalMapPath;
		std::string splatMapPath;

		// Materials used based on splat map R, G, B respectively
		std::string materialHighPath;
		std::string materialMiddlePath;
		std::string materialLowPath;

		float heightScale{ 1.f };
		int resolution{ 1024 };
	};

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TerrainMaterial, name, heightMapPath, normalMapPath, splatMapPath, materialHighPath, materialMiddlePath, materialLowPath, heightScale, resolution);

	// @brief Represents a 3D mesh resource
	// A mesh resource contains the vertex data, sub-meshes, and indices for a 3D
	// model
	class Terrain : public Resource
	{
		enum TerrainMaterialType
		{
			HIGH = 0,
			MIDDLE,
			LOW
		};

	public:
		Terrain(uint32_t id, std::string_view resourceName, Logger& logger)
		    : Resource(id, std::string(resourceName)),
		      m_logger(logger, fmt::format("Terrain:{}", resourceName))
		{}

		// @brief Initializes the terrain by loading the terrain folder
		// @brief The terrain folder contains 
		// @param meshPath The file path to the mesh file
		// @returns true if the initialization was successful, false otherwise
		bool init(std::string_view terrainPath);

		// @brief Destroys the mesh resource and releases any allocated resources
		void destroy();

		const std::vector<Vertex>& getVertices() const { return m_vertices; }
		const std::vector<SubMesh>& getSubMeshes() const { return m_submeshes; }
		const std::vector<uint32_t>& getIndices() const { return m_indices; }
		const std::vector<PBRMaterial>& getMaterials() const { return m_materials; }

	private:
		void _loadMaterial(TerrainMaterialType materialType, std::string_view materialPath, Assimp::Importer& importer);
		bool _loadTerrainJson(const std::string& path, nlohmann::json& json);

		std::vector<Vertex> m_vertices;
		std::vector<SubMesh> m_submeshes;
		std::vector<uint32_t> m_indices;
		std::vector<PBRMaterial> m_materials;

		LoggerProxy m_logger;
	};

} // namespace SE
