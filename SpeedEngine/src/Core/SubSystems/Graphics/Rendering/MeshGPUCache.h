#pragma once
#include "Core/Logger/LoggerProxy.h"
#include "Core/ResourceManagers/Resource.h"
#include "Core/ResourceManagers/MeshResource/MeshResource.h"
#include "Core/ResourceManagers/Terrain/Terrain.h"

namespace SE
{
	struct MeshGPUData
	{
		unsigned int VAO = 0;
		unsigned int VBO = 0;
		unsigned int EBO = 0;
	};

	// @brief Owns uploaded geometry shared by rendering passes
	class MeshGPUCache
	{
	public:
		explicit MeshGPUCache(Logger& logger)
			: m_logger(logger, "MeshGPUCache")
		{}
		~MeshGPUCache();

		MeshGPUCache(const MeshGPUCache&) = delete;
		MeshGPUCache& operator=(const MeshGPUCache&) = delete;
		MeshGPUCache(MeshGPUCache&&) = delete;
		MeshGPUCache& operator=(MeshGPUCache&&) = delete;

		void draw(ResourceID id, const MeshResource& mesh, size_t subMeshIndex);
		void draw(ResourceID id, const Terrain& terrain, size_t subMeshIndex);
		void clear();

	private:
		void drawGeometry(ResourceID id, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<SubMesh>& subMeshes, size_t subMeshIndex);
		const MeshGPUData* getOrUpload(ResourceID id, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		static void _destroy(MeshGPUData& data);

		LoggerProxy m_logger;
		std::unordered_map<ResourceID, MeshGPUData> m_meshes;
	};
} // namespace SE
