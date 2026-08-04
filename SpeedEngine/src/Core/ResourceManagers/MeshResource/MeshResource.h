#pragma once
#include "../Resource.h"
#include "../../Logger/LoggerProxy.h"

#include "glm/glm.hpp"

namespace SE
{
	struct Vertex {
		glm::vec3 position{ 0.f };
		glm::vec3 normal{ 0.f };
		glm::vec2 uv{ 0.f };
	};
	struct SubMesh {
		uint32_t indexOffset;
		uint32_t indexCount;
		int32_t materialIndex;
	};
	class MeshResource : public Resource
	{
	public:
		MeshResource(uint32_t id, std::string_view resourceName, Logger& logger) : Resource(id, std::string(resourceName)),
			m_logger(logger, fmt::format("MeshResource:{}", resourceName)) {}

		bool init(); //asdfasfsadfsa

		void destroy() {}; //asdfasjfsakfhja

	private:
		std::vector<Vertex> m_vertices;
		std::vector<SubMesh> m_submeshes;
		std::vector<uint32_t> m_indices; //respresents rapidobj::Array<Index> without tex and normals
		//std::vector<float> m_vertices;
		//std::vector<uint8_t> m_num_face_vertices;

		LoggerProxy m_logger;
	};

}

