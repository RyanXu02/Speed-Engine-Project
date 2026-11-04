#include "pch.h"

#include "MeshResource.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/GltfMaterial.h>

#include <filesystem>

namespace SE
{
	bool MeshResource::init(std::string_view meshPath)
	{
		Assimp::Importer importer;

		// Read in mesh file with given flags
		// NOTE: may need aiProcess_FlipUVs
		unsigned int flags = aiProcess_Triangulate
			| aiProcess_JoinIdenticalVertices
			| aiProcess_GenSmoothNormals;

		
		// PreTransformVertices bakes the node hierarchy transforms into the vertices,
		// otherwise meshes under different nodes end up at their untransformed positions
		// NOTE: for demonstration only, this would destroy bone hierarchy transform and break skeletal animation
		flags |= aiProcess_PreTransformVertices;

		const aiScene* scene = importer.ReadFile(meshPath.data(), flags);

		if (!scene)
		{
			m_logger.warn("Failed to load mesh at {}. \n {}", meshPath, importer.GetErrorString());
			return false;
		}

		m_submeshes.resize(scene->mNumMeshes);

		// Count vertices and indices to reserve space in vectors
		unsigned int numVertices = 0;
		unsigned int numIndices = 0;

		for (unsigned int i = 0; i < m_submeshes.size(); i++)
		{
			m_submeshes[i].indexCount = scene->mMeshes[i]->mNumFaces * 3;
			m_submeshes[i].vertexCount = scene->mMeshes[i]->mNumVertices;
			m_submeshes[i].indexOffset = numIndices;
			m_submeshes[i].vertexOffset = numVertices;
			m_submeshes[i].materialIndex = scene->mMeshes[i]->mMaterialIndex;

			numVertices += scene->mMeshes[i]->mNumVertices;
			numIndices += m_submeshes[i].indexCount;
		}

		m_indices.reserve(numIndices);
		m_vertices.reserve(numVertices);

		for (unsigned int i = 0; i < m_submeshes.size(); i++)
		{
			const aiMesh* mesh = scene->mMeshes[i];

			for (unsigned int j = 0; j < mesh->mNumVertices; j++)
			{
				const aiVector3D& vPos = mesh->mVertices[j];
				const aiVector3D& vNormal = mesh->mNormals[j];
				const aiVector3D& vUV = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][j] : aiVector3D(0.0f, 0.0f, 0.0f);

				Vertex v;
				v.position = glm::vec3(vPos.x, vPos.y, vPos.z);
				v.normal = glm::vec3(vNormal.x, vNormal.y, vNormal.z);
				v.uv = glm::vec2(vUV.x, vUV.y);
				m_vertices.push_back(v);
			}
			for (unsigned int k = 0; k < mesh->mNumFaces; k++)
			{
				const aiFace& face = mesh->mFaces[k];
				m_indices.push_back(face.mIndices[0]);
				m_indices.push_back(face.mIndices[1]);
				m_indices.push_back(face.mIndices[2]);
			}
		}

		_loadMaterials(scene, meshPath);

		m_logger.info("Loaded mesh: {} vertices, {} indices, {} submeshes, {} materials", m_vertices.size(), m_indices.size(), m_submeshes.size(), m_materials.size());

		return true;
	}

	void MeshResource::_loadMaterials(const aiScene* scene, std::string_view meshPath)
	{
		const std::string directory = std::filesystem::path(meshPath).parent_path().string();

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

		m_materials.resize(scene->mNumMaterials);

		for (unsigned int i = 0; i < scene->mNumMaterials; i++)
		{
			const aiMaterial* mat = scene->mMaterials[i];
			PBRMaterial& pbr = m_materials[i];

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

			m_logger.debug("Material {} '{}': albedo='{}' normal='{}' ao='{}' emissive='{}' metallicRoughness='{}'", i, pbr.name, pbr.albedoPath, pbr.normalPath, pbr.aoPath, pbr.emissivePath, pbr.roughnessPath);
		}
	}

	void MeshResource::destroy()
	{
		m_vertices.clear();
		m_submeshes.clear();
		m_indices.clear();
		m_materials.clear();
	}
} // namespace SE
