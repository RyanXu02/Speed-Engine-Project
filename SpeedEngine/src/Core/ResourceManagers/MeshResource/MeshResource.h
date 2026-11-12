#pragma once
#include "../../Logger/LoggerProxy.h"
#include "../Resource.h"
#include <glm/glm.hpp>

struct aiScene;

namespace SE
{
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
	class MeshResource : public Resource
	{
	public:
		MeshResource(uint32_t id, std::string_view resourceName, Logger& logger)
		    : Resource(id, std::string(resourceName)),
		      m_logger(logger, fmt::format("MeshResource:{}", resourceName))
		{}

		// @brief Initializes the mesh resource by loading the mesh file
		// @param meshPath The file path to the mesh file
		// @returns true if the initialization was successful, false otherwise
		bool init(std::string_view meshPath);

		// @brief Destroys the mesh resource and releases any allocated resources
		void destroy();

		const std::vector<Vertex>& getVertices() const { return m_vertices; }
		const std::vector<SubMesh>& getSubMeshes() const { return m_submeshes; }
		const std::vector<uint32_t>& getIndices() const { return m_indices; }
		const std::vector<PBRMaterial>& getMaterials() const { return m_materials; }

	private:
		void _loadMaterials(const aiScene* scene, std::string_view meshPath);

		std::vector<Vertex> m_vertices;
		std::vector<SubMesh> m_submeshes;
		std::vector<uint32_t> m_indices;
		std::vector<PBRMaterial> m_materials;

		LoggerProxy m_logger;
	};

} // namespace SE
