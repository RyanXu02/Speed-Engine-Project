#include "pch.h"

#include "Terrain.h"
#include "Core/Features/Serializable.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/GltfMaterial.h>

#include <filesystem>

using nlohmann::json;

namespace SE
{
	bool Terrain::init(std::string_view terrainPath)
	{
		// Deserialize terrain path
		json jsonTemp;
		_loadTerrainJson(terrainPath.data(), jsonTemp);
		TerrainMaterial terrainMat = jsonTemp.get<TerrainMaterial>();

		// Build terrain mesh
		unsigned int numVertices = terrainMat.resolution * terrainMat.resolution;
		m_indices.reserve((terrainMat.resolution - 1) * (terrainMat.resolution - 1) * 6);
		m_vertices.reserve(numVertices);

		float r = static_cast<float>(terrainMat.resolution - 1);
		int n = terrainMat.resolution;
		for (int j = 0; j < n; j++)
		{
			float s = j / r;
			for (int i = 0; i < n; i++)
			{
				float t = i / r;
				Vertex v;
				v.position = { s * 2.0f - 1.0f, 0.0f, t * 2.0f - 1.0f};
				v.normal = {0.0, 1.0, 0.0};
				v.uv = { s, t };
				m_vertices.push_back(v);
				if (i > 0 && j > 0)
				{	// Triangulate quads
					int idxI = (i - 1) * (n) + (j - 1);
					int idxJ = (i - 1) * (n) + (j);
					int idxK = (i) * (n) + (j);
					int idxL = (i) * (n) + (j - 1);
					m_indices.push_back(idxI);
					m_indices.push_back(idxJ);
					m_indices.push_back(idxK);
					m_indices.push_back(idxI);
					m_indices.push_back(idxK);
					m_indices.push_back(idxL);
				}
			}
		}

		// Only ever 1 submesh
		m_submeshes.resize(1);
		SubMesh subMesh;
		subMesh.indexCount = static_cast<uint32_t>(m_indices.size());
		subMesh.indexOffset = 0;
		subMesh.vertexCount = static_cast<uint32_t>(m_vertices.size());
		subMesh.vertexOffset = 0;
		subMesh.materialIndex = 0;
		m_submeshes.push_back(subMesh);

		// Import materials
		Assimp::Importer importer;
		m_materials.resize(3);
		_loadMaterial(TerrainMaterialType::HIGH, terrainMat.materialHighPath, importer);
		_loadMaterial(TerrainMaterialType::MIDDLE, terrainMat.materialMiddlePath, importer);
		_loadMaterial(TerrainMaterialType::LOW, terrainMat.materialLowPath, importer);

		m_splatMapPath = terrainMat.splatMapPath;
		m_heightMapPath = terrainMat.heightMapPath;
		m_heightScale = terrainMat.heightScale;

		m_logger.info("Loaded terrain: {} vertices, {} indices", m_vertices.size(), m_indices.size());

		return true;
	}

	void Terrain::_loadMaterial(TerrainMaterialType materialType, std::string_view materialPath, Assimp::Importer& importer)
	{
		// Use assimp to import pbr material
		const aiScene* scene = importer.ReadFile(materialPath.data(), 0);

		if (!scene)
		{
			m_logger.warn("Failed to load terrain material {} at {}. \n {}", static_cast<int>(materialType), materialPath, importer.GetErrorString());
			return;
		}

		const std::string directory = std::filesystem::path(materialPath).parent_path().string();

		auto getTexturePath = [&](const aiMaterial* mat, std::initializer_list<aiTextureType> types) -> std::string
		{
			for (aiTextureType type : types)
			{
				aiString path;
				if (mat->GetTexture(type, 0, &path) != AI_SUCCESS || path.length == 0) {
					continue;
				}

				std::string texPath = path.C_Str();
				if (texPath[0] == '*') {	//TODO: this should ideally be supported for binary file load
					m_logger.warn("Embedded texture {} is not supported", texPath);
					return "";
				}
				return directory + "/" + texPath;
			}
			return "";
		};

		const aiMaterial* mat = scene->mMaterials[0];
		PBRMaterial& pbr = m_materials[materialType];

		pbr.name = mat->GetName().C_Str();
		pbr.albedoPath = getTexturePath(mat, { aiTextureType_BASE_COLOR, aiTextureType_DIFFUSE });
		pbr.normalPath = getTexturePath(mat, { aiTextureType_NORMALS });
		pbr.aoPath = getTexturePath(mat, { aiTextureType_LIGHTMAP, aiTextureType_AMBIENT_OCCLUSION });
		pbr.emissivePath = getTexturePath(mat, { aiTextureType_EMISSIVE });
		pbr.roughnessPath = getTexturePath(mat, { aiTextureType_GLTF_METALLIC_ROUGHNESS, aiTextureType_DIFFUSE_ROUGHNESS, aiTextureType_METALNESS });
		pbr.metallicPath = pbr.roughnessPath;

		aiColor4D albedo;
		if (mat->Get(AI_MATKEY_BASE_COLOR, albedo) == AI_SUCCESS || mat->Get(AI_MATKEY_COLOR_DIFFUSE, albedo) == AI_SUCCESS) {
			pbr.albedoFactor = glm::vec4(albedo.r, albedo.g, albedo.b, albedo.a);
		}

		aiColor3D emissive;
		if (mat->Get(AI_MATKEY_COLOR_EMISSIVE, emissive) == AI_SUCCESS) {
			pbr.emissiveFactor = glm::vec3(emissive.r, emissive.g, emissive.b);
		}

		mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, pbr.roughnessFactor);
		mat->Get(AI_MATKEY_METALLIC_FACTOR, pbr.metallicFactor);
		mat->Get(AI_MATKEY_GLTF_TEXTURE_STRENGTH(aiTextureType_LIGHTMAP, 0), pbr.aoStrength);
		mat->Get(AI_MATKEY_GLTF_TEXTURE_SCALE(aiTextureType_NORMALS, 0), pbr.normalScale);

		m_logger.debug("Material {} '{}': albedo='{}' normal='{}' ao='{}' emissive='{}' metallicRoughness='{}'", static_cast<int>(materialType), pbr.name, pbr.albedoPath, pbr.normalPath, pbr.aoPath, pbr.emissivePath, pbr.roughnessPath);
	}

	void Terrain::destroy()
	{
		m_vertices.clear();
		m_submeshes.clear();
		m_indices.clear();
		m_materials.clear();
	}

	bool Terrain::_loadTerrainJson(const std::string& path, nlohmann::json& json)
	{
		std::ifstream file(path);
		if (!file.is_open())
		{
			m_logger.warn("Fail to open json: {}", path);
			return false;
		}

		// noexcept, returns a discarded json object on error
		json = nlohmann::json::parse(file, nullptr, false);

		if (json.is_discarded())
		{
			m_logger.warn("Fail to parse json: {}", path);
			return false;
		}

		return true;
	}
} // namespace SE
