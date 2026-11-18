#pragma once
#include <glm/glm.hpp>


namespace SE
{
	class Logger;
	// @brief Represents a vertex in a 3D mesh
	struct Vertex
	{
		glm::vec3 position{ 0.f };
		glm::vec3 normal{ 0.f };
		glm::vec2 uv{ 0.f };
	};

	// @brief Represents a sub-mesh within a 3D mesh
	// This is used for meshes that have multiple materials applied to different
	// parts of the mesh
	struct SubMesh
	{
		uint32_t indexOffset; // start of "mesh". index into m_indices
		uint32_t indexCount; // how many indices are contained in "mesh"
		uint32_t vertexCount; // how many vertices are contained in "mesh"
		uint32_t vertexOffset; // start of "mesh" vertices. index into m_vertices
		int32_t materialIndex; // index into materials container?????
	};

	struct PBRMaterial
	{
		static constexpr int roughnessChannel = 1;
		static constexpr int metallicChannel = 2;

		std::string name;

		std::string albedoPath;
		std::string normalPath;
		std::string aoPath;
		std::string emissivePath;
		std::string roughnessPath;
		std::string metallicPath;

		glm::vec4 albedoFactor{ 1.f };
		glm::vec3 emissiveFactor{ 0.f };
		float roughnessFactor{ 1.f };
		float metallicFactor{ 1.f };
		float aoStrength{ 1.f };
		float normalScale{ 1.f };
	};

	// @brief Represents a 3D mesh resource
	// A mesh resource contains the vertex data, sub-meshes, and indices for a 3D
	// model
	struct MeshResource
	{
		const std::vector<Vertex>& getVertices() const { return m_vertices; }
		const std::vector<SubMesh>& getSubMeshes() const { return m_submeshes; }
		const std::vector<uint32_t>& getIndices() const { return m_indices; }
		const std::vector<PBRMaterial>& getMaterials() const { return m_materials; }

		std::vector<Vertex> m_vertices;
		std::vector<SubMesh> m_submeshes;
		std::vector<uint32_t> m_indices;
		std::vector<PBRMaterial> m_materials;
	};

	std::unique_ptr<MeshResource> loadMeshData(const std::string& path, Logger& logger);

} // namespace SE
