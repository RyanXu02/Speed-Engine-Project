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
		uint32_t indexOffset; //start of "mesh". index into m_indices
		uint32_t indexCount; //how many vertices are contained in "mesh"
		int32_t materialIndex; //index into materials container?????
	};
	class MeshResource : public Resource
	{
	public:
		MeshResource(uint32_t id, std::string_view resourceName, Logger& logger) : Resource(id, std::string(resourceName)),
			m_logger(logger, fmt::format("MeshResource:{}", resourceName)) {}

		bool init(std::string_view objpath, std::string_view mtlpath); //asdfasfsadfsa

		void destroy();

		std::vector<Vertex> getVertices() const { return m_vertices; }
		std::vector<uint32_t> getIndices() const { return m_indices; }

	private:
		std::vector<Vertex> m_vertices;
		std::vector<SubMesh> m_submeshes;
		std::vector<uint32_t> m_indices; //respresents rapidobj::Array<Index> without tex and normals

		LoggerProxy m_logger;
	};

}

