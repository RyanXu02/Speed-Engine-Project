#include "pch.h"

#include "MeshGPUCache.h"

#include <glad/glad.h>

namespace SE
{
	MeshGPUCache::~MeshGPUCache()
	{
		clear();
	}

	const MeshGPUData* MeshGPUCache::getOrUpload(ResourceID meshId, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	{
		auto [it, inserted] = m_meshes.try_emplace(meshId);
		if (!inserted)
			return &it->second;

		auto& data = it->second;
		glGenVertexArrays(1, &data.VAO);
		glGenBuffers(1, &data.VBO);
		glGenBuffers(1, &data.EBO);
		if (!data.VAO || !data.VBO || !data.EBO)
		{
			_destroy(data);
			m_meshes.erase(it);
			m_logger.error("Failed to create GPU buffers for mesh {}", meshId);
			return nullptr;
		}

		glBindVertexArray(data.VAO);
		glBindBuffer(GL_ARRAY_BUFFER, data.VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		m_logger.info("Uploaded mesh {} to GPU: {} vertices, {} indices", meshId, vertices.size(), indices.size());
		return &data;
	}

	void MeshGPUCache::draw(ResourceID id, const MeshResource& mesh, size_t subMeshIndex)
	{
		drawGeometry(id, mesh.getVertices(), mesh.getIndices(), mesh.getSubMeshes(), subMeshIndex);
	}

	void MeshGPUCache::draw(ResourceID id, const Terrain& terrain, size_t subMeshIndex)
	{
		drawGeometry(id, terrain.getVertices(), terrain.getIndices(), terrain.getSubMeshes(), subMeshIndex);
	}

	void MeshGPUCache::drawGeometry(ResourceID id, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
		const std::vector<SubMesh>& subMeshes, size_t subMeshIndex)
	{
		if (subMeshIndex >= subMeshes.size() || subMeshes[subMeshIndex].indexCount == 0)
			return;

		const auto* data = getOrUpload(id, vertices, indices);
		if (!data)
			return;
		const auto& subMesh = subMeshes[subMeshIndex];
		glBindVertexArray(data->VAO);
		glDrawElementsBaseVertex(GL_TRIANGLES, static_cast<GLsizei>(subMesh.indexCount), GL_UNSIGNED_INT,
			reinterpret_cast<const void*>(sizeof(uint32_t) * subMesh.indexOffset), static_cast<GLint>(subMesh.vertexOffset));
		glBindVertexArray(0);
	}

	void MeshGPUCache::clear()
	{
		for (auto& data : m_meshes | std::views::values)
			_destroy(data);
		m_meshes.clear();
	}

	void MeshGPUCache::_destroy(MeshGPUData& data)
	{
		glDeleteBuffers(1, &data.EBO);
		glDeleteBuffers(1, &data.VBO);
		glDeleteVertexArrays(1, &data.VAO);
	}
} // namespace SE
