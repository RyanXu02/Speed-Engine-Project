#include "pch.h"

#include "MeshResource.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace SE
{
	bool MeshResource::init(std::string_view meshPath)
	{
		Assimp::Importer importer;

		// Read in mesh file with given flags
		// NOTE: may need aiProcess_FlipUVs
		const aiScene* scene = importer.ReadFile(meshPath.data(),
			aiProcess_Triangulate			|
			aiProcess_JoinIdenticalVertices	|
			aiProcess_GenSmoothNormals);

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
			m_submeshes[i].indexOffset = numIndices;
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
				const aiVector3D& vUV = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][j] : aiVector3D( 0.0f, 0.0f, 0.0f );

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

		m_logger.info("Loaded mesh: {} vertices, {} indices, {} submeshes", m_vertices.size(), m_indices.size(), m_submeshes.size());

		return true;
	}

	void MeshResource::destroy()
	{
		m_vertices.clear();
		m_submeshes.clear();
		m_indices.clear();
	}
} // namespace SE
