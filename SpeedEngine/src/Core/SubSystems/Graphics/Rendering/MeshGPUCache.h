#pragma once
#include "Core/Logger/LoggerProxy.h"
#include "Core/ResourceManagers/MeshResource/MeshResource.h"

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

		const MeshGPUData& getOrUpload(uint32_t meshId, const MeshResource& mesh);
		void draw(uint32_t meshId, const MeshResource& mesh);
		void remove(uint32_t meshId);
		void clear();

	private:
		static void _destroy(MeshGPUData& data);

		LoggerProxy m_logger;
		std::unordered_map<uint32_t, MeshGPUData> m_meshes;
	};
} // namespace SE
