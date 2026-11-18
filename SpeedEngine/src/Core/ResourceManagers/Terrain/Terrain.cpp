#include "pch.h"

#include "Terrain.h"
#include "Core/Features/Serializable.h"
#include "Core/Logger/LoggerProxy.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/GltfMaterial.h>

#include <filesystem>
#include <stb_image.h>

using nlohmann::json;

namespace SE
{
	static void loadMaterial(PBRMaterial& pbr, const std::string& materialPath,
		Assimp::Importer& importer, LoggerProxy& m_logger);

	std::unique_ptr<Terrain> loadTerrainData(const std::string& terrainPath, Logger& logger)
	{
		LoggerProxy m_logger(logger, "Terrain");
		// Deserialize terrain path
		json jsonTemp;
		if (!Serializable::loadJson(std::string(terrainPath), m_logger, jsonTemp))
			return {};

		TerrainMaterial terrainMat;
		try
		{
			terrainMat = jsonTemp.get<TerrainMaterial>();
		}
		catch (const json::exception& error)
		{
			m_logger.warn("Invalid terrain {}: {}", terrainPath, error.what());
			return {};
		}

		if (terrainMat.resolution < 2)
			return {};

		// Build terrain mesh
		auto data = std::make_unique<Terrain>();
		auto& m_vertices = data->m_vertices;
		auto& m_indices = data->m_indices;
		auto& m_submeshes = data->m_submeshes;
		auto& m_materials = data->m_materials;
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
		m_submeshes[0].indexCount = static_cast<uint32_t>(m_indices.size());
		m_submeshes[0].indexOffset = 0;
		m_submeshes[0].vertexCount = static_cast<uint32_t>(m_vertices.size());
		m_submeshes[0].vertexOffset = 0;
		m_submeshes[0].materialIndex = 0;

		// Import materials
		Assimp::Importer importer;
		m_materials.resize(3);
		loadMaterial(m_materials[0], terrainMat.materialHighPath, importer, m_logger);
		loadMaterial(m_materials[1], terrainMat.materialMiddlePath, importer, m_logger);
		loadMaterial(m_materials[2], terrainMat.materialLowPath, importer, m_logger);

		data->m_splatMapPath = terrainMat.splatMapPath;
		data->m_heightMapPath = terrainMat.heightMapPath;
		data->m_heightScale = terrainMat.heightScale;
		
		stbi_set_flip_vertically_on_load_thread(true);

		int width = 0, height = 0, channels = 0;
		const std::unique_ptr<uint16_t, decltype(&stbi_image_free)> samples(stbi_load_16(data->m_heightMapPath.c_str(), &width, &height, &channels, 1), stbi_image_free);
		if (samples && width == height && width > 1)
		{
			data->heightSampleCount = static_cast<uint32_t>(height);
			data->heightSamples.resize(static_cast<size_t>(width) * height);
			std::transform(samples.get(), samples.get() + data->heightSamples.size(), data->heightSamples.begin(),
				[](uint16_t sample) { return sample / 65535.0f; });
		}
		else
			m_logger.warn("Terrain {} has no usable square collision heightmap", terrainPath);

		m_logger.info("Loaded terrain: {} vertices, {} indices", m_vertices.size(), m_indices.size());

		return data;
	}

	static void loadMaterial(PBRMaterial& pbr, const std::string& materialPath, Assimp::Importer& importer, LoggerProxy& m_logger)
	{
		// Use assimp to import pbr material
		const aiScene* scene = importer.ReadFile(materialPath.data(), 0);

		if (!scene || !scene->HasMaterials())
		{
			m_logger.warn("Failed to load terrain material at {}: {}", materialPath, importer.GetErrorString());
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

	}

} // namespace SE
