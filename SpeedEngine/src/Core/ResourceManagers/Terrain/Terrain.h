#pragma once
#include "Core/ResourceManagers/MeshResource/MeshResource.h"

#include <nlohmann/json.hpp>
#include <glm/glm.hpp>

namespace SE
{
	struct TerrainMaterial
	{
		std::string name;

		std::string heightMapPath;
		std::string splatMapPath;

		// Materials used based on splat map R, G, B respectively
		std::string materialHighPath;
		std::string materialMiddlePath;
		std::string materialLowPath;

		float heightScale{ 1.f };
		int resolution{ 1024 };
	};

	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(TerrainMaterial, name, heightMapPath, splatMapPath, materialHighPath, materialMiddlePath, materialLowPath, heightScale, resolution);

	struct Terrain
	{
		const std::vector<Vertex>& getVertices() const { return m_vertices; }
		const std::vector<SubMesh>& getSubMeshes() const { return m_submeshes; }
		const std::vector<uint32_t>& getIndices() const { return m_indices; }
		const std::vector<PBRMaterial>& getMaterials() const { return m_materials; }
		const std::string& getSplatMapPath() const { return m_splatMapPath; }
		const std::string& getHeightMapPath() const { return m_heightMapPath; }
		float getHeightScale() const { return m_heightScale; }

		std::vector<Vertex> m_vertices;
		std::vector<SubMesh> m_submeshes;
		std::vector<uint32_t> m_indices;
		std::vector<PBRMaterial> m_materials;
		std::string m_splatMapPath;
		std::string m_heightMapPath;
		float m_heightScale{ 0.0f };

		uint32_t heightSampleCount = 0;
		std::vector<float> heightSamples;
	};

	std::unique_ptr<Terrain> loadTerrainData(const std::string& path, Logger& logger);

} // namespace SE
